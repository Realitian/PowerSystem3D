// VideoCompression.cpp : implementation file
//

#include "stdafx.h"
#include "VideoCompression.h"


// CVideoCompression dialog

#define  MAX_COMPRESS	10000
#define	 PAGE_STEP		1000
#define	 ONE_STEP		100


IMPLEMENT_DYNAMIC(CVideoCompression, CDialog)
CVideoCompression::CVideoCompression(CWnd* pParent /*=NULL*/)
	: CDialog(CVideoCompression::IDD, pParent)
	, m_nCompress(0)
	, m_sCompressValue(_T(""))
	, m_bKeyFrame(TRUE)
	, m_nKeyFrame(15)
{
	m_nCompress = 0;
	ZeroMemory(&m_sCompressOpt, sizeof(AVICOMPRESSOPTIONS));

	m_sCompressOpt.fccType = streamtypeVIDEO;
	m_sCompressOpt.dwKeyFrameEvery = (DWORD) -1; // Default
	m_sCompressOpt.dwQuality = (DWORD) ICQUALITY_DEFAULT;
	m_sCompressOpt.dwFlags = AVICOMPRESSF_VALID | AVICOMPRESSF_KEYFRAMES;
	m_sCompressOpt.dwInterleaveEvery = 1;
}

CVideoCompression::~CVideoCompression()
{
//	if(m_sCompressOpt.lpParms)
//		free(m_sCompressOpt.lpParms);
//	m_sCompressOpt.lpParms = NULL;
}

void CVideoCompression::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO_COMPRESS, m_cCompressCombo);
	DDX_CBIndex(pDX, IDC_COMBO_COMPRESS, m_nCompress);
	DDX_Text(pDX, IDC_STATIC_COMPRESS, m_sCompressValue);
	DDX_Check(pDX, IDC_CHECK_COMPRESS, m_bKeyFrame);
	DDX_Text(pDX, IDC_EDIT_COMPRESS, m_nKeyFrame);

	//{{AFX_DATA_MAP(CVideoCompression)
	DDX_Control(pDX, IDC_STATIC_COMPRESSOR, m_lblCompressor);
	DDX_Control(pDX, IDC_STATIC_CMPQLTY, m_lblQuality);
	DDX_Control(pDX, IDC_STATIC_COMPRESS, m_lblQualityValue);
	DDX_Control(pDX, IDC_SCROLL_COMPRESS, m_sclQuality);
	DDX_Control(pDX, IDC_CHECK_COMPRESS, m_chkKeyFrame);
	DDX_Control(pDX, IDC_EDIT_COMPRESS, m_editCompress);
	DDX_Control(pDX, IDC_STATIC_FRAMES, m_lblFrames);
	DDX_Control(pDX, IDC_BTN_ABOUT, m_btnAbout);
	DDX_Control(pDX, IDC_BTN_SETTING, m_btnSetting);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	//}}AFX_DATA_MAP

}


BEGIN_MESSAGE_MAP(CVideoCompression, CDialog)
	ON_WM_HSCROLL()
	ON_BN_CLICKED(IDC_BTN_ABOUT, OnAboutCompress)
	ON_BN_CLICKED(IDC_BTN_SETTING, OnSettingCompress)
	ON_CBN_SELCHANGE(IDC_COMBO_COMPRESS, OnSelchangeCompressType)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDOK, OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, OnBnClickedCancel)
	ON_BN_CLICKED(IDC_CHECK_COMPRESS, OnBnClickedCheckCompress)
END_MESSAGE_MAP()


// CVideoCompression message handlers



void CVideoCompression::OnAboutCompress()
{
	HIC hic = (HIC) m_cCompressCombo.GetItemData(m_nCompress);

	if(ICQueryAbout(hic))
		ICAbout(hic, GetSafeHwnd());
}

void CVideoCompression::OnSettingCompress()
{
	HIC hic = (HIC) m_cCompressCombo.GetItemData(m_nCompress);
	if(ICQueryConfigure(hic))
		ICConfigure(hic, GetSafeHwnd());
}

