#pragma once



// CHelpView ������ͼ

class CHelpView : public CScrollView
{
	DECLARE_DYNCREATE(CHelpView)

public:
	CHelpView();           // ��̬������ʹ�õ��ܱ����Ĺ��캯��
	virtual ~CHelpView();

public:
	enum { IDD = IDD_HELP_VIEW };

	CImage m_imgReadme;

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
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
protected:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual void OnDraw(CDC* /*pDC*/);
};


