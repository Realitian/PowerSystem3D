
#if !defined(AFX_AVIGENERATOR_H__6BAF2E9D_3866_4779_A43B_D1B21E7E4F39__INCLUDED_)
#define AFX_AVIGENERATOR_H__6BAF2E9D_3866_4779_A43B_D1B21E7E4F39__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// needed headers
#include <comdef.h>
#include <memory.h>
#include <tchar.h>
#include <string.h>
#include <Vfw.h>

#ifndef MAXNUMSTREAMS
#define MAXNUMSTREAMS 50
#endif

// undefine this if you don't use MFC
#define _AVIGENERATOR_USE_MFC

class CAVIVideo  
{
public:
	//! \name Constructors and destructors
	//@{
	//! Default constructor 
	CAVIVideo();
	CAVIVideo(LPCTSTR sFileName, SIZE size, DWORD dwRate, AVICOMPRESSOPTIONS opts); 

	~CAVIVideo();
	//@}
	
	// create new avi file.
	HRESULT CreateAVIVideo();

	// close open avi file.
	void	CloseAVIVideo();

	/*! \brief Adds a frame to the movie. 
	
	The data pointed by bmBits has to be compatible with the bitmap description of the movie.
	*/
	HRESULT AddFrame(BYTE* bmBits);

	LPVOID	GetFrame(UINT nFrame);

	//! \name Setters and getters
	//@{
	//! Sets bitmap info to match pView dimension.
	void SetBitmapHeader(CView* pView);
	//! Sets bitmap info as in lpbih
	void SetBitmapHeader(LPBITMAPINFOHEADER lpbih);

	void SetBitmapHeader(SIZE size);
	//! returns a pointer to bitmap info
	LPBITMAPINFOHEADER GetBitmapHeader()							{	return &m_bih;};
	//! sets the name of the ouput file (should be .avi)
	void SetFileName(LPCTSTR _sFileName)							{	m_sFile=_sFileName; };
	//! Sets FrameRate (should equal or greater than one)
	void SetRate(DWORD dwRate)										{	m_dwRate=dwRate;};
	//@}
	
	//! \name Error handling
	//@{
	//! returns last  error message
	LPCTSTR GetLastErrorMessage() const								{	return m_sError;};
	//@}

	AVISTREAMINFO GetAVIInfo() const { return m_avis;}

	//Initialize AVI Compress options m_opts
	void InitAVICompressOptions();

protected:	
	//! name of output file
	_bstr_t m_sFile;			
	//! Frame rate 
	DWORD m_dwRate;	
	//! structure contains information for a single stream
	BITMAPINFOHEADER m_bih;	
	//! last error string
	_bstr_t m_sError;

private:
	//! frame counter
	long m_lFrame;
	//! file interface pointer
	PAVIFILE m_pAVIFile;
	//! Address of the stream interface
	PAVISTREAM m_pStream;		
	//! Address of the compressed video stream
	PAVISTREAM m_pStreamCompressed; 
	//! Stream Information of video stream

	// the current streams
	PAVISTREAM m_pAviStream[MAXNUMSTREAMS];	
	// data for decompressing
	PGETFRAME  m_gFrame[MAXNUMSTREAMS];	

	INT			m_iStreams;

	AVISTREAMINFO	m_avis;

	AVICOMPRESSOPTIONS m_opts;

	PGETFRAME	    m_pgf;

	/*
	LPBYTE			m_bmBits;*/
};

#endif // !defined(AFX_AVIGENERATOR_H__6BAF2E9D_3866_4779_A43B_D1B21E7E4F39__INCLUDED_)
