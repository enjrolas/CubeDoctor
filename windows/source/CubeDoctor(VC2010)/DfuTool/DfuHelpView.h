#pragma once
#include "afxwin.h"

#include "Label.h"

// CDfuHelpView ������ͼ

class CDfuHelpView : public CFormView
{
	DECLARE_DYNCREATE(CDfuHelpView)

protected:
	CDfuHelpView();           // ��̬������ʹ�õ��ܱ����Ĺ��캯��
	virtual ~CDfuHelpView();

public:
	enum { IDD = IDD_DFUHELP_VIEW };

	CLabel m_LabelNote;


	CFont	m_Font1;//,m_Font2,m_Font3;

	ImageEx*	m_image;

#ifdef _DEBUG
	virtual void AssertValid() const;
#ifndef _WIN32_WCE
	virtual void Dump(CDumpContext& dc) const;
#endif
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

	DECLARE_MESSAGE_MAP()
public:
	virtual void OnInitialUpdate();
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
};


