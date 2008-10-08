#ifndef _PSEXPORTFILEDLG_H_
#define _PSEXPORTFILEDLG_H_

#include <afxdlgs.h>
#include <Vfw.h>
#include "VideoCompression.h"

class CPSExportFileDlg : public CFileDialog
{
public:
	CPSExportFileDlg(BOOL bOpenFileDialog, // TRUE for FileOpen, FALSE for FileSaveAs
		LPCTSTR lpszDefExt = NULL,
		LPCTSTR lpszFileName = NULL,
		DWORD dwFlags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		LPCTSTR lpszFilter = NULL,
		CWnd* pParentWnd = NULL);

public:
	AVICOMPRESSOPTIONS m_aviOpts;

protected:
	virtual BOOL OnInitDialog();
	virtual BOOL OnFileNameOK();
};

#endif