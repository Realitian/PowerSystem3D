#include "stdafx.h"
#include "PS3DViewer.h"
#include "DlgDownLoad.h"


// CDlgDownLoad dialog

IMPLEMENT_DYNAMIC(CDlgDownLoad, CDialog)

CDlgDownLoad::CDlgDownLoad(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgDownLoad::IDD, pParent)
	, m_strName(_T(""))
{

}

CDlgDownLoad::~CDlgDownLoad()
{
}

void CDlgDownLoad::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_NAMELIST, m_ComboBox);
	DDX_CBString(pDX, IDC_NAMELIST, m_strName);
}


BEGIN_MESSAGE_MAP(CDlgDownLoad, CDialog)
	ON_CBN_SETFOCUS(IDC_NAMELIST, &CDlgDownLoad::OnCbnSetfocusNamelist)
	ON_CBN_SELCHANGE(IDC_NAMELIST, &CDlgDownLoad::OnCbnSelchangeNamelist)
END_MESSAGE_MAP()


// CDlgDownLoad message handlers

BOOL CDlgDownLoad::OnInitDialog()
{
	CDialog::OnInitDialog();

	for ( int i = 0 ; i < m_strNames.GetSize(); i++ )
		m_ComboBox.AddString( m_strNames[i] );

	GetDlgItem(IDOK)->EnableWindow( FALSE );

	return TRUE;
}
void CDlgDownLoad::OnCbnSetfocusNamelist()
{
	// TODO: Add your control notification handler code here
}

void CDlgDownLoad::OnCbnSelchangeNamelist()
{
	// TODO: Add your control notification handler code here
	GetDlgItem(IDOK)->EnableWindow( TRUE );
}
