// PS3DViewer.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "PS3DViewer.h"
#include "MainFrm.h"

#include "PS3DViewerDoc.h"
#include "PS3DViewerView.h"
#include "PSGame.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CPS3DViewerApp

BEGIN_MESSAGE_MAP(CPS3DViewerApp, CWinApp)
	ON_COMMAND(ID_APP_ABOUT, &CPS3DViewerApp::OnAppAbout)
	// Standard file based document commands
	ON_COMMAND(ID_FILE_NEW, &CWinApp::OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, &CWinApp::OnFileOpen)
END_MESSAGE_MAP()


// CPS3DViewerApp construction

CPS3DViewerApp::CPS3DViewerApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
	QueryPerformanceCounter(&m_lEnd);
}


// The one and only CPS3DViewerApp object

CPS3DViewerApp theApp;


// CPS3DViewerApp initialization

BOOL CPS3DViewerApp::InitInstance()
{
	// InitCommonControlsEx() is required on Windows XP if an application
	// manifest specifies use of ComCtl32.dll version 6 or later to enable
	// visual styles.  Otherwise, any window creation will fail.
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// Set this to include all the common control classes you want to use
	// in your application.
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();

	// Initialize OLE libraries
	if (!AfxOleInit())
	{
		AfxMessageBox(IDP_OLE_INIT_FAILED);
		return FALSE;
	}
	AfxEnableControlContainer();

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	// of your final executable, you should remove from the following
	// the specific initialization routines you do not need
	// Change the registry key under which our settings are stored
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization
	//SetRegistryKey(_T("Local AppWizard-Generated Applications"));
	//LoadStdProfileSettings(4);  // Load standard INI file options (including MRU)

	// Register the application's document templates.  Document templates
	//  serve as the connection between documents, frame windows and views
	CSingleDocTemplate* pDocTemplate;
	pDocTemplate = new CSingleDocTemplate(
		IDR_MAINFRAME,
		RUNTIME_CLASS(CPS3DViewerDoc),
		RUNTIME_CLASS(CMainFrame),       // main SDI frame window
		RUNTIME_CLASS(CPS3DViewerView));
	if (!pDocTemplate)
		return FALSE;
	AddDocTemplate(pDocTemplate);

	// Enable DDE Execute open
	//EnableShellOpen();
	//RegisterShellFileTypes(TRUE);

	// Parse command line for standard shell commands, DDE, file open
	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);

	// Dispatch commands specified on the command line.  Will return FALSE if
	// app was launched with /RegServer, /Register, /Unregserver or /Unregister.
	if (!ProcessShellCommand(cmdInfo))
		return FALSE;

	// The one and only window has been initialized, so show and update it
	m_pMainWnd->SetWindowText("POWER SYSTEM 3D");
	m_pMainWnd->ShowWindow(SW_SHOWMAXIMIZED);
	m_pMainWnd->UpdateWindow();

	PSGame::MakeEnvDir();
	m_Splash.Create( IDD_SPLASH, m_pMainWnd );
	m_Splash.ShowWindow(SW_SHOW);

	// call DragAcceptFiles only if there's a suffix
	//  In an SDI app, this should occur after ProcessShellCommand
	// Enable drag/drop open
	m_pMainWnd->DragAcceptFiles();
	
	return TRUE;
}



// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()

// App command to run the dialog
void CPS3DViewerApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}


// CPS3DViewerApp message handlers


BOOL CPS3DViewerApp::OnIdle(LONG lCount)
{
	if ( CWinApp::OnIdle(lCount) )
		return TRUE;

	CView* view = ((CMainFrame*)AfxGetMainWnd())->GetActiveView();

	if ( !g_PSGame.m_bExportAvi )
	{
		LARGE_INTEGER Frequency;
		LARGE_INTEGER lStart;

		QueryPerformanceFrequency(&Frequency);
		QueryPerformanceCounter(&lStart);

		float fInterval = ((float)(lStart.QuadPart - m_lEnd.QuadPart)) / ((float)(Frequency.QuadPart));
		QueryPerformanceCounter(&m_lEnd);

		g_PSGame.Idle( fInterval );
		view->SendMessage(WM_PAINT);

		static char hint[1024];
		g_PSGame.GetHintTxt( hint );
		((CMainFrame*)AfxGetMainWnd())->SetStatusBarText( hint );
	}
	else
	{
		if ( g_PSGame.m_pFrameBuffer == 0 ) 
			g_PSGame.m_pFrameBuffer = new GLbyte[g_PSGame.m_nWidth * 3 * g_PSGame.m_nHeight];
		
		static char hint[1024];
		g_PSGame.GetHintTxt( hint );
		((CMainFrame*)AfxGetMainWnd())->SetStatusBarText( hint );

		g_PSGame.Idle(1.0f/g_PSGame.m_nAviFps);
		view->SendMessage(WM_PAINT);

		memset(g_PSGame.m_pFrameBuffer, 0, g_PSGame.m_nWidth * 3 * g_PSGame.m_nHeight);
		GLint lastBuffer;
		glGetIntegerv(GL_READ_BUFFER, &lastBuffer);
		glReadBuffer(GL_FRONT);
		glReadPixels(0, 0, g_PSGame.m_nWidth, g_PSGame.m_nHeight, GL_BGR_EXT, GL_UNSIGNED_BYTE, g_PSGame.m_pFrameBuffer);
		glReadBuffer(lastBuffer);
		g_PSGame.m_pVideoMgr->AddFrame((BYTE*)g_PSGame.m_pFrameBuffer);

		if ( g_PSGame.m_bEnded/*g_PSGame.m_fExportedTime >= g_PSGame.m_fExportFrameTime+1*/ )
		{
			((CMainFrame*)AfxGetMainWnd())->SetStatusBarText( "Ending Export..." );

			SAFE_DELETE(g_PSGame.m_pFrameBuffer);

			if(g_PSGame.m_pVideoMgr)
				g_PSGame.m_pVideoMgr->CloseAVIVideo();
			
			g_PSGame.m_fExportedTime = 0; 
			g_PSGame.m_bExportAvi = false;
			g_PSGame.m_GameStage = STAND;

			ShowCursor( TRUE );
			g_PSGame.m_bCalcExportFrameTime = false;
			g_PSGame.m_bEnded = false;
			g_PSGame.m_fElapsedFrameTime = 0;
		}

		g_PSGame.m_fExportedTime += 1.0f/g_PSGame.m_nAviFps;
	}

	return TRUE;
}

BOOL CPS3DViewerApp::ProcessMessageFilter(int code, LPMSG lpMsg)
{
	// TODO: Add your specialized code here and/or call the base class
	QueryPerformanceCounter(&m_lEnd);

	return CWinApp::ProcessMessageFilter(code, lpMsg);
}
