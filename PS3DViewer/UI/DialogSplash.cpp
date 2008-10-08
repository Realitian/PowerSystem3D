// DialogSplash.cpp : implementation file
//

#include "stdafx.h"
#include "DialogSplash.h"
#include "Global.h"

extern void CreateJpegDB( const char* dirpath, const char* prefix, int startid, int endid );
extern unsigned char* ReadJpegDB( const char* dirpath, const char* prefix, int id, int& img_width, int &img_height );
extern void ReadJpegRef( const char* dirpath, const char* prefix, int &startid, int &endid );

// CDialogSplash dialog

IMPLEMENT_DYNAMIC(CDialogSplash, CDialog)

CDialogSplash::CDialogSplash(CWnd* pParent /*=NULL*/)
	: CDialog(CDialogSplash::IDD, pParent)
{
	m_pBmpBuffer = 0;
	m_pTitleBuffer = 0;
	m_pTxtBuffer = 0;

	memset(&m_BMI, 0, sizeof(BITMAPINFO));
	m_BMI.bmiHeader.biSize        = sizeof(BITMAPINFOHEADER);
	m_BMI.bmiHeader.biWidth       = 0;
	m_BMI.bmiHeader.biHeight      = 0;
	m_BMI.bmiHeader.biPlanes      = 1;
	m_BMI.bmiHeader.biBitCount    = 24;
	m_BMI.bmiHeader.biCompression = BI_RGB;
	m_BMI.bmiHeader.biSizeImage   = 0;

	memset(&m_BMITxt, 0, sizeof(BITMAPINFO));
	m_BMITxt.bmiHeader.biSize        = sizeof(BITMAPINFOHEADER);
	m_BMITxt.bmiHeader.biWidth       = 0;
	m_BMITxt.bmiHeader.biHeight      = 0;
	m_BMITxt.bmiHeader.biPlanes      = 1;
	m_BMITxt.bmiHeader.biBitCount    = 24;
	m_BMITxt.bmiHeader.biCompression = BI_RGB;
	m_BMITxt.bmiHeader.biSizeImage   = 0;

	memset(&m_BMITitle, 0, sizeof(BITMAPINFO));
	m_BMITitle.bmiHeader.biSize        = sizeof(BITMAPINFOHEADER);
	m_BMITitle.bmiHeader.biWidth       = 0;
	m_BMITitle.bmiHeader.biHeight      = 0;
	m_BMITitle.bmiHeader.biPlanes      = 1;
	m_BMITitle.bmiHeader.biBitCount    = 24;
	m_BMITitle.bmiHeader.biCompression = BI_RGB;
	m_BMITitle.bmiHeader.biSizeImage   = 0;
}

CDialogSplash::~CDialogSplash()
{
	if ( m_pBmpBuffer )
	{
		free( m_pBmpBuffer );
		m_pBmpBuffer = 0;
	}

	if ( m_pTitleBuffer )
	{
		free( m_pTitleBuffer );
		m_pTitleBuffer = 0;
	}

	if ( m_pTxtBuffer )
	{
		free( m_pTxtBuffer );
		m_pTxtBuffer = 0;
	}
}

void CDialogSplash::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CDialogSplash, CDialog)
	ON_WM_TIMER()	
	ON_WM_PAINT()
END_MESSAGE_MAP()


// CDialogSplash message handlers

void CDialogSplash::OnTimer(UINT_PTR nIDEvent)
{
	int img_width, img_height;
	if ( m_pBmpBuffer )
		free( m_pBmpBuffer );

	static int id = 10001;
	static int ei = 0;
	id ++;

	if ( id > 10120 )
	{
		ei++;
		id = 10001;
	}

	if ( ei > 9 )
	{
		m_pBmpBuffer = 0;
		DestroyWindow();
		return;
	}

	CString path;
	path.Format("%s\\Effect\\%d\\", GetAppDirectory(), ei);
	m_pBmpBuffer = ReadJpegDB( path, "", id, img_width, img_height );

	m_BMI.bmiHeader.biWidth       = img_width;
	m_BMI.bmiHeader.biHeight      = img_height;
	m_BMI.bmiHeader.biSizeImage   = img_width * img_height * 3;

	//txt image
	path.Format("%s\\Effect\\title\\", GetAppDirectory() );
	if ( m_pTxtBuffer )
		free( m_pTxtBuffer );
	m_pTxtBuffer = ReadJpegDB( path, "", ei+1, img_width, img_height );
	m_BMITxt.bmiHeader.biWidth       = img_width;
	m_BMITxt.bmiHeader.biHeight      = img_height;
	m_BMITxt.bmiHeader.biSizeImage   = img_width * img_height * 3;

	Invalidate(FALSE);
	
	CDialog::OnTimer( nIDEvent );
}

