// AVIGenerator.cpp: implementation of the CAVIVideo class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "AVIVideo.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#include "shlwapi.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////


CAVIVideo::CAVIVideo()
: m_sFile(_T("Untitled.avi")), m_dwRate(30),
m_pAVIFile(NULL), m_pStream(NULL), m_pStreamCompressed(NULL), m_pgf(NULL), m_iStreams(0)//, m_bmBits(NULL)
{
	// Initialize AVI library.
	AVIFileInit();
	memset(&m_bih,0,sizeof(BITMAPINFOHEADER));
	InitAVICompressOptions();
}

CAVIVideo::CAVIVideo(LPCTSTR sFileName, SIZE size, DWORD dwRate, AVICOMPRESSOPTIONS opts)
: m_sFile(sFileName), m_dwRate(dwRate),
m_pAVIFile(NULL), m_pStream(NULL), m_pStreamCompressed(NULL), m_pgf(NULL), m_iStreams(0)//, m_bmBits(NULL)
{
	// Initialize AVI library.
	AVIFileInit();
	SetBitmapHeader(size);
	m_opts = opts;
	m_iStreams = 0;
}

void CAVIVideo::InitAVICompressOptions()
{
	memset(&m_opts, 0, sizeof(m_opts));

	m_opts.fccType = streamtypeVIDEO;
	m_opts.dwKeyFrameEvery = (DWORD) -1; // Default
	m_opts.dwQuality = (DWORD) ICQUALITY_DEFAULT;
	m_opts.dwFlags = AVICOMPRESSF_VALID | AVICOMPRESSF_KEYFRAMES;
	m_opts.dwInterleaveEvery = 1;
}

CAVIVideo::~CAVIVideo()
{
#ifndef _DEBUG
	try 
#endif//_DEBUG
	{
		// Close engine
		AVIFileExit();
	}
#ifndef _DEBUG
	catch(...) {
	}
#endif//_DEBUG
}

void CAVIVideo::SetBitmapHeader(LPBITMAPINFOHEADER lpbih)
{
	// checking that bitmap size are multiple of 4
	ASSERT(lpbih->biWidth%4==0);
	ASSERT(lpbih->biHeight%4==0);

	// copying bitmap info structure.
	// corrected thanks to Lori Gardi
	memcpy(&m_bih,lpbih, sizeof(BITMAPINFOHEADER));
}

#ifdef _AVIGENERATOR_USE_MFC
void CAVIVideo::SetBitmapHeader(CView *pView)
{
	ASSERT_VALID(pView);

	////////////////////////////////////////////////
	// Getting screen dimensions
	// Get client geometry 
	CRect rect; 
	pView->GetClientRect(&rect); 
	CSize size(rect.Width(),rect.Height()); 

	/////////////////////////////////////////////////
	// changing size of image so dimension are multiple of 4
	size.cx=(size.cx/4)*4;
	size.cy=(size.cy/4)*4;

	// initialize m_bih
	memset(&m_bih,0, sizeof(BITMAPINFOHEADER));
	// filling bitmap info structure.
	m_bih.biSize=sizeof(BITMAPINFOHEADER);
	m_bih.biWidth=size.cx;
	m_bih.biHeight=size.cy;
	m_bih.biPlanes=1;
	m_bih.biBitCount=24;
	m_bih.biSizeImage=((m_bih.biWidth*m_bih.biBitCount+31)/32 * 4)*m_bih.biHeight; 
	m_bih.biCompression=BI_RGB;		//BI_RGB means BRG in reality
}
#endif

void CAVIVideo::SetBitmapHeader(SIZE size)
{
	size.cx=(size.cx/4)*4;
	size.cy=(size.cy/4)*4;

	// initialize m_bih
	memset(&m_bih,0, sizeof(BITMAPINFOHEADER));
	// filling bitmap info structure.
	m_bih.biSize=sizeof(BITMAPINFOHEADER);
	m_bih.biWidth=size.cx;
	m_bih.biHeight=size.cy;
	m_bih.biPlanes=1;
	m_bih.biBitCount=24;
	m_bih.biSizeImage=((m_bih.biWidth*m_bih.biBitCount+31)/32 * 4)*m_bih.biHeight; 
	m_bih.biCompression=BI_RGB;		//BI_RGB means BRG in reality
}

HRESULT CAVIVideo::AddFrame(BYTE *bmBits)
{
	HRESULT hr = S_FALSE;
#ifndef _DEBUG
	try 
#endif//_DEBUG
	{
		if (!m_pStreamCompressed) 
			return hr;

		// compress bitmap
		hr = AVIStreamWrite(m_pStreamCompressed,	// stream pointer
			m_lFrame,						// time of this frame
			1,						// number to write
			bmBits,					// image buffer
			m_bih.biSizeImage,		// size of this frame
			AVIIF_KEYFRAME,			// flags....
			NULL,
			NULL);

		// updating frame counter
		m_lFrame++;
	}
#ifndef _DEBUG
	catch (...)
	{
		hr = S_FALSE;
	}
#endif//_DEBUG
	return hr;
}


