#pragma once


// CHelpFrame ���

class CHelpFrame : public CFrameWnd
{
	DECLARE_DYNCREATE(CHelpFrame)
public:
	CHelpFrame();           // ��̬������ʹ�õ��ܱ����Ĺ��캯��
	virtual ~CHelpFrame();

	CView	*m_pParentView;
protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnClose();
};