BOOL CDialogSplash::OnInitDialog()
{
#if 0
	CreateJpegDB( GetAppDirectory() + "\\Effect\\1\\", "", 10001, 10120 );
	CreateJpegDB( GetAppDirectory() + "\\Effect\\2\\", "", 10001, 10120 );
	CreateJpegDB( GetAppDirectory() + "\\Effect\\3\\", "", 10001, 10120 );
	CreateJpegDB( GetAppDirectory() + "\\Effect\\4\\", "", 10001, 10120 );
	CreateJpegDB( GetAppDirectory() + "\\Effect\\5\\", "", 10001, 10120 );
	CreateJpegDB( GetAppDirectory() + "\\Effect\\6\\", "", 10001, 10120 );
	CreateJpegDB( GetAppDirectory() + "\\Effect\\7\\", "", 10001, 10120 );
	CreateJpegDB( GetAppDirectory() + "\\Effect\\8\\", "", 10001, 10120 );
	CreateJpegDB( GetAppDirectory() + "\\Effect\\9\\", "", 10001, 10120 );
	CreateJpegDB( GetAppDirectory() + "\\Effect\\0\\", "", 10001, 10120 );
	CreateJpegDB( GetAppDirectory() + "\\Effect\\title\\", "", 0, 10 );
#endif

	CDialog::OnInitDialog();
	
	MoveWindow(0, 0, 512, 512);
	CenterWindow(NULL);
	
	CString path;
	path.Format("%s\\Effect\\title\\", GetAppDirectory() );

	int img_width, img_height;
	m_pTitleBuffer = ReadJpegDB( path, "", 0, img_width, img_height );
	m_BMITitle.bmiHeader.biWidth = img_width;
	m_BMITitle.bmiHeader.biHeight = img_height;
	m_BMITitle.bmiHeader.biSizeImage   = m_BMITitle.bmiHeader.biWidth * m_BMITitle.bmiHeader.biHeight * 3;

	return TRUE;  
}

void CDialogSplash::OnPaint()
{
	CRect rc;
	GetClientRect(rc);
	
	CPaintDC cdc(this);
	
	CDC dc;
	dc.CreateCompatibleDC(&cdc);
	CBitmap off;
	off.CreateCompatibleBitmap(&cdc, rc.Width(), rc.Height());
	dc.SelectObject(&off);

	if ( m_pBmpBuffer )
		StretchDIBits( dc, 0, 0, rc.Width(), rc.Height(), 0, 0, m_BMI.bmiHeader.biWidth, m_BMI.bmiHeader.biHeight, m_pBmpBuffer, &m_BMI, DIB_RGB_COLORS, SRCCOPY );
	if ( m_pTitleBuffer )
		StretchDIBits( dc, 0, 0, m_BMITitle.bmiHeader.biWidth, m_BMITitle.bmiHeader.biHeight, 0, 0, m_BMITitle.bmiHeader.biWidth, m_BMITitle.bmiHeader.biHeight, m_pTitleBuffer, &m_BMITitle, DIB_RGB_COLORS, SRCCOPY );
	if ( m_pTxtBuffer )
		StretchDIBits( dc, 0, rc.Height() - m_BMITxt.bmiHeader.biHeight, m_BMITxt.bmiHeader.biWidth, m_BMITxt.bmiHeader.biHeight, 0, 0, m_BMITxt.bmiHeader.biWidth, m_BMITxt.bmiHeader.biHeight, m_pTxtBuffer, &m_BMITxt, DIB_RGB_COLORS, SRCCOPY );

	CPen pen(PS_SOLID, 3, RGB(255, 255, 0));
	dc.SelectObject(&pen);
	dc.SelectStockObject(NULL_BRUSH);
	dc.Rectangle(rc);

	cdc.BitBlt(0, 0, rc.Width(), rc.Height(), &dc, 0, 0, SRCCOPY);
}

LRESULT CDialogSplash::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	// TODO: Add your specialized code here and/or call the base class
	switch ( message )
	{
	case WM_CREATE:
		SetTimer( 33, 30, NULL );
		break;
	case WM_TIMER:
		{
			int a = 0;
		}
		break;
	case WM_DESTROY:
		{
			int a = 0;
		}
		break;
	}

	return CDialog::WindowProc(message, wParam, lParam);
}
