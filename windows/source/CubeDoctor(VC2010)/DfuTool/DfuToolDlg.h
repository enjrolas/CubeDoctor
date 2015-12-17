
// DfuToolDlg.h : header file
//

#pragma once
#include "afxwin.h"
#include "Label.h"
#include "CircleButton1.h"
#include <getopt.h>
#include <main.h>
#include "HelpFrame.h"
//#include "HelpView.h"
#ifdef USE_ONEKEY_DOWNLOAD
#include "libwdi.h"
#include "msapi_utf8.h"
#endif
// CDfuToolDlg dialog
class CDfuToolDlg : public CDialog
{
// Construction
public:
	CDfuToolDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
#ifdef USE_ONEKEY_DOWNLOAD
	enum { IDD = IDD_DFUTOOL_DIALOG_ONEKEY };
#else
	enum { IDD = IDD_DFUTOOL_DIALOG };
#endif
//	CStatic m_LabelTitle;
	CLabel m_LabelTitle;
	CLabel m_LabelNote;
	CEdit m_EditUpdateFile[MAX_UPDATE_NUM];
//	CString m_strEditUpdateFile[MAX_UPDATE_NUM];
	CProgressCtrl m_ProgressCtrl;
	CEdit m_EditPrompt;
	CCircleButton m_btnLampDfu;
	CCircleButton m_btnLampUpdate[1+MAX_UPDATE_NUM];	//0:AUTO,1-3:UPDATE1-3
#ifdef USE_ONEKEY_DOWNLOAD
	CLinkCtrl m_LinkCtrl1,m_LinkCtrl2;
#endif
	CFont	m_Font1,m_Font2,m_Font3;
	CHelpFrame *m_pHelpFrame;
	CHelpFrame *m_pDfuHelpFrame;

	HANDLE m_hWaitEvent;
	int m_nDfuMode;
	TCHAR m_szDfuModeInfo[MAX_PATH];

	int	m_nCurWebFileTotalLen;	//��ǰҪ���������ص��ļ��ܳ���
	//ȡ��ǰҪ���������ص��ļ��ܳ���,����-1�򲻳ɹ�
	int GetWebFileTotalLen();
	//ȡnItem��Ҫ���������ص��ļ�����,����-1,-2�򲻳ɹ�
	int GetWebFileLen(int nItem);
	//
	void SetLamp(CCircleButton *pBtn,int nLampID);

	void UpdateParam(BOOL bSaveToTheApp=TRUE);
	void SetProgressInfo(int nItem,int pos);
	void ClearItemInfo(int nItem=-1);
	void EnableControls();
	//���ļ����еĵ�ǰ����Ŀ¼��ȥ�����õ�pWnd����,�Ա���
	void SetFileNameNoCurrentDir(CWnd *pWnd,LPCTSTR strFileName);

	BOOL IsExistLibusbk();
	BOOL DoCheckDriverInstall();
	BOOL DoCheckDfuMode(BOOL bPrompt=TRUE);
	BOOL DoInstallLibusbk();
	BOOL MakeUpdateCmd();
#if 0
	void DoHelp();
#endif
	void DoDfuHelp();

	volatile int m_nDownloadItem;
	volatile BOOL m_bExit;
	volatile BOOL m_bDownloading;
	void DoDownload(int nItem);
	static UINT DownloadFileThread(LPVOID pParam);
	BOOL DownloadFile(BOOL bCheckIsExist=FALSE);

	void DoBrowseFile(int nItem);

	volatile int m_nUpdateItem;
	volatile BOOL m_bUpdating;
	BOOL DoUpdate(int nItem,BOOL bRunThread=TRUE);
	static UINT UpdateFileThread(LPVOID pParam);
//	BOOL UpdateFile();
//	volatile int m_nUpdateItem;
	volatile BOOL m_bAutoUpdating;
	volatile int m_nAutoUpdateItemN;
	static UINT AutoUpdateThread(LPVOID pParam);
	BOOL AutoUpdateFile();
#ifdef USE_ONEKEY_DOWNLOAD
	static UINT AutoFixcubeThread(LPVOID pParam);
	BOOL DoFixcube();
//	BOOL AutoUpdateFile();
	//���б������ʾ��pszHasNameIn�����豸,��pszHasNameIn=NULL,����ʾ����,�����б������
	int display_devices(struct wdi_device_info *list,char *pszHasNameIn=NULL);
	struct wdi_device_info* GetItemDevice(int nIdx);

	struct wdi_options_install_driver m_stIdOptions;
	struct wdi_device_info *m_pstDevice, *m_pstListDev;
	struct wdi_options_create_list m_stClOptions; // = { 0 };
	int m_nCurDevIndex;
	int DoListAll();

	CArray<SSerInfo,SSerInfo&> m_arrSI;
	int DoScanComm();
	BOOL SwitchToDfuMode();		//�л���DFUģʽ
	//��װ(Photon with WiFiʱ)usb2com��(Photon DFU Modeʱ��)libusbk����
	BOOL DoInstallDriver(wdi_device_info *dev);
	//����Ƿ���Dfuģʽ������������װ
	BOOL CheckInstallDfuModeDriver();
	//����Ƿ���Usb2com������������װ
	BOOL CheckInstallUsb2comDriver();
#define DESCNAME_DFUMODE	"Photon DFU Mode"
#define DESCNAME_USBSER		"Photon with WiFi"
	int CheckDescNameIsInDevList(char descname[]);
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	virtual void OnOK();
	virtual void OnCancel();

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedBtnInstalllibusbk();
	afx_msg void OnBnClickedBtnAuto();
	afx_msg void OnBnClickedBtnBrowse1();
	afx_msg void OnBnClickedBtnBrowse2();
	afx_msg void OnBnClickedBtnBrowse3();
	afx_msg void OnBnClickedBtnDownload1();
	afx_msg void OnBnClickedBtnDownload2();
	afx_msg void OnBnClickedBtnDownload3();
	afx_msg void OnBnClickedBtnUpdate1();
	afx_msg void OnBnClickedBtnUpdate2();
	afx_msg void OnBnClickedBtnUpdate3();
#if 0
	afx_msg void OnDfuHelp();
	afx_msg void OnInstallLibusbHelp();
#endif
	afx_msg LRESULT OnMyPrintMessage(WPARAM wParam,LPARAM lParam);
	afx_msg LRESULT OnMySetProgressMessage(WPARAM wParam,LPARAM lParam);
	afx_msg LRESULT OnMyDfuModeMessage(WPARAM wParam,LPARAM lParam);

#ifdef USE_ONEKEY_DOWNLOAD
	afx_msg void OnBnClickedBtnFixcube();
	afx_msg void OnBnClickedBtnListAll();
	afx_msg void OnBnClickedBtnToDfuMode();
	afx_msg void OnBnClickedBtnInstall();
	afx_msg void OnNMClickSyslink(NMHDR *pNMHDR, LRESULT *pResult);
#endif
	afx_msg void OnTimer(UINT_PTR nIDEvent);
};
