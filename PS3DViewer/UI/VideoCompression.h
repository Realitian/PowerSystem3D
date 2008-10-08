#pragma once

#include "vfw.h"
#include "resource.h"
// CVideoCompression dialog
class CVideoCompression : public CDialog
{
	DECLARE_DYNAMIC(CVideoCompression)

public:
	CVideoCompression(CWnd* pParent=NULL);   // standard constructor
	virtual ~CVideoCompression();

// Dialog Data
	//{{AFX_DATA(CVideoCompression)
	enum { IDD = IDD_VIDEO_COMPRESSION };
	CStatic		m_lblCompressor;
	CStatic		m_lblQuality;
	CStatic		m_lblQualityValue;
	CScrollBar	m_sclQuality;
	CButton	m_chkKeyFrame;
	CEdit		m_editCompress;
	CStatic		m_lblFrames;
	CButton		m_btnAbout;
	CButton		m_btnSetting;
	CButton		m_btnOK;
	CButton		m_btnCancel;
	//}}AFX_DATA

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	
	int			m_nCompress;
	int			m_nCompressValue;
	CString		m_sCompressValue;
	CComboBox	m_cCompressCombo;

	AVICOMPRESSOPTIONS m_sCompressOpt;
	HIC					m_hICLocate;

protected:
	void		UpdateScrollState();
    afx_msg void OnDestroy();
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnAboutCompress();
	afx_msg void OnSettingCompress();
	afx_msg void OnSelchangeCompressType();
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
public:
	void InitAVICompressOption(LPAVICOMPRESSOPTIONS opt);
	BOOL m_bKeyFrame;
	int m_nKeyFrame;
	afx_msg void OnBnClickedCheckCompress();
};
