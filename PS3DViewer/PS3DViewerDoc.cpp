// PS3DViewerDoc.cpp : implementation of the CPS3DViewerDoc class
//

#include "stdafx.h"
#include "PS3DViewer.h"

#include "PS3DViewerDoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CPS3DViewerDoc

IMPLEMENT_DYNCREATE(CPS3DViewerDoc, CDocument)

BEGIN_MESSAGE_MAP(CPS3DViewerDoc, CDocument)
END_MESSAGE_MAP()


// CPS3DViewerDoc construction/destruction

CPS3DViewerDoc::CPS3DViewerDoc()
{
	// TODO: add one-time construction code here

}

CPS3DViewerDoc::~CPS3DViewerDoc()
{
}

BOOL CPS3DViewerDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: add reinitialization code here
	// (SDI documents will reuse this document)

	return TRUE;
}




// CPS3DViewerDoc serialization

void CPS3DViewerDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: add storing code here
	}
	else
	{
		// TODO: add loading code here
	}
}


// CPS3DViewerDoc diagnostics

#ifdef _DEBUG
void CPS3DViewerDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CPS3DViewerDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG


// CPS3DViewerDoc commands