void CVideoCompression::OnSelchangeCompressType()
{
	ICINFO icinfo;
	BOOL fSupports;
	DWORD dwFlags;

	UpdateData();

	HIC hic = (HIC) m_cCompressCombo.GetItemData(m_nCompress);

	GetDlgItem(IDC_BTN_SETTING)->EnableWindow(ICQueryConfigure(hic));
	GetDlgItem(IDC_BTN_ABOUT)->EnableWindow(ICQueryAbout(hic));
	m_nCompressValue = -1;

	GetDlgItem(IDC_SCROLL_COMPRESS)->EnableWindow(FALSE);
	GetDlgItem(IDC_STATIC_CMPQLTY)->EnableWindow(FALSE);
	GetDlgItem(IDC_STATIC_COMPRESS)->EnableWindow(FALSE);

	if (hic) {
		ICGetInfo(hic, &icinfo, sizeof(ICINFO));
		dwFlags = icinfo.dwFlags;
	}
	else{
		dwFlags = 0;
	}
	fSupports = !!(dwFlags & VIDCF_TEMPORAL);	

	GetDlgItem(IDC_CHECK_COMPRESS)->EnableWindow(fSupports);
	GetDlgItem(IDC_EDIT_COMPRESS)->EnableWindow(fSupports);
	GetDlgItem(IDC_STATIC_FRAMES)->EnableWindow(fSupports);

	if(hic == NULL)
		return;

	DWORD quality;
	if(ICSendMessage(hic, 
		ICM_GETDEFAULTQUALITY, 
		(DWORD_PTR)(&quality), 
		sizeof(DWORD)) == ICERR_OK)
	{
		m_nCompressValue = quality;
	}
	UpdateScrollState();
}

void CVideoCompression::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	//SB_THUMBPOSITION
	CDialog::OnHScroll(nSBCode, nPos, pScrollBar);
	CScrollBar* pScroll = (CScrollBar*)GetDlgItem(IDC_SCROLL_COMPRESS);
	int nPageStep = 10;
	if(pScrollBar == pScroll && nSBCode != SB_ENDSCROLL)
	{
		switch(nSBCode)
		{
		case SB_LINEUP:
			m_nCompressValue -= ONE_STEP;
			break;
		case SB_LINEDOWN:
			m_nCompressValue += ONE_STEP;
			break;
		case SB_PAGEUP:
			m_nCompressValue -= PAGE_STEP;
			break;
		case SB_PAGEDOWN:
			m_nCompressValue += PAGE_STEP;
			break;
		case SB_THUMBPOSITION:
		case SB_THUMBTRACK:
			m_nCompressValue = nPos;
			break;
		default:
			return;
		}
		if(m_nCompressValue < 0)
			m_nCompressValue = 0;
		if(m_nCompressValue > MAX_COMPRESS)
			m_nCompressValue = MAX_COMPRESS;
		UpdateScrollState();
	}
}

void CVideoCompression::OnDestroy()
{
	for(int i = 0; i < m_cCompressCombo.GetCount(); i++)
	{
		HIC hic = (HIC) m_cCompressCombo.GetItemData(i);
		if(hic != NULL)
			ICClose(hic);
	}
	if(m_hICLocate != NULL)
		ICClose(m_hICLocate);
	m_hICLocate = NULL;
}