LPVOID CAVIVideo::GetFrame(UINT nFrame)
{
	LPVOID	lpbi = NULL;

#ifndef _DEBUG
	try
#endif//_DEBUG
	{
		for(INT i = 0; i < m_iStreams; i ++)
		{
			AVISTREAMINFO avis;
			if (m_pAviStream[i]) {
				AVIStreamInfo(m_pAviStream[i], &avis, sizeof(avis));

				//Draw Video Stream

				if(avis.fccType == streamtypeVIDEO)
				{
					if(m_gFrame[i] == NULL)
						continue;

					if (m_gFrame[i] && (LONG)nFrame >= AVIStreamStart(m_pAviStream[i]))
					{
						lpbi = (LPBITMAPINFOHEADER)AVIStreamGetFrame(m_gFrame[i], nFrame);
						return lpbi;
					}
					else
						lpbi = NULL;
				}
			}
		}
	}
#ifndef _DEBUG
	catch(...)
	{
		lpbi = NULL;
	}
#endif//_DEBUG

	return lpbi;
}

HRESULT CAVIVideo::CreateAVIVideo()
{	
	AVISTREAMINFO strHdr; // information for a single stream 

	TCHAR szBuffer[1024];
	HRESULT hr;
#ifndef _DEBUG
	try	
#endif//_DEBUG
	{
		m_sError=_T("Ok");

		// Step 0 : Let's make sure we are running on 1.1 
		DWORD wVer = HIWORD(VideoForWindowsVersion());
		if (wVer < 0x010a)
		{
			// oops, we are too old, blow out of here 
			m_sError=_T("Version of Video for Windows too old.");
			return S_FALSE;
		}

		CloseAVIVideo();

		::DeleteFile((LPCSTR)m_sFile);

		// Step 1 : Open the movie file for writing....
		hr = AVIFileOpen(&m_pAVIFile,			// Address to contain the new file interface pointer
			(LPCSTR)m_sFile,				// Null-terminated string containing the name of the file to open
			OF_WRITE | OF_CREATE,			// Access mode to use when opening the file. 
			NULL);							// use handler determined from file extension.
		// Name your file .avi -> very important

		if (hr != AVIERR_OK)
		{
			_tprintf(szBuffer,_T("AVI Engine failed to initialize. Check filename %s."),m_sFile);
			m_sError=szBuffer;
			goto ERREXIT;
		}

		// Fill in the header for the video stream....
		memset(&strHdr, 0, sizeof(strHdr));
		strHdr.fccType                = streamtypeVIDEO;	// video stream type
		strHdr.fccHandler             = 0;
		strHdr.dwScale                = 1;					// should be one for video
		strHdr.dwRate                 = m_dwRate;		    // fps
		strHdr.dwSuggestedBufferSize  = m_bih.biSizeImage;	// Recommended buffer size, in bytes, for the stream.
		SetRect(&strHdr.rcFrame, 0, 0,		    // rectangle for stream
			(int) m_bih.biWidth,
			(int) m_bih.biHeight);

		// Step 2 : Create the stream;
		hr = AVIFileCreateStream(m_pAVIFile,		    // file pointer
			&m_pStream,		    // returned stream pointer
			&strHdr);	    // stream header

		// Check it succeded.
		if (hr != AVIERR_OK)
		{
			m_sError=_T("AVI Stream creation failed. Check Bitmap info.");
			goto ERREXIT;
		}

		// Step 4:  Create a compressed stream using codec options.
		hr = AVIMakeCompressedStream(&m_pStreamCompressed, 
			m_pStream, 
			&m_opts, 
			NULL);

		if (hr != AVIERR_OK)
		{
			m_sError=_T("AVI Compressed Stream creation failed.");
			goto ERREXIT;
		}

		// Step 5 : sets the format of a stream at the specified position
		hr = AVIStreamSetFormat(m_pStreamCompressed, 
			0,			// position
			&m_bih,	    // stream format
			m_bih.biSize +   // format size
			m_bih.biClrUsed * sizeof(RGBQUAD));

		if (hr != AVIERR_OK)
		{
			m_sError=_T("AVI Compressed Stream format setting failed.");
			goto ERREXIT;
		}

		// Step 6 : Initialize step counter
		m_lFrame=0;
	}
#ifndef _DEBUG
	catch(...)
	{
		return S_FALSE;
	}
#endif//_DEBUG

	return hr;
ERREXIT:
	if (m_pStreamCompressed) {
		AVIStreamRelease(m_pStreamCompressed);
		m_pStreamCompressed = NULL;
	}

	if (m_pStream) {
		AVIStreamRelease(m_pStream);
		m_pStream = NULL;
	}

	if (m_pAVIFile) {
		AVIFileRelease(m_pAVIFile);
		m_pAVIFile = NULL;
	}

	return hr;
}

void CAVIVideo::CloseAVIVideo()
{
	if (m_iStreams) {
#ifndef _DEBUG
		try 
#endif//_DEBUG
		{
			for(int i = 0; i < m_iStreams; i++)
			{
				if(m_gFrame[i])
				{
					// Close Streams 
					AVIStreamGetFrameClose(m_gFrame[i]);
					m_gFrame[i] = NULL;
				}
				AVIStreamRelease(m_pAviStream[i]);
				m_pAviStream[i] = NULL;
			}
		}
#ifndef _DEBUG
		catch(...)
		{
			for (int i = 0; i < m_iStreams; i++)
			{
				m_gFrame[i] = NULL;
				m_pAviStream[i] = NULL;
			}
		}
#endif//_DEBUG

		m_iStreams =  0;
	}

	if (m_pStreamCompressed)
	{
		AVIStreamRelease(m_pStreamCompressed);
		m_pStreamCompressed=NULL;
	}

	if (m_pStream)
	{
		AVIStreamRelease(m_pStream);
		m_pStream=NULL;
	}

	if (m_pAVIFile)
	{
		AVIFileRelease(m_pAVIFile);
		m_pAVIFile=NULL;
	}
}