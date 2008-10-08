// PS3DViewerDoc.h : interface of the CPS3DViewerDoc class
//


#pragma once


class CPS3DViewerDoc : public CDocument
{
protected: // create from serialization only
	CPS3DViewerDoc();
	DECLARE_DYNCREATE(CPS3DViewerDoc)

// Attributes
public:

// Operations
public:

// Overrides
public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);

// Implementation
public:
	virtual ~CPS3DViewerDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()
};


