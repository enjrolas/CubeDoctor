// CircleButton1.cpp : implementation file
//

#include "stdafx.h"
//#include "PlaneTest.h"
#include "CircleButton1.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CCircleButton

CCircleButton::CCircleButton()
{
	m_textcolor = RGB(0,0,0); //Ĭ��Ϊ��ɫ
	m_bitmapID = 0; //IDB_BITMAP1;
	isPressed= false;
	size.cx = 0;
	size.cy = 0;
	h_rgn = 0;
}

CCircleButton::~CCircleButton()
{
	m_textcolor = RGB(0,0,0); //Ĭ��Ϊ��ɫ
	m_bitmapID = 0; //IDB_BITMAP1;
	isPressed= false;
	size.cx = 0;
	size.cy = 0;
	h_rgn = 0;
}


BEGIN_MESSAGE_MAP(CCircleButton, CButton)
	//{{AFX_MSG_MAP(CCircleButton)
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCircleButton message handlers
void CCircleButton::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct) 
{
	if(m_bitmapID==0)
		return;
	CDC dc;
	dc.Attach(lpDrawItemStruct->hDC);
	
	UINT state = lpDrawItemStruct->itemState; //��ȡ��ť״̬

	CBitmap bitmap;
	if (!bitmap.LoadBitmap(m_bitmapID)) //����λͼ
		return;
	BITMAPINFO info;
	bitmap.GetObject(sizeof(info),(void*)&info);//��ȡλͼ��С
	

	size.cy = info.bmiHeader.biHeight;
	size.cx = info.bmiHeader.biWidth; 
	
	//����λͼ��С���ð�ť��С
	CRect rect;
	GetClientRect(rect);
	MapWindowPoints(GetParent(),rect);
	MoveWindow(rect.left,rect.top,size.cx,size.cy);

	CDC memdc;
	memdc.CreateCompatibleDC(&dc);
	memdc.SelectObject(&bitmap);


	//����λͼ��С���ð�ť��ʾ����
	h_rgn = CreateEllipticRgn(0,0,size.cx,size.cy); 
	SetWindowRgn(h_rgn,true);

	//���ñ���͸��
	dc.SetBkMode(TRANSPARENT);

	//����һ��λͼ��ˢ
	CBrush brush;
	brush.CreatePatternBrush(&bitmap);

	dc.SelectObject(&brush);

	if ((state&ODS_SELECTED)||(state&ODS_FOCUS))
	{
	
		dc.Ellipse(0,0,size.cx,size.cy); //���õ�ǰ��ˢ��䰴ť
		
	}
	else
	{
		CPen pen(PS_NULL,1,RGB(0,0,0));
		dc.SelectObject(&pen);
		//SetCapture();
		dc.Ellipse(-1,-1,size.cx+2,size.cy+2);
	}

	//���ư�ť�ı�
	CString str;
	GetWindowText(str);
	dc.SetTextColor(m_textcolor);
	dc.DrawText(str,CRect(0,0,size.cx,size.cy),DT_CENTER|DT_VCENTER|DT_SINGLELINE);		
}

void CCircleButton::OnLButtonDown(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	m_textcolor = RGB(255,0,0);	
	
	CButton::OnLButtonDown(nFlags, point);
}

void CCircleButton::OnLButtonUp(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	m_textcolor = RGB(0,0,0);
	
	CButton::OnLButtonUp(nFlags, point);
}

void CCircleButton::SetButtonBitmap(UINT bitmapID)
{
	m_bitmapID = bitmapID;

}

void CCircleButton::OnMouseMove(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	CRgn temp;
	temp.CreateEllipticRgn(0,0,size.cx,size.cy);

	if (!temp.PtInRegion(point)) //�ж������Ƿ���Բ��������
	{
		m_textcolor = RGB(0,0,0);  //������㳬����Բ������ָ���ť�ı���ɫ
		ReleaseCapture(); //�����ť������,�ͷ���겶׽
	}
	
	CButton::OnMouseMove(nFlags, point);
}
