// PS3DViewerView.cpp : implementation of the CPS3DViewerView class
//

#include "stdafx.h"
#include "PS3DViewer.h"

#include "PS3DViewerDoc.h"
#include "PS3DViewerView.h"
#include "MainFrm.h"
#include "PSGame.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CPS3DViewerView

IMPLEMENT_DYNCREATE(CPS3DViewerView, CView)

BEGIN_MESSAGE_MAP(CPS3DViewerView, CView)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_SIZE()
	ON_WM_ERASEBKGND()
	ON_WM_QUERYNEWPALETTE()
	ON_WM_PALETTECHANGED()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_RBUTTONDOWN()
	ON_WM_MOUSEMOVE()
	ON_WM_KEYDOWN()
	ON_WM_KEYUP()
END_MESSAGE_MAP()

// CPS3DViewerView construction/destruction

CPS3DViewerView::CPS3DViewerView()
{
	// TODO: add construction code here

}

CPS3DViewerView::~CPS3DViewerView()
{
}

BOOL CPS3DViewerView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs
	cs.style |= (WS_CLIPCHILDREN | WS_CLIPSIBLINGS | CS_OWNDC);


	return CView::PreCreateWindow(cs);
}

// CPS3DViewerView drawing

void CPS3DViewerView::OnDraw(CDC* /*pDC*/)
{
	g_PSGame.GLRenderProc();
	SwapBuffers(m_hDC);
}


// CPS3DViewerView diagnostics

#ifdef _DEBUG
void CPS3DViewerView::AssertValid() const
{
	CView::AssertValid();
}

void CPS3DViewerView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CPS3DViewerDoc* CPS3DViewerView::GetDocument() const // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CPS3DViewerDoc)));
	return (CPS3DViewerDoc*)m_pDocument;
}
#endif //_DEBUG


// CPS3DViewerView message handlers

int CPS3DViewerView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CView::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO:  Add your specialized creation code here
	int nPixelFormat; // Pixel format index
	m_hDC = ::GetDC(m_hWnd); // Get the device context
	static PIXELFORMATDESCRIPTOR pfd = {
		sizeof(PIXELFORMATDESCRIPTOR), // Size of this structure
		1, // Version of this structure
		PFD_DRAW_TO_WINDOW | // Draw to Window (not bitmap)
		PFD_SUPPORT_OPENGL | // Support OpenGL in window
		PFD_DOUBLEBUFFER, // Double -buffered mode
		PFD_TYPE_RGBA, // RGBA color mode
		32, // Want 32bit color
		0,0,0,0,0,0, // Not used to select mode
		0,0, // Not used to select mode
		0,0,0,0,0, // Not used to select mode
		24, // Size of depth buffer
		0, // Not used to select mode
		0, // Not used to select mode
		0, // Draw in main plane
		0, // Not used to select mode
		0,0,0 
	}; // Not used to select mode

	// Choose a pixel format that best matches that described in pfd
	nPixelFormat = ChoosePixelFormat(m_hDC, &pfd );
	// Set the pixel format for the device context
	VERIFY(SetPixelFormat(m_hDC, nPixelFormat, &pfd));
	// Create the rendering context
	m_hRC = wglCreateContext(m_hDC);
	// Make the rendering context current, perform initialization, then deselect it
	VERIFY(wglMakeCurrent(m_hDC,m_hRC));
	
	g_PSGame.OnInitialize();
	g_PSGame.GLSetupRC();

	return 0;
}

void CPS3DViewerView::OnDestroy()
{
	CView::OnDestroy();

	// TODO: Add your message handler code here
	wglDeleteContext(m_hRC);
	::ReleaseDC(m_hWnd,m_hDC);
	KillTimer( 33 );
}

void CPS3DViewerView::OnSize(UINT nType, int cx, int cy)
{
	CView::OnSize(nType, cx, cy);

	// TODO: Add your message handler code here
	g_PSGame.GLResize(cx, cy);
}

BOOL CPS3DViewerView::OnEraseBkgnd(CDC* pDC)
{
	// TODO: Add your message handler code here and/or call default

	return FALSE;//CView::OnEraseBkgnd(pDC);
}

BOOL CPS3DViewerView::OnQueryNewPalette()
{
	// TODO: Add your message handler code here and/or call default
	// If the palette was created.
	if((HPALETTE)m_GLPalette)
	{
		int nRet;
		// Selects the palette into the current device context
		SelectPalette(m_hDC, (HPALETTE)m_GLPalette, FALSE);
		// Map entries from the currently selected palette to
		// the system palette. The return value is the number
		// of palette entries modified.
		nRet = RealizePalette(m_hDC);
		// Repaint, forces remap of palette in current window
		Invalidate(FALSE);
		return nRet;
	}

	return CView::OnQueryNewPalette();
}

void CPS3DViewerView::OnPaletteChanged(CWnd* pFocusWnd)
{
	CView::OnPaletteChanged(pFocusWnd);

	// TODO: Add your message handler code here
	if(((HPALETTE)m_GLPalette != NULL) && (pFocusWnd != this))
	{
		// Select the palette into the device context
		SelectPalette(m_hDC,(HPALETTE)m_GLPalette,FALSE);
		// Map entries to system palette
		RealizePalette(m_hDC);
		// Remap the current colors to the newly realized palette
		UpdateColors(m_hDC);
		return;
	}
}

void CPS3DViewerView::OnLButtonDown(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	g_PSGame.OnLButtonDown(nFlags, point);
	CView::OnLButtonDown(nFlags, point);
}

void CPS3DViewerView::OnLButtonUp(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	g_PSGame.OnLButtonUp(nFlags, point);
	CView::OnLButtonUp(nFlags, point);
}

void CPS3DViewerView::OnRButtonDown(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	g_PSGame.OnRButtonDown(nFlags, point);
	CView::OnRButtonDown(nFlags, point);
}

void CPS3DViewerView::OnMouseMove(UINT nFlags, CPoint point)
{
 	// TODO: Add your message handler code here and/or call default
 	g_PSGame.OnMouseMove(nFlags, point);
	CView::OnMouseMove(nFlags, point);
}

void CPS3DViewerView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	// TODO: Add your message handler code here and/or call default
	if ( nChar == VK_ESCAPE && theApp.m_Splash.GetSafeHwnd() )
		theApp.m_Splash.DestroyWindow();

	g_PSGame.OnKeyDown( nChar, nRepCnt, nFlags );
	CView::OnKeyDown(nChar, nRepCnt, nFlags);
}

void CPS3DViewerView::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	// TODO: Add your message handler code here and/or call default
	g_PSGame.OnKeyUp( nChar, nRepCnt, nFlags );
	CView::OnKeyUp(nChar, nRepCnt, nFlags);
}
