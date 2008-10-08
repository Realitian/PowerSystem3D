// PS3DViewerView.h : interface of the CPS3DViewerView class
//


#pragma once


class CPS3DViewerView : public CView
{
protected: // create from serialization only
	CPS3DViewerView();
	DECLARE_DYNCREATE(CPS3DViewerView)

// Attributes
public:
	CPS3DViewerDoc* GetDocument() const;
	
	HGLRC m_hRC; // Rendering Context
	HDC m_hDC; // Device Context
	CPalette m_GLPalette;

// Operations
public:

// Overrides
public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:

// Implementation
public:
	virtual ~CPS3DViewerView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()

	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg BOOL OnQueryNewPalette();
	afx_msg void OnPaletteChanged(CWnd* pFocusWnd);

	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
};

#ifndef _DEBUG  // debug version in PS3DViewerView.cpp
inline CPS3DViewerDoc* CPS3DViewerView::GetDocument() const
   { return reinterpret_cast<CPS3DViewerDoc*>(m_pDocument); }
#endif