BOOL CVideoCompression::OnInitDialog()
{
	CDialog::OnInitDialog();

	CScrollBar* pScroll = (CScrollBar*)GetDlgItem(IDC_SCROLL_COMPRESS);
	pScroll->SetScrollRange(0, MAX_COMPRESS, FALSE);

	CComboBox *pCombo = (CComboBox*)GetDlgItem(IDC_COMBO_COMPRESS);

	BITMAPINFOHEADER bih;
	ZeroMemory(&bih, sizeof(BITMAPINFOHEADER));
	bih.biSize = sizeof(BITMAPINFOHEADER);
	bih.biPlanes = 1;
	bih.biCompression = BI_RGB;		// standard RGB bitmap
	bih.biBitCount = 24;			// 24 bits-per-pixel format

	DWORD fccType = ICTYPE_VIDEO;
	m_hICLocate = ICLocate (fccType, 0L, (LPBITMAPINFOHEADER) &bih, NULL, ICMODE_COMPRESS);
	ICINFO icinfo;
	for(int i = 0; ICInfo(fccType, i, &icinfo); i++)
	{
		HIC hic = ICOpen(icinfo.fccType, icinfo.fccHandler, ICMODE_COMPRESS);
		if(hic)
		{
			// Find out the compressor name.
			ICGetInfo(hic, &icinfo, sizeof(ICINFO));
			if((icinfo.dwFlags & VIDCF_COMPRESSFRAMES) != 0)
			{
				// Add it to the combo box.
				USES_CONVERSION;
				int n = pCombo->AddString(W2T(&icinfo.szDescription[0]));
				pCombo->SetItemData( n, (DWORD_PTR)(hic) );
			}
			else
			{
				ICClose(hic);
			}
		}
	}
	CString str;
	str.LoadString(IDS_UNCOMPRESSED);
	pCombo->AddString((LPCTSTR) str);
	pCombo->SetCurSel(0);

	GetDlgItem(IDC_BTN_SETTING)->EnableWindow(FALSE);
	GetDlgItem(IDC_BTN_ABOUT)->EnableWindow(FALSE);
	GetDlgItem(IDC_SCROLL_COMPRESS)->EnableWindow(FALSE);
	GetDlgItem(IDC_STATIC_CMPQLTY)->EnableWindow(FALSE);
	GetDlgItem(IDC_STATIC_COMPRESS)->EnableWindow(FALSE);
	GetDlgItem(IDC_CHECK_COMPRESS)->EnableWindow(FALSE);
	GetDlgItem(IDC_EDIT_COMPRESS)->EnableWindow(FALSE);
	GetDlgItem(IDC_STATIC_FRAMES)->EnableWindow(FALSE);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CVideoCompression::OnBnClickedOk()
{
	if (!UpdateData())
		return;	// don't close CFileDialog
	HIC hic = (HIC) m_cCompressCombo.GetItemData(m_nCompress);

	if(hic == NULL)
	{
		OnOK();
		return;
	}

	ICINFO icinfo;
	ICGetInfo(hic, &icinfo, sizeof(ICINFO));
	DWORD nStateSize = ICGetStateSize(hic);

	m_sCompressOpt.fccType = icinfo.fccType;
	m_sCompressOpt.fccHandler = icinfo.fccHandler;
	if (m_bKeyFrame)
		m_sCompressOpt.dwKeyFrameEvery = m_nKeyFrame;
	else
		m_sCompressOpt.dwKeyFrameEvery = ICGetDefaultKeyFrameRate(hic);
	m_sCompressOpt.dwQuality = (DWORD)m_nCompressValue;
	m_sCompressOpt.dwFlags = AVICOMPRESSF_KEYFRAMES | AVICOMPRESSF_VALID;
	m_sCompressOpt.lpParms = malloc(nStateSize);
	m_sCompressOpt.cbParms = nStateSize;
	m_sCompressOpt.dwInterleaveEvery = 1;

	ICGetState(hic, m_sCompressOpt.lpParms, nStateSize);

	OnOK();
}

void CVideoCompression::OnBnClickedCancel()
{
	// TODO: Add your control notification handler code here
	OnCancel();
}

void CVideoCompression::UpdateScrollState()
{
	CScrollBar* pScroll = (CScrollBar*)GetDlgItem(IDC_SCROLL_COMPRESS);
	if(m_nCompressValue < 0)
	{
		pScroll->EnableWindow(FALSE);
		GetDlgItem(IDC_STATIC_CMPQLTY)->EnableWindow(FALSE);
		GetDlgItem(IDC_STATIC_COMPRESS)->EnableWindow(FALSE);
	}
	else
	{
		if(!pScroll->IsWindowEnabled())
		{
			pScroll->EnableWindow(TRUE);
			GetDlgItem(IDC_STATIC_CMPQLTY)->EnableWindow(TRUE);
			GetDlgItem(IDC_STATIC_COMPRESS)->EnableWindow(TRUE);
		}
		m_sCompressValue.Format(_T("%d"), m_nCompressValue / 100);
		pScroll->SetScrollPos(m_nCompressValue);
		UpdateData(FALSE);
	}
}

void CVideoCompression::OnBnClickedCheckCompress()
{
	UpdateData();

	GetDlgItem(IDC_EDIT_COMPRESS)->EnableWindow(m_bKeyFrame);
	GetDlgItem(IDC_STATIC_FRAMES)->EnableWindow(m_bKeyFrame);
}
