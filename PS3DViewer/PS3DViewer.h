// PS3DViewer.h : main header file for the PS3DViewer application
//
#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"       // main symbols
#include "DialogSplash.h"

// CPS3DViewerApp:
// See PS3DViewer.cpp for the implementation of this class
//

class CPS3DViewerApp : public CWinApp
{
public:
	CPS3DViewerApp();


// Overrides
public:
	virtual BOOL InitInstance();

// Implementation
	afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnIdle(LONG lCount);
	LARGE_INTEGER m_lEnd;
	CDialogSplash m_Splash;
public:
	virtual BOOL ProcessMessageFilter(int code, LPMSG lpMsg);
};

extern CPS3DViewerApp theApp;