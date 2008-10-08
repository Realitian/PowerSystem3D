#include "stdafx.h"
#include "PS3DViewer.h"
#include "DlgUpLoad.h"


// CDlgUpLoad dialog

IMPLEMENT_DYNAMIC(CDlgUpLoad, CDialog)

CDlgUpLoad::CDlgUpLoad(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgUpLoad::IDD, pParent)
	, m_strScenario(_T(""))
{

}

CDlgUpLoad::~CDlgUpLoad()
{
}

void CDlgUpLoad::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_SAVENAME, m_strScenario);
}


BEGIN_MESSAGE_MAP(CDlgUpLoad, CDialog)
END_MESSAGE_MAP()


// CDlgUpLoad message handlers
