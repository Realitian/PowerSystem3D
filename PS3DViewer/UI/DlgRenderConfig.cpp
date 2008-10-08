#include "stdafx.h"
#include "PS3DViewer.h"
#include "DlgRenderConfig.h"
#include "PSExportFileDlg.h"
#include "Global.h"

//
#define FPS_MIN_VALUE 1
#define FPS_MAX_VALUE	100

// CDlgRenderConfig dialog

IMPLEMENT_DYNAMIC(CDlgRenderConfig, CDialog)

CDlgRenderConfig::CDlgRenderConfig(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgRenderConfig::IDD, pParent)
	, m_nFps(0)
	, m_aviPath(_T(""))
{
	m_nFps = 24;
	ZeroMemory(&m_aviOpts, sizeof m_aviOpts);
	m_aviOpts.fccType = streamtypeVIDEO;
	m_aviOpts.dwKeyFrameEvery = (DWORD) -1; // Default
	m_aviOpts.dwQuality = (DWORD) ICQUALITY_DEFAULT;
	m_aviOpts.dwFlags = AVICOMPRESSF_VALID | AVICOMPRESSF_KEYFRAMES;
	m_aviOpts.dwInterleaveEvery = 1;
}

CDlgRenderConfig::~CDlgRenderConfig()
{
}

void CDlgRenderConfig::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_FPS, m_nFps);
	DDV_MinMaxShort(pDX, m_nFps, 1, 100);
	DDX_Text(pDX, IDC_FilePath, m_aviPath);
	DDX_Control(pDX, IDC_FilePath, m_cAviPath);
}

BOOL CDlgRenderConfig::OnInitDialog()
{
	CDialog::OnInitDialog();
	m_cAviPath.SetSel(0, -1);
	return TRUE;
}

BEGIN_MESSAGE_MAP(CDlgRenderConfig, CDialog)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_FPS, &CDlgRenderConfig::OnDeltaposSpinFps)
	ON_BN_CLICKED(IDC_File, &CDlgRenderConfig::OnBnClickedFile)
	ON_EN_CHANGE(IDC_FilePath, &CDlgRenderConfig::OnEnChangeFilepath)
END_MESSAGE_MAP()


// CDlgRenderConfig message handlers

void CDlgRenderConfig::OnDeltaposSpinFps(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);
	// TODO: Add your control notification handler code here
	m_nFps -= pNMUpDown->iDelta;
	if(m_nFps < FPS_MIN_VALUE)
		m_nFps = FPS_MIN_VALUE;
	if(m_nFps > FPS_MAX_VALUE)
		m_nFps = FPS_MAX_VALUE;

	UpdateData(FALSE);

	*pResult = 0;
}

void CDlgRenderConfig::OnBnClickedFile()
{
	// TODO: Add your control notification handler cod here
	if(m_aviPath.IsEmpty())
		m_aviPath = GetAppDirectory() + "\\Movies\\*.avi";
	CPSExportFileDlg dlg(FALSE, "avi", m_aviPath, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, "AVI File(*.avi)|*.avi|");
	dlg.m_aviOpts = m_aviOpts;
	if (dlg.DoModal() != IDOK)
		return;
	m_aviPath = dlg.GetPathName();
	m_aviOpts = dlg.m_aviOpts;
	UpdateData(FALSE);
	m_cAviPath.SetSel(0, -1);
}

void CDlgRenderConfig::OnEnChangeFilepath()
{
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.

	// TODO:  Add your control notification handler code here
}

void CDlgRenderConfig::OnOK()
{
	UpdateData(TRUE);
	if (m_aviPath.IsEmpty())
	{
		m_aviPath = GetAppDirectory() + "\\Movies\\*.avi";
		OnBnClickedFile();
	}
	else
		EndDialog(IDOK);
}