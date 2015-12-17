
// DfuToolDlg.cpp : implementation file
//

#include "stdafx.h"
#include "DfuTool.h"
#include "DfuToolDlg.h"
#include <afxinet.h>
#include <io.h>
#include "registry.h"
//#include "DfuHelpView.h"

//#define USE_MYDEBUG

#ifdef _DEBUG
#define new DEBUG_NEW
#endif
#ifdef USE_ONEKEY_DOWNLOAD
#include "comm.h"	//加串口是为切换到DFU模式

#define BPS_SWITCHTODFU		14400
CComm gl_Comm;		//用于切换到DFU,以14400波特率打开串口，实现切换
/*
 * Converts a name + ext UTF-8 pair to a valid MS filename.
 * Returned string is allocated and needs to be freed manually
 */
char* to_valid_filename(char* name, char* ext);
char* to_valid_filename(char* name, char* ext)
{
	size_t i, j, k;
	BOOL found;
	char* ret;
	wchar_t unauthorized[] = L"\x0001\x0002\x0003\x0004\x0005\x0006\x0007\x0008\x000a"
		L"\x000b\x000c\x000d\x000e\x000f\x0010\x0011\x0012\x0013\x0014\x0015\x0016\x0017"
		L"\x0018\x0019\x001a\x001b\x001c\x001d\x001e\x001f\x007f\"*/:<>?\\|,";
	wchar_t to_underscore[] = L" \t";
	wchar_t *wname, *wext, *wret;

	if ((name == NULL) || (ext == NULL)) {
		return NULL;
	}

	if (strlen(name) > WDI_MAX_STRLEN) return NULL;

	// Convert to UTF-16
	wname = utf8_to_wchar(name);
	wext = utf8_to_wchar(ext);
	if ((wname == NULL) || (wext == NULL)) {
		safe_free(wname); safe_free(wext); return NULL;
	}

	// The returned UTF-8 string will never be larger than the sum of its parts
	wret = (wchar_t*)calloc(2*(wcslen(wname) + wcslen(wext) + 2), 1);
	if (wret == NULL) {
		safe_free(wname); safe_free(wext); return NULL;
	}
	wcscpy(wret, wname);
	safe_free(wname);
	wcscat(wret, wext);
	safe_free(wext);

	for (i=0, k=0; i<wcslen(wret); i++) {
		found = FALSE;
		for (j=0; j<wcslen(unauthorized); j++) {
			if (wret[i] == unauthorized[j]) {
				found = TRUE; break;
			}
		}
		if (found) continue;
		found = FALSE;
		for (j=0; j<wcslen(to_underscore); j++) {
			if (wret[i] == to_underscore[j]) {
				wret[k++] = '_';
				found = TRUE; break;
			}
		}
		if (found) continue;
		wret[k++] = wret[i];
	}
	wret[k] = 0;
	ret = wchar_to_utf8(wret);
	safe_free(wret);
	return ret;
}
#endif

#define MPTMSG_WORKINFO		11

#define PRINTPROMPT(...) do { char ss[2048];\
	sprintf(ss, __VA_ARGS__); SendMessage(MY_PRINT_MESSAGE,1,(LPARAM)ss); } while (0)

#define PRINTWORKINFO(...) do { static TCHAR ss[2048];\
	sprintf(ss, __VA_ARGS__); SendMessage(MY_PRINT_MESSAGE,MPTMSG_WORKINFO,(LPARAM)ss); } while (0)

// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()


// CDfuToolDlg dialog


CDfuToolDlg::CDfuToolDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CDfuToolDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	int fw,fh;
// 	m_Font1.CreateFont(14,6, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET,
// 		OUT_DEFAULT_PRECIS, CLIP_CHARACTER_PRECIS, ANTIALIASED_QUALITY,
// 		DEFAULT_PITCH | FF_DONTCARE, _T("Arial")); //Microsoft Sans Serif
#ifdef USE_ONEKEY_DOWNLOAD
	fw=10; fh=20;
#else
	fw=6; fh=15;
#endif
	m_Font1.CreateFont(fh, fw, 0, 0, FW_NORMAL-100, 0, 0, 0, ANSI_CHARSET, OUT_TT_PRECIS, 
		CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, VARIABLE_PITCH | FF_SWISS,
		_T("Arial"));
// 	m_Font2.CreateFont(20,10, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET,
// 		OUT_DEFAULT_PRECIS, CLIP_CHARACTER_PRECIS, ANTIALIASED_QUALITY,
// 		DEFAULT_PITCH | FF_DONTCARE, _T("Arial"));
#ifdef USE_ONEKEY_DOWNLOAD
	fw=16; fh=32;
#else
	fw=14; fh=30;
#endif
	m_Font2.CreateFont(fh,fw, 0, 0, FW_MEDIUM, 0, 0, 0, ANSI_CHARSET, OUT_TT_PRECIS, 
		CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, VARIABLE_PITCH | FF_SWISS,
		_T("Arial"));
	m_Font3.CreateFont(16, 8, 0, 0, FW_NORMAL, 0, 0, 0, ANSI_CHARSET, OUT_TT_PRECIS, 
		CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, VARIABLE_PITCH | FF_SWISS,
		_T("Courier New"));

	m_pHelpFrame=NULL;
	m_pDfuHelpFrame=NULL;

	m_bExit=FALSE;
	m_bDownloading=FALSE;
	m_nDownloadItem=-1;

	m_nUpdateItem=-1;
	m_bUpdating=FALSE;
	m_bAutoUpdating=FALSE; m_nAutoUpdateItemN=0;

	m_nDfuMode=MDMSG_NO;
	m_szDfuModeInfo[0]=0;

	m_nCurWebFileTotalLen=0;	//当前要从网上下载的文件总长度

	m_hWaitEvent=CreateEvent(NULL,TRUE,FALSE,NULL);	

#ifdef USE_ONEKEY_DOWNLOAD
	memset(&m_stIdOptions,0,sizeof(m_stIdOptions));
	m_pstDevice=NULL; m_pstListDev=NULL;
	memset(&m_stClOptions,0,sizeof(m_stClOptions));
	m_nCurDevIndex=-1;
#endif	
}

void CDfuToolDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STATIC_TITLE, m_LabelTitle);
	DDX_Control(pDX, IDC_STATIC_NOTE, m_LabelNote);
	DDX_Control(pDX, IDC_PROGRESS1, m_ProgressCtrl);
	DDX_Control(pDX, IDC_EDIT_PROMPT, m_EditPrompt);
	
	DDX_Control(pDX, IDC_BTN_DFULAMP, m_btnLampDfu);
	for(int i=0;i<1+MAX_UPDATE_NUM;i++)
	{
		DDX_Control(pDX, IDC_BTN_AUTOLAMP+i, m_btnLampUpdate[i]);
	}
	for(int i=0;i<MAX_UPDATE_NUM;i++)
	{
		DDX_Control(pDX, IDC_EDIT1+i, m_EditUpdateFile[i]);
	}
#ifdef USE_ONEKEY_DOWNLOAD
	DDX_Control(pDX, IDC_SYSLINK1, m_LinkCtrl1);
	DDX_Control(pDX, IDC_SYSLINK2, m_LinkCtrl2);
#endif	
}

BEGIN_MESSAGE_MAP(CDfuToolDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_BTN_INSTALLLIBUSBK, &CDfuToolDlg::OnBnClickedBtnInstalllibusbk)
	ON_BN_CLICKED(IDC_BTN_AUTO, &CDfuToolDlg::OnBnClickedBtnAuto)
	ON_BN_CLICKED(IDC_BTN_BROWSE1, &CDfuToolDlg::OnBnClickedBtnBrowse1)
	ON_BN_CLICKED(IDC_BTN_BROWSE2, &CDfuToolDlg::OnBnClickedBtnBrowse2)
	ON_BN_CLICKED(IDC_BTN_BROWSE3, &CDfuToolDlg::OnBnClickedBtnBrowse3)
	ON_BN_CLICKED(IDC_BTN_DOWNLOAD1, &CDfuToolDlg::OnBnClickedBtnDownload1)
	ON_BN_CLICKED(IDC_BTN_DOWNLOAD2, &CDfuToolDlg::OnBnClickedBtnDownload2)
	ON_BN_CLICKED(IDC_BTN_DOWNLOAD3, &CDfuToolDlg::OnBnClickedBtnDownload3)
	ON_BN_CLICKED(IDC_BTN_UPDATE1, &CDfuToolDlg::OnBnClickedBtnUpdate1)
	ON_BN_CLICKED(IDC_BTN_UPDATE2, &CDfuToolDlg::OnBnClickedBtnUpdate2)
	ON_BN_CLICKED(IDC_BTN_UPDATE3, &CDfuToolDlg::OnBnClickedBtnUpdate3)

#if 0
	ON_COMMAND(IDM_DFUHELP,OnDfuHelp)
	ON_COMMAND(IDM_INSTALLLIBUSBHELP,OnInstallLibusbHelp)
#endif
	ON_MESSAGE(MY_PRINT_MESSAGE, OnMyPrintMessage)
	ON_MESSAGE(MY_PROGRESS_MESSAGE, OnMySetProgressMessage)
	ON_MESSAGE(MY_DFUMODE_MESSAGE, OnMyDfuModeMessage)
#ifdef USE_ONEKEY_DOWNLOAD
	ON_BN_CLICKED(IDC_BTN_FIXCUBE, OnBnClickedBtnFixcube)
	ON_BN_CLICKED(IDC_BTN_LISTALL, OnBnClickedBtnListAll)
	ON_BN_CLICKED(IDC_BTN_INSTALL, OnBnClickedBtnInstall)
	ON_BN_CLICKED(IDC_BTN_TODFUMODE, OnBnClickedBtnToDfuMode)
	ON_NOTIFY(NM_CLICK, IDC_SYSLINK1, OnNMClickSyslink)
	ON_NOTIFY(NM_CLICK, IDC_SYSLINK2, OnNMClickSyslink)
#endif
	ON_WM_TIMER()
END_MESSAGE_MAP()


// CDfuToolDlg message handlers

BOOL CDfuToolDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

#ifdef USE_ONEKEY_DOWNLOAD
	GetDlgItem(IDC_BTN_FIXCUBE)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_SYSLINK1)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_SYSLINK2)->ShowWindow(SW_SHOW);
	m_LinkCtrl1.SetWindowText("Cube Doctor is released by LookingGlass Co.,Ltd.<a href=\"http://www.cubetube.org\">http://www.cubetube.org</a>");
	m_LinkCtrl2.SetWindowText("Cube Doctor is released under the GPLv3. Source is available from <a href=\"https://github.com/enjrolas/CubeDoctor\">https://github.com/enjrolas/CubeDoctor</a>");
	CButton *pbtn=(CButton *)GetDlgItem(IDC_BTN_FIXCUBE);
	pbtn->SetFont(&m_Font1);
	GetDlgItem(IDC_STATIC_WORKINFO)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_STATIC_WORKINFO)->SetWindowText(NULL);//_T("OK"));
	GetDlgItem(IDC_STATIC_CURFILE)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_STATIC_CURFILE)->SetWindowText(NULL);//_T("OK"));
#ifdef USE_MYDEBUG
	UINT idv[]={IDC_COMBO_DEVLIST,IDC_COMBO_COMM,IDC_BTN_LISTALL,IDC_BTN_TODFUMODE,IDC_BTN_INSTALL};
	ShowControl(this,idv,sizeof(idv)/sizeof(UINT),TRUE);
#endif
#endif
	m_LabelTitle.SetFont(&m_Font2);
	m_LabelTitle.SetTextColor(RGB(41,21,170));
	m_LabelNote.SetFont(&m_Font1);
	m_LabelNote.SetTextColor(RGB(170,41,21));
	m_EditPrompt.SetFont(&m_Font3);

	TCHAR strNote[]=_T("Note: \r\n\
1.make sure libusbK driver is installed on your compupter properly. Not installed? click here for instruction\r\n\
2.make sure your photon is in DFU mode(blink yellow). \r\n\
   Not blink yellow? press and hole SETUP key, and tack RESET key until you see it blink yellow\r\n\
3.Now you can click AUTO to download the latest Firmware and demo loop and update to your Photo automatically\
 or mannually");
// 	3.Now you can click AUTO to download the latest Firmware and demo loop and update to your Photo Automatically\r\n\
// 		Or you can download the firmware and demo loop mannually");
	m_LabelNote.SetWindowText(strNote);

//	SetLamp(&m_btnLampAuto,0);
	for (int i=0;i<MAX_UPDATE_NUM+1;i++)
	{
		SetLamp(&m_btnLampUpdate[i],IDB_LAMPGRAY);
	}
	SetLamp(&m_btnLampDfu,m_nDfuMode==MDMSG_DFU?IDB_LAMPYELLOW24:IDB_LAMPGRAY24);
	
	UpdateParam(FALSE);
//	SetTimer(1,1000,NULL);
#ifdef USE_ONEKEY_DOWNLOAD
	OnBnClickedBtnListAll();
#endif
	return TRUE;  // return TRUE  unless you set the focus to a control
}
#ifdef USE_ONEKEY_DOWNLOAD
//在列表框里显示有pszHasNameIn串的设备,若pszHasNameIn=NULL,则显示所有,返回列表框项数
int CDfuToolDlg::display_devices(struct wdi_device_info *list,char *pszHasNameIn)
{
	char szDev[MAX_PATH];
	char szName[MAX_PATH];
//	CString strTemp;
	struct wdi_device_info *dev;
	int index = -1;
	szName[0]=0;
	if(pszHasNameIn)
	{
//		strName.Format(_T("%s"),pszHasNameIn);	strName.MakeLower();
		safe_strcpy(szName,MAX_PATH-1,pszHasNameIn);
		strlwr(szName);
	}
	CComboBox *pCombo=(CComboBox *)GetDlgItem(IDC_COMBO_DEVLIST);
	if(pCombo==NULL)
		return 0;
	pCombo->ResetContent();
	m_nCurDevIndex=-1;
	if(list==NULL)
		return 0;
	for (dev = list; dev != NULL; dev = dev->next) {
		// Compute the width needed to accommodate our text
		if (dev->desc == NULL) {
//			dprintf("error: device description is empty");
			break;
		}
		BOOL bFind=FALSE;
		if(pszHasNameIn)
		{
//			strTemp.Format(_T("%s"),dev->desc);	//desc里有汉字的,后面用到Find(strName)可能出错
			safe_strcpy(szDev,MAX_PATH-1,dev->desc);
			//当desc里有汉字的,下面可能出错
// 			strTemp.MakeLower();
// 			if(strTemp.Find(strName)<0)	//没找到串则不加入
// 				continue;
			strlwr(szDev);
			if(strstr(szDev,szName)==NULL)
				continue;
 			bFind=TRUE;
		}
// 		strTemp.Format(_T("%s"),dev->desc);
// 		index = pCombo->AddString(strTemp);
		safe_strcpy(szDev,MAX_PATH-1,dev->desc);
		index = pCombo->AddString(szDev);
		if(index<0)
			continue;
		if(bFind || pszHasNameIn==NULL)
			m_nCurDevIndex=index;
		pCombo->SetItemData(index,(LPARAM)dev);
		// Select by Hardware ID if one's available
// 		if (safe_strcmp(current_device_hardware_id, dev->hardware_id) == 0) {
// 			current_device_index = index;
// 			safe_free(current_device_hardware_id);
// 		}
	}
	if(pCombo->GetCount()==0)
		return 0;
#if 0
	index=pCombo->FindString(0,"Photon with WiFi");
	if(index<0)
		index=pCombo->FindString(0,"Photon");
	if(index>=0)
		m_nCurDevIndex=index;
//	PrintMsgInfo("%d",index);
// 	_IGNORE(ComboBox_SetCurSel(hDeviceList, current_device_index));
	// Set the width to computed value
// 	SendMessage(hDeviceList, CB_SETDROPPEDWIDTH, max_width, 0);
#endif
	pCombo->SetCurSel(m_nCurDevIndex);
	return pCombo->GetCount();
//	return index;
}
void CDfuToolDlg::OnBnClickedBtnListAll()
{
	DoListAll();
	DoScanComm();
// 	if(IsExistLibusbk())
// 		PrintMsgInfo("Is Exist Libusbk");
}
int CDfuToolDlg::DoListAll()
{
	m_stClOptions.list_all=true;
	m_stClOptions.trim_whitespaces=true;
	if(m_pstListDev!=NULL)
	{
		wdi_destroy_list(m_pstListDev);
	}
	m_nCurDevIndex=-1;
	int r = wdi_create_list(&m_pstListDev, &m_stClOptions);
	if (r != WDI_SUCCESS) 
		return 0;
	int nm=display_devices(m_pstListDev,"photon");//PrintMsgInfo(_T("%s"),list->driver);

#if 0 //def USE_MYDEBUG
	CString strTemp,ss;
	wdi_device_info *device=GetItemDevice(m_nCurDevIndex);
	if(device==NULL)
		return nm;
	BOOL replace_driver = (device->driver != NULL);
	if (replace_driver) {
		if (device->driver_version == 0) {
			strTemp=device->driver;
		} else {
			strTemp.Format(_T("%s (v%d.%d.%d.%d)"), device->driver,
				(int)((device->driver_version>>48)&0xffff), (int)((device->driver_version>>32)&0xffff),
				(int)((device->driver_version>>16)&0xffff), (int)(device->driver_version & 0xffff));
		}
	} else {
		strTemp=_T("(NONE)");
	}
	// Display the VID,PID,MI
	ss.Format(_T(" VID=%04X,PID=%04X"),device->vid,device->pid);
	strTemp+=ss;
	if(device->is_composite)
	{
		ss.Format(_T(",MI=%02X"),device->mi);
		strTemp+=ss;
	}
	SetDlgItemText(IDC_STATIC_WORKINFO,strTemp);
#endif
	return nm;
}
int CDfuToolDlg::DoScanComm() 
{
	int i,m=0;
	CString strTemp;
	m_arrSI.RemoveAll();
	CComboBox *pCombo=(CComboBox *)GetDlgItem(IDC_COMBO_COMM);
	if(pCombo==NULL)
		return 0;
	pCombo->ResetContent();
	// Populate the list of serial ports.
	if(EnumSerialPorts(m_arrSI,FALSE/*include all*/))
	{
		int n=m_arrSI.GetSize();
		for (i=0; i<n; i++) 
		{
			//				strTemp.Format(_T("COM%d"),asi[i].nComPort);
			strTemp=m_arrSI[i].strFriendlyName.MakeLower();
			if(strTemp.Find(_T("photon"))<0)	//有photon 标识的串口
				continue;
			int idx=pCombo->AddString(m_arrSI[i].strPortName); //strTemp);
			pCombo->SetItemData(idx,m_arrSI[i].nComPort);
	//				PrintMsgInfo(_T("<%s>\n%s"),asi[i].strFriendlyName,asi[i].strPortDesc);
			m++;
		}
	}
	if(m>0) // theApp.m_nCommPort==m_CombSelComm.GetItemData(i))
	{
		pCombo->SetCurSel(0);
	}
	return m;
}
//切换到DFU模式
BOOL CDfuToolDlg::SwitchToDfuMode()
{
	CComboBox *pCombo=(CComboBox *)GetDlgItem(IDC_COMBO_COMM);
	int i,n=pCombo->GetCount();
	if(n==0)
		return FALSE;
	int nComPort;
	TCHAR ss[100];
	CString strTemp;
	for (i=0; i<n; i++) 
	{
		nComPort=pCombo->GetItemData(i);
		strTemp.Format("%d,n,8,1",BPS_SWITCHTODFU);
		sprintf(ss,"COM%d",nComPort);
		gl_Comm.OpenComm(ss,strTemp);
	}
	gl_Comm.CloseCommPort();
	return TRUE;
}
void CDfuToolDlg::OnBnClickedBtnToDfuMode()
{
	BOOL bDfuMode=FALSE;
	bDfuMode=DoCheckDfuMode(FALSE);
	if(!bDfuMode)
	{
		DoListAll();	
		int n=DoScanComm();	//扫描串口
		if(n>0)
		{
			if(SwitchToDfuMode())
			{
				Sleep(1000);
				DoListAll();	
				n=DoScanComm();	//扫描串口
				bDfuMode=DoCheckDfuMode(FALSE);
			}
		}
	}
	if(bDfuMode)
	{
		PrintMsgInfo(_T("Now Dfu Mode."));
	}
}
void CDfuToolDlg::OnBnClickedBtnInstall()
{
	wdi_device_info *dev=GetItemDevice(m_nCurDevIndex);
	if(dev==NULL)
		return;
	DoInstallDriver(dev);
}
//检查是否有Dfu模式的驱动，无则安装
BOOL CDfuToolDlg::CheckInstallDfuModeDriver()
{
// 	wdi_device_info *dev=GetItemDevice(m_nCurDevIndex);
	const char* driver_display_name[WDI_NB_DRIVERS] = { "WinUSB", "libusb-win32", "libusbK", "Custom (extract only)" };
//	char descname[]="Photon with WiFi";
	char descname[]="Photon DFU Mode";
	struct wdi_device_info *dev = NULL;
	CComboBox *pCombo=(CComboBox *)GetDlgItem(IDC_COMBO_DEVLIST);
	CString strTemp;
	int nm=pCombo->GetCount();
	for (int i=0;i<nm;i++)
	{
		dev = (struct wdi_device_info*)pCombo->GetItemData(i);
		if(dev==NULL)
			continue;
		if(strcmp(dev->desc,descname)!=0)
			continue;
		if(dev->driver==NULL)	//没有则安装
		{
			return DoInstallDriver(dev);
		}
		strTemp=dev->driver;
		if(strTemp.Find(driver_display_name[2])<0)
		{
			return DoInstallDriver(dev);
		}
	}
	return FALSE;
}
//	char descname[]="Photon with WiFi";
int CDfuToolDlg::CheckDescNameIsInDevList(char descname[])
{
	//	char descname[]="Photon DFU Mode";
	struct wdi_device_info *dev = NULL;
	CComboBox *pCombo=(CComboBox *)GetDlgItem(IDC_COMBO_DEVLIST);
	CString strTemp;
	int nm=pCombo->GetCount();
	int m=0;
	for (int i=0;i<nm;i++)
	{
		dev = (struct wdi_device_info*)pCombo->GetItemData(i);
		if(dev==NULL)
			continue;
		if(strcmp(dev->desc,descname)!=0)
			continue;
		m++;
//		return TRUE;
	}
	return m;
}
//检查是否有Usb2com的驱动，无则安装
BOOL CDfuToolDlg::CheckInstallUsb2comDriver()
{
	const char* driver_display_name="usbser";
	char descname[]="Photon with WiFi";
//	char descname[]="Photon DFU Mode";
	struct wdi_device_info *dev = NULL;
	CComboBox *pCombo=(CComboBox *)GetDlgItem(IDC_COMBO_DEVLIST);
	CString strTemp;
	int nm=pCombo->GetCount();
	int m=0;
	for (int i=0;i<nm;i++)
	{
		dev = (struct wdi_device_info*)pCombo->GetItemData(i);
		if(dev==NULL)
			continue;
		if(strcmp(dev->desc,descname)!=0)
			continue;
		if(dev->driver==NULL)	//没有则安装
		{
			return DoInstallDriver(dev);
		}
		strTemp=dev->driver;
		if(strTemp.Find(driver_display_name)<0)
		{
			return DoInstallDriver(dev);
		}
		m++;
	}
	if(m>0)
	{
		pCombo=(CComboBox *)GetDlgItem(IDC_COMBO_COMM);
		if(pCombo->GetCount()==0)	//这里没列出也重安装
			return DoInstallDriver(dev);
	}
	return FALSE;
}
//安装(Photon with WiFi时)usb2com或(Photon DFU Mode时的)libusbk驱动
BOOL CDfuToolDlg::DoInstallDriver(wdi_device_info *dev)
{
	char extraction_path[MAX_PATH];
	const char* driver_display_name[WDI_NB_DRIVERS] = { "WinUSB", "libusb-win32", "libusbK", "Custom (extract only)" };
	const char* driver_name[WDI_NB_DRIVERS-1] = { "WinUSB", "libusb0", "libusbK" };
	struct wdi_options_prepare_driver pd_options = { 0 };
//	struct wdi_options_install_driver id_options = { 0 };
	int r;
	BOOL bOK=FALSE;
	char *inf_name;
	if(strcmp(dev->desc,DESCNAME_USBSER)==0)	//安装USB2COM驱动
	{
//		PRINTWORKINFO(_T("Installing the usbser driver,for switching to DFU mode..."));
		PRINTWORKINFO(_T("The usb serial auto installing,it may take a few minutes,please waiting..."));
		pd_options.driver_type=WDI_USER;
		inf_name= to_valid_filename("photon", ".inf");
//		PrintMsgInfo("%s",inf_name); return;
	}
	else
	{
//		PRINTWORKINFO(_T("Installing the libusbK driver,for updating the firmware..."));
		PRINTWORKINFO(_T("The libusbK auto installing,it may take a few minutes,please waiting..."));
		pd_options.driver_type=WDI_LIBUSBK;
		inf_name= to_valid_filename(dev->desc, ".inf");
	}
	if (inf_name == NULL) 
		return FALSE;
	//	pd_options.driver_type=WDI_LIBUSBK;
	pd_options.disable_cat=FALSE;
	pd_options.use_wcid_driver=FALSE;
	pd_options.disable_signing=TRUE;
	pd_options.cert_subject=NULL;
	char *tmp =getenvU("USERPROFILE");
	safe_sprintf(extraction_path, sizeof(extraction_path), "%s\\usb_driver", tmp);
//	PrintMsgInfo("type=%d",pd_options.driver_type);
	r = wdi_prepare_driver(dev, extraction_path, inf_name, &pd_options);
	if (r == WDI_SUCCESS) { //return;
//		dsprintf("Successfully extracted driver files.");
		// Perform the install if not extracting the files only
		if (1) //(pd_options.driver_type != WDI_USER ) ) 
		{
// 			if ( (get_driver_type(dev) == DT_SYSTEM)
// 				&& (MessageBoxA(hMain, "You are about to modify a system driver.\n"
// 				"Are you sure this is what you want?", "Warning - System Driver",
// 				MB_YESNO | MB_ICONWARNING | MB_DEFBUTTON2) == IDNO) ) {
// 					r = WDI_ERROR_USER_CANCEL; goto out;
// 			}
			//开始安装驱动 bywbj
//			dsprintf("Installing driver. Please wait...");
			m_stIdOptions.hWnd =NULL; //m_hWnd;
			r = wdi_install_driver(dev, extraction_path, inf_name, &m_stIdOptions);
			// Switch to non driverless-only mode and set hw ID to show the newly installed device
// 			current_device_hardware_id = (dev != NULL)?safe_strdup(dev->hardware_id):NULL;
// 			if ((r == WDI_SUCCESS) && (!cl_options.list_all) && (!pd_options.use_wcid_driver)) {
// 				toggle_driverless(FALSE);
// 			}
//			PostMessage(WM_DEVICECHANGE, 0, 0);	// Force a refresh
			bOK=(r == WDI_SUCCESS);
			safe_free(inf_name);
		}
	} else {
//		dsprintf("Could not extract files");
	}
	safe_free(inf_name);
	if(bOK)
	{
		PRINTWORKINFO(_T("The driver installation successful."));
	}
	else
	{
//		PRINTWORKINFO(_T("The driver(libusbK) installation failed!"));
//		PRINTWORKINFO(_T(" "));
	}
	return bOK;
}
struct wdi_device_info* CDfuToolDlg::GetItemDevice(int nIdx)
{
	struct wdi_device_info *dev = NULL;
	CComboBox *pCombo=(CComboBox *)GetDlgItem(IDC_COMBO_DEVLIST);
	if(pCombo==NULL || pCombo->GetCount()==0 || nIdx<0||nIdx>=pCombo->GetCount())
		return dev;
	dev = (struct wdi_device_info*)pCombo->GetItemData(nIdx);
	return dev;
}

#endif

void CDfuToolDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CDfuToolDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CDfuToolDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CDfuToolDlg::SetLamp(CCircleButton *pBtn,int nLampID)
{
	if(pBtn==NULL)
		return;
	if(nLampID!=0)
		pBtn->SetButtonBitmap(nLampID);
	else
		pBtn->SetButtonBitmap(IDB_LAMPGREEN16);
	pBtn->Invalidate();
}

void CDfuToolDlg::OnBnClickedBtnInstalllibusbk()
{
// 	IsExistLibusbk(); 	return;
// 	int m=GetDfuMode(m_hWnd,m_szDfuModeInfo,sizeof(m_szDfuModeInfo)-1);
// 	PrintMsgInfo("%d",m); return;
//	PrintPrompt("[>] 安装USB驱动.. ");
	DoInstallLibusbk();
}
BOOL CDfuToolDlg::DoInstallLibusbk()
{
	CString strFile;
	int nWinVer;
	const char *pp=PrintWinVersion(nWinVer);
#if 0
	strFile.Format("%d,%s",nWinVer,pp);
	m_EditPrompt.SetWindowText(strFile); return;
#endif
	if(nWinVer<=WINDOWS_XP)
		strFile.Format(_T("%s\\%s"),theApp.m_szRunDir,"zadig_xp_2.1.2.exe");//g_strLibusbk);
	else
		strFile.Format(_T("%s\\%s"),theApp.m_szRunDir,"zadig_2.1.2.exe");//g_strLibusbk);
	if(_access((const char *)strFile,0)!=0)	//文件不存在
	{
		PrintMsgWarning(_T("Not found the driver installation file:\n%s"),strFile);
		return FALSE;
	}

	//	PrintMsgWarning(strFile);
	HINSTANCE rt=ShellExecute(NULL,_T("open"),strFile,NULL,NULL,SW_SHOWNORMAL); 
	if((int)rt>32)
	{
		//DoHelp();
		PostMessage(WM_COMMAND,IDM_INSTALLLIBUSBHELP);
	}
	else
		return FALSE;
//	PrintMsgInfo("%d",rt);
// 	strFile.Format(_T("%s\\Readme.docx"),theApp.m_szRunDir);
// 	ShellExecute(NULL,_T("open"),strFile,NULL,NULL,SW_SHOWNORMAL); 

// 	if (Is_X64())
// 		system("CESG502\\dpinst64.exe") ; // /SW
// 	else
// 		system("CESG502\\dpinst32.exe /LM") ;// /SW
	return TRUE;
}
#if 0
void CDfuToolDlg::DoDfuHelp()
{
	if(m_pDfuHelpFrame==NULL)
	{
		CSize m_sizeBrowseDataWin(480+20,680);

		m_pDfuHelpFrame = new CHelpFrame;
		CCreateContext cc;
		cc.m_pCurrentDoc = NULL;
		cc.m_pCurrentFrame = m_pDfuHelpFrame;
		cc.m_pNewViewClass = RUNTIME_CLASS(CDfuHelpView);

		CRect wrc,rc;
		GetDesktopWindow()->GetWindowRect(&wrc);
//		m_sizeBrowseDataWin.cy=wrc.Height();
		//rc.left=(wrc.left+wrc.right-m_sizeBrowseDataWin.cx)/2; 
		rc.right=rc.left+m_sizeBrowseDataWin.cx;
		rc.top=(wrc.top+wrc.bottom-m_sizeBrowseDataWin.cy)/2; rc.bottom=rc.top+m_sizeBrowseDataWin.cy;
//			WS_MINIMIZEBOX 
		m_pDfuHelpFrame->Create(NULL, _T("DfuMode Help"), WS_OVERLAPPED|WS_CAPTION|WS_SYSMENU|WS_BORDER,rc, NULL, NULL, 0, &cc); // CFrameWnd::rectDefault
		//	pFrame->GetActiveView()->OnInitialUpdate();
		m_pDfuHelpFrame->SendMessageToDescendants(WM_INITIALUPDATE, 0, 0, TRUE, TRUE);
		m_pDfuHelpFrame->ShowWindow(SW_SHOW);
		m_pDfuHelpFrame->UpdateWindow();
		m_pDfuHelpFrame->MoveWindow(&rc);
		//		m_pDfuHelpFrame->CenterWindow();
	}
	else
	{
		if(m_pDfuHelpFrame->IsIconic())
		{
			m_pDfuHelpFrame->ShowWindow(SW_NORMAL);
		}
		else
		{
			if(!m_pDfuHelpFrame->IsWindowVisible())
				m_pDfuHelpFrame->ShowWindow(SW_SHOW);
			m_pDfuHelpFrame->BringWindowToTop();
		}
	}
}
void CDfuToolDlg::OnDfuHelp()
{
	DoDfuHelp();
}
#endif
#if 0
void CDfuToolDlg::OnInstallLibusbHelp()
{
	DoHelp();
}
void CDfuToolDlg::DoHelp()
{
	if(m_pHelpFrame==NULL)
	{
		CSize m_sizeBrowseDataWin(700,600);

		m_pHelpFrame = new CHelpFrame;
		CCreateContext cc;
		cc.m_pCurrentDoc = NULL;
		cc.m_pCurrentFrame = m_pHelpFrame;
		cc.m_pNewViewClass = RUNTIME_CLASS(CHelpView);

		CRect wrc,rc;
		GetDesktopWindow()->GetWindowRect(&wrc);
		m_sizeBrowseDataWin.cy=wrc.Height();
		//rc.left=(wrc.left+wrc.right-m_sizeBrowseDataWin.cx)/2; 
		rc.right=rc.left+m_sizeBrowseDataWin.cx;
		rc.top=(wrc.top+wrc.bottom-m_sizeBrowseDataWin.cy)/2; rc.bottom=rc.top+m_sizeBrowseDataWin.cy;
		m_pHelpFrame->Create(NULL, _T("Readme"), WS_OVERLAPPEDWINDOW,rc, NULL, NULL, 0, &cc); // CFrameWnd::rectDefault
		//	pFrame->GetActiveView()->OnInitialUpdate();
		m_pHelpFrame->SendMessageToDescendants(WM_INITIALUPDATE, 0, 0, TRUE, TRUE);
		m_pHelpFrame->ShowWindow(SW_SHOW);
		m_pHelpFrame->UpdateWindow();
		m_pHelpFrame->MoveWindow(&rc);
		//		m_pHelpFrame->CenterWindow();
	}
	else
	{
		if(m_pHelpFrame->IsIconic())
		{
			m_pHelpFrame->ShowWindow(SW_NORMAL);
		}
		else
		{
			if(!m_pHelpFrame->IsWindowVisible())
				m_pHelpFrame->ShowWindow(SW_SHOW);
			m_pHelpFrame->BringWindowToTop();
		}
	}


}
#endif

void CDfuToolDlg::OnBnClickedBtnBrowse1()
{
	DoBrowseFile(0);
}

void CDfuToolDlg::OnBnClickedBtnBrowse2()
{
	DoBrowseFile(1);
}

void CDfuToolDlg::OnBnClickedBtnBrowse3()
{
	DoBrowseFile(2);
}
void CDfuToolDlg::DoBrowseFile(int nItem)
{
	CString	strFile;
	m_nDownloadItem=nItem;
	m_EditUpdateFile[nItem].GetWindowText(strFile);
	strFile.Trim();

	if(strFile.GetLength()==0)
	{
		strFile.Format(_T("%s\\*.bin"),theApp.m_szRunDir);
	}

	CString strFilter=_T("Bin File(*.bin) |*.bin||");
	CString strCaption(_T("Open Bin File"));
	CFileDialog dlg(TRUE,_T("bin"),strFile,//szFileName,
		OFN_HIDEREADONLY|OFN_PATHMUSTEXIST,//|OFN_CREATEPROMPT|OFN_OVERWRITEPROMPT,
		strFilter);
	dlg.m_ofn.lpstrTitle = strCaption;
	if(dlg.DoModal()!=IDOK)
		return;
	strFile=dlg.GetPathName();
//	m_EditUpdateFile[nItem].SetWindowText(strFile);
	SetFileNameNoCurrentDir(&m_EditUpdateFile[nItem],strFile);
}

void CDfuToolDlg::OnBnClickedBtnDownload1()
{
	if(m_bDownloading)
		m_bExit=TRUE;
	else
		DoDownload(0);
}

void CDfuToolDlg::OnBnClickedBtnDownload2()
{
	if(m_bDownloading)
		m_bExit=TRUE;
	else
		DoDownload(1);
}

void CDfuToolDlg::OnBnClickedBtnDownload3()
{
	if(m_bDownloading)
		m_bExit=TRUE;
	else
		DoDownload(2);
}
void CDfuToolDlg::DoDownload(int nItem)
{
	m_bExit=FALSE;
	m_nDownloadItem=nItem;
	SetDlgItemText(IDC_STATIC_LENINFO1+nItem,NULL);
	m_EditPrompt.SetWindowText(NULL);
	m_ProgressCtrl.SetPos(0);
//	ClearItemInfo(nItem);
	AfxBeginThread(DownloadFileThread,this);
}
static char *s_strCmd[3]={
	"dfu-util -d 2b04:d006 -a 0 -s 0x8020000 -D ",//firmware-part1.bin",
	"dfu-util -d 2b04:d006 -a 0 -s 0x8060000 -D ",//firmware-part2.bin",
	"dfu-util -d 2b04:d006 -a 0 -s 0x80A0000:leave -D ",//demo.bin"
};
static char s_strCurCmd[MAX_PATH]="";

BOOL CDfuToolDlg::DoUpdate(int nItem,BOOL bRunThread)
{
	//注意：每个下载烧录之间要有至少1秒间隔，以便下位机处理。
	m_nUpdateItem=nItem;
	UpdateParam(TRUE);
// 	m_ProgressCtrl.SetPos(0);
// 	m_EditPrompt.SetWindowText(NULL);
// 	SetDlgItemText(IDC_STATIC_LENINFO4+nItem,NULL);
	if(bRunThread)
	{
		if(!DoCheckDriverInstall())
			return FALSE;
//		UpdateParam(TRUE);
//		PRINTPROMPT("\n");
		ClearItemInfo(nItem);
		if(!DoCheckDfuMode())
			return FALSE;
		m_bExit=FALSE;
		AfxBeginThread(UpdateFileThread,this);
	}
	else
	{
		PRINTPROMPT("\n");
		MakeUpdateCmd();
		DoDfuUtil(m_hWnd,s_strCurCmd);
	}
	return TRUE;
}
BOOL CDfuToolDlg::MakeUpdateCmd()
{
	TCHAR szFileName[MAX_PATH];
	TCHAR szExtName[MAX_PATH];
	CString strPath,strFile;
	s_strCurCmd[0]=0;
	strPath=GetFolderFromFullpath(theApp.m_strUpdateFile[m_nUpdateItem],szFileName,szExtName);
	strFile.Format(_T("%s%s"),szFileName,szExtName);
	//	PrintMsgInfo("%s\xd\xa %s",strPath,strFile);
	if(strFile.GetLength()==0)
	{
//		m_EditPrompt.SetWindowText("No update file! Please select a file to update.");
		PRINTPROMPT("No update file! Please select a file to update.\n");
		return FALSE;
	}
	if(strPath.GetLength()==0)	//空的就用当前目录
		strPath=theApp.m_szRunDir;
	SetCurrentDirectory(strPath);	//更换到当前目录
	strcpy(s_strCurCmd,s_strCmd[m_nUpdateItem]);
	strcat(s_strCurCmd,strFile);
	return TRUE;
}

UINT CDfuToolDlg::UpdateFileThread(LPVOID pParam)
{
	CDfuToolDlg *pParent=(CDfuToolDlg *)pParam;
// 	if(m_nUpdateItem<0||m_nUpdateItem>=MAX_UPDATE_NUM)
// 		return FALSE;
// 	CButton* pBtn=(CButton* )pParent->GetDlgItem(IDC_BTN_UPDATE1+pParent->m_nUpdateItem);
// 	pBtn->EnableWindow(FALSE);
	//	pParent->m_EditUpdateFile[pParent->m_nUpdateItem].GetWindowText(strFile);
	pParent->m_bUpdating=TRUE;
	pParent->EnableControls();

	pParent->m_nDownloadItem=pParent->m_nUpdateItem;
	pParent->DownloadFile();//TRUE);
	pParent->UpdateParam();
	if(pParent->MakeUpdateCmd())
	{
		pParent->SendMessage(MY_PRINT_MESSAGE,1,(LPARAM)"\n");
		DoDfuUtil(pParent->m_hWnd,s_strCurCmd);
	}
	Sleep(1000);
	pParent->m_bUpdating=FALSE;
//	pBtn->EnableWindow(TRUE);
//	return TRUE;
	pParent->EnableControls();
	return 0;
}

void CDfuToolDlg::OnBnClickedBtnUpdate1()
{
	if(m_bUpdating)
		m_bExit=TRUE;
	else
		DoUpdate(0);
}

void CDfuToolDlg::OnBnClickedBtnUpdate2()
{
	if(m_bUpdating)
		m_bExit=TRUE;
	else
		DoUpdate(1);
}

void CDfuToolDlg::OnBnClickedBtnUpdate3()
{
	if(m_bUpdating)
		m_bExit=TRUE;
	else
		DoUpdate(2);
}
void CDfuToolDlg::ClearItemInfo(int nItem)
{
	int t1,t2;
	int i;
	if(nItem<=-1)
	{	
		t1=0;t2=MAX_UPDATE_NUM; 
		SetLamp(&m_btnLampUpdate[0],IDB_LAMPGRAY24);
#ifdef USE_ONEKEY_DOWNLOAD
		GetDlgItem(IDC_STATIC_WORKINFO)->SetWindowText(NULL);//_T("OK"));
		GetDlgItem(IDC_STATIC_CURFILE)->SetWindowText(NULL);//_T("OK"));
#endif
	}
	else
	{	t1=nItem; t2=t1+1; }
	m_EditPrompt.SetWindowText(NULL);
	m_ProgressCtrl.SetPos(0);
	for (i=t1;i<t2;i++)
	{
		SetLamp(&m_btnLampUpdate[i+1],IDB_LAMPGRAY);
		SetDlgItemText(IDC_STATIC_LENINFO1+i,NULL);
		SetDlgItemText(IDC_STATIC_LENINFO4+i,NULL);
	}
}
BOOL IsDirectoryExists(TCHAR *dirName)
{
	WIN32_FILE_ATTRIBUTE_DATA dataDirAttrData;
	if (!::GetFileAttributesEx(dirName, GetFileExInfoStandard, &dataDirAttrData))
	{
		DWORD lastError = ::GetLastError();
		if (lastError == ERROR_PATH_NOT_FOUND || lastError == ERROR_FILE_NOT_FOUND || lastError == ERROR_NOT_READY)
			return FALSE;
	}
	return (dataDirAttrData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
}
#if 1
/*
Windows Registry Editor Version 5.00

[HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Control\Class\{ECFB0CFD-74C4-4F52-BBF7-343461CD72AC}]
"Class"="libusbk devices"
"ClassDesc"="libusbK USB Devices"
@="libusbK USB Devices"
"Icon"="-20"
"IconPath"=hex(7):43,00,3a,00,5c,00,57,00,69,00,6e,00,64,00,6f,00,77,00,73,00,\
5c,00,73,00,79,00,73,00,74,00,65,00,6d,00,33,00,32,00,5c,00,53,00,45,00,54,\
00,55,00,50,00,41,00,50,00,49,00,2e,00,64,00,6c,00,6c,00,2c,00,2d,00,32,00,\
30,00,00,00,00,00

[HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Control\Class\{ECFB0CFD-74C4-4F52-BBF7-343461CD72AC}\0000]
"InfPath"="oem90.inf"
"InfSection"="LUsbK_Device"
"InfSectionExt"=".NTAMD64"
"ProviderName"="libusbK"
"DriverDateData"=hex:00,40,eb,bf,f9,0c,d0,01
"DriverDate"="12-1-2014"
"DriverVersion"="3.0.7.0"
"MatchingDeviceId"="usb\\vid_2b04&pid_c006"
"DriverDesc"="Photon with WiFi"
*/
BOOL CDfuToolDlg::IsExistLibusbk()
{
	CRegistry	m_RegTab;
	//	BOOL bOK=TRUE;
	WCHAR szSubKName[MAX_PATH];
	WCHAR szKName[]=L"SYSTEM\\CurrentControlSet\\Control\\Class\\{ECFB0CFD-74C4-4F52-BBF7-343461CD72AC}";
	WCHAR szValue[200];
	WCHAR ss[]=L"libusbk devices";//Universal Serial Bus devices";
	TCHAR s1[100];
	BOOL bb=m_RegTab.Open(HKEY_LOCAL_MACHINE,szKName);
	if(!bb)
		return FALSE;
	bb=m_RegTab.ReadString(L"Class",szValue,100);
	if(bb && memcmp(ss,szValue,sizeof(ss)-2)==0)
	{
		CStringArray strArraySubKey;
		m_RegTab.EnumSubKey(strArraySubKey);
		int n=strArraySubKey.GetCount();
		m_RegTab.Close();
//		PrintMsgInfo("%d",n);
		for (int i=0;i<n;i++)
		{
			_tcscpy(s1,strArraySubKey.GetAt(i));
			szValue[0]=0;
			for(int j=0;;j++)
			{
				szValue[j]=s1[j];
				if(s1[j]==0)
					break;
			}
			wsprintfW(szSubKName,L"%s\\%s",szKName,szValue);
			bb=m_RegTab.Open(HKEY_LOCAL_MACHINE,szSubKName);
			if(!bb)
				continue;
// 			WCHAR sv1[]=L"Photon with WiFi";	//"DriverDesc"="Photon with WiFi"
// 			bb=m_RegTab.ReadString(L"DriverDesc",szValue,100);
			WCHAR sv1[]=L"libusbK";	//"ProviderName"="libusbK"
			bb=m_RegTab.ReadString(L"ProviderName",szValue,100);
			m_RegTab.Close();
			if(bb && memcmp(sv1,szValue,sizeof(sv1)-2)==0)
			{
//				MessageBoxW(NULL,szValue,NULL,MB_OK);
				return TRUE;
			}
		}
//		return FALSE;
	}
	else
	{
		m_RegTab.Close();
		bb=FALSE;
	}
	return FALSE;

#if 0
	CString strFile;
	TCHAR szDir[MAX_PATH];
	GetSystemDirectory(szDir,sizeof(szDir)-1); 
	strFile.Format(_T("%s\\drivers\\libusb0.sys"),szDir);
	PrintMsgInfo(strFile);
	// 	if(!IsDirectoryExists((TCHAR *)(LPCTSTR)strFile))	//文件!存在
	if(_access((const char *)strFile,0)!=0)	//文件!存在
 		return FALSE;
	strFile.Format(_T("%s\\drivers\\libusbk.sys"),szDir);
	if(_access((const char *)strFile,0)!=0)	//文件!存在
		return FALSE;
	PrintMsgInfo(szDir);
#endif 
//	system("dir c:\\windows\\system32\\drivers\\libusbk.*>x.txt");
	return TRUE;
}
#endif
BOOL CDfuToolDlg::DoCheckDriverInstall()
{
	if(IsExistLibusbk())
		return TRUE;
	if(MessageBox(_T("Driver not found,now install?(Y/N)"),NULL,MB_ICONQUESTION|MB_YESNO)==IDYES)
	{
		OnBnClickedBtnInstalllibusbk();
	}
	return FALSE;
}
BOOL CDfuToolDlg::DoCheckDfuMode(BOOL bPrompt)
{
//	IsExistLibusbk();
//	return TRUE;
//	CSize m_sizeBrowseDataWin(400,600);
	int m=GetDfuMode(m_hWnd,m_szDfuModeInfo,sizeof(m_szDfuModeInfo)-1);
	if(m<0)
	{
		if(bPrompt)
			PrintMsgWarning(_T("The driver not installed! Please install."));
		return FALSE;
	}
	else if(m!=MDMSG_DFU)
	{
		if(bPrompt)
			PrintMsgWarning(_T("The photon is not in DFU mode!"));
		return FALSE;
	}
	return TRUE;
}
void CDfuToolDlg::OnBnClickedBtnAuto()
{
	if(m_bAutoUpdating || m_bDownloading||m_bUpdating)
	{
		return;
	}
	if(!DoCheckDriverInstall())
	{
		return;
	}
	if(!DoCheckDfuMode())
		return;
	m_bExit=FALSE;
	UpdateParam(TRUE);
	ClearItemInfo();
	SetLamp(&m_btnLampUpdate[0],IDB_LAMPYELLOW16);
	AfxBeginThread(AutoUpdateThread,this);
}
UINT CDfuToolDlg::AutoUpdateThread(LPVOID pParam)
{
	CDfuToolDlg *pParent=(CDfuToolDlg *)pParam;
	pParent->m_bAutoUpdating=TRUE;
	pParent->EnableControls();
	pParent->AutoUpdateFile();
	pParent->m_bAutoUpdating=FALSE;
	pParent->EnableControls();
	return 0;
}
BOOL CDfuToolDlg::AutoUpdateFile()
{
// 	CButton* pBtn=(CButton* )GetDlgItem(IDC_BTN_UPDATE1+m_nUpdateItem);
// 	pBtn->EnableWindow(FALSE);
	TCHAR szInfo[MAX_PATH];
	int i;
	m_nAutoUpdateItemN=0;
	for(i=0;i<MAX_UPDATE_NUM;i++)
	{
		SetLamp(&m_btnLampUpdate[i+1],IDB_LAMPYELLOW16);
#ifdef USE_ONEKEY_DOWNLOAD
		_stprintf(szInfo,_T("%d/%d: %s"),i+1,MAX_UPDATE_NUM,g_strUpdateFile[i]);
		GetDlgItem(IDC_STATIC_CURFILE)->SetWindowText(szInfo);
#endif
		m_ProgressCtrl.SetPos(0);
		m_nDownloadItem=i;
		DownloadFile();//TRUE);

//		m_nUpdateItem=i;
		if(!DoUpdate(i,FALSE))
			break;

		Sleep(1000);
	}
	Sleep(200);
// 	pBtn->EnableWindow(TRUE);
//	return TRUE;
//	SetLamp(&m_btnLampUpdate[0],IDB_LAMPGREEN16);
	return 0;
}

void CDfuToolDlg::UpdateParam(BOOL bSaveToTheApp)
{
	int i;
	CString strTemp;
	if(bSaveToTheApp)
	{
		for(i=0;i<MAX_UPDATE_NUM;i++)
		{
			m_EditUpdateFile[i].GetWindowText(strTemp);
			theApp.m_strUpdateFile[i]=strTemp.Trim();
		}
//		PrintMsgInfo(theApp.m_strUpdateFile[2]);
	}
	else
	{
		for(i=0;i<MAX_UPDATE_NUM;i++)
		{
			m_EditUpdateFile[i].SetWindowText(theApp.m_strUpdateFile[i]);
		}
//		PrintMsgInfo(theApp.m_strUpdateFile[2]);
	}
}

void CDfuToolDlg::OnOK()
{
	OnCancel();
	//	CDialog::OnOK();
}

void CDfuToolDlg::OnCancel()
{
	if(MessageBox(_T("Are you sure to exit?(Y/N)"),_T("Exit"),MB_ICONQUESTION|MB_YESNO)!=IDYES)
		return;

	UpdateParam(TRUE);
#ifndef USE_DEMO
	//	if(usb_handle)
	//		Connect_Complete();
	//	usb_handle=NULL;
#endif
	CDialog::OnCancel();
}
UINT CDfuToolDlg::DownloadFileThread(LPVOID pParam)
{
	CDfuToolDlg *pParent=(CDfuToolDlg *)pParam;
	CButton* pBtnDownload=(CButton* )pParent->GetDlgItem(IDC_BTN_DOWNLOAD1+pParent->m_nDownloadItem);
	pParent->m_bDownloading=TRUE;
	pParent->EnableControls();
// 	pBtnDownload->SetWindowText(_T("Stop"));
// 	pBtnDownload->EnableWindow(TRUE);	
	pParent->DownloadFile();
	pParent->m_bDownloading=FALSE;
	pParent->EnableControls();
// 	pBtnDownload->SetWindowText(_T("DOWNLOAD"));
	return 0;
}
//	int	m_nCurWebFileTotalLen;	//当前要从网上下载的文件总长度
int CDfuToolDlg::GetWebFileTotalLen()
{
	int nTotalLen=0;
	int nLen;
	for (int i=0;i<MAX_UPDATE_NUM;i++)
	{
		nLen=GetWebFileLen(i);
		if(nLen<0)
			return -1;
		nTotalLen+=nLen;
	}
	return nTotalLen;
}
int CDfuToolDlg::GetWebFileLen(int nDownloadItem)
{
	if(nDownloadItem<0||nDownloadItem>=MAX_UPDATE_NUM)
		return -1;
	CString strFileURL;
	CInternetSession	netSession;
	CStdioFile			*fTargFile=NULL;
	CString strTemp,strFile;
	static TCHAR szInfo[MAX_PATH];
	int nDownloaded=0;
	int flen=0;
	CFile fDestFile;

	strFileURL.Format(_T("%s/%s"),g_strWebAddr,g_strUpdateFile[nDownloadItem]);

	BOOL bOpen=FALSE;
	try
	{
		nDownloaded = 1;
		sprintf(szInfo,"Verifying Web site: %s\n",strFileURL); //"正在校验下载地址..."
#ifdef USE_ONEKEY_DOWNLOAD
		PostMessage(MY_PRINT_MESSAGE,MPTMSG_WORKINFO,(LPARAM)szInfo);
#else
		PostMessage(MY_PRINT_MESSAGE,1,(LPARAM)szInfo);
#endif
		fTargFile = netSession.OpenURL(strFileURL,1,INTERNET_FLAG_TRANSFER_BINARY | INTERNET_FLAG_RELOAD);
		flen = fTargFile->SeekToEnd();
		fTargFile->SeekToBegin();
	}
	catch(CInternetException *IE)
	{
		strcpy(szInfo,"Download error!\n");
#ifdef USE_ONEKEY_DOWNLOAD
		PostMessage(MY_PRINT_MESSAGE,MPTMSG_WORKINFO,(LPARAM)szInfo);
#else
		PostMessage(MY_PRINT_MESSAGE,1,(LPARAM)szInfo);
#endif
		nDownloaded = 0;
		delete fTargFile; fTargFile=NULL;
		IE->Delete();					// 删除异常对象，以防止泄漏
		flen=-2;
	}
exitdf0:
	if(fTargFile)
	{
		fTargFile->Close();					// 关闭远程文件
		delete fTargFile;					// 删除CStdioFile对象，以防止泄漏
		fTargFile=NULL;
	}
	return flen;
}
BOOL CDfuToolDlg::DownloadFile(BOOL bCheckIsExist)
{
	if(m_nDownloadItem<0||m_nDownloadItem>=MAX_UPDATE_NUM)
		return FALSE;
	CEdit *pEditFile=&m_EditUpdateFile[m_nDownloadItem];

	char				filebuf[512];
	CInternetSession	netSession;
	CStdioFile			*fTargFile=NULL;
	int					outfs;
	CString				strFileURL,KBsec; //KBin,Perc,
	CString strTemp,strOutFile,strFile;
	static TCHAR szInfo[MAX_PATH];
	int nDownloaded=0;

	UpdateParam(TRUE);

	BOOL bDo=FALSE;
	if(theApp.m_strUpdateFile[m_nDownloadItem].GetLength()==0)	//空则要下载
	{
		strFile.Format("%s\\%s",theApp.m_szRunDir,g_strUpdateFile[m_nDownloadItem]);
//		pEditFile->SetWindowText(strFile);
		SetFileNameNoCurrentDir(pEditFile,strFile);
		bDo=TRUE;
	}
	else
	{
		strFile=theApp.m_strUpdateFile[m_nDownloadItem];
	}
	CString strPath=GetFolderFromFullpath(strFile);//,szFileName,szExtName);
//	strFile.Format(_T("%s%s"),szFileName,szExtName);
	if(strPath.GetLength()==0)	//空的就用当前目录
	{
		strPath.Format("%s\\%s",theApp.m_szRunDir,strFile);
		strFile=strPath;
		strPath=theApp.m_szRunDir;
	}
//	SetCurrentDirectory(strPath);	//更换到当前目录
	if(bCheckIsExist && !bDo)	//当充许检测存在,且不空时,检查是否存在本地，如有则不下载
	{
		if(_access((const char *)strFile,0)==0)	//文件存在
		{
			return FALSE;
		}
	}
	strFileURL.Format(_T("%s/%s"),g_strWebAddr,g_strUpdateFile[m_nDownloadItem]);
	strOutFile.Empty();

	PRINTPROMPT("\n");

	CFile fDestFile;
	BOOL bOpen=FALSE;
	try
	{
		nDownloaded = 1;
//		GetDlgItemText(IDC_EDIT1,szFile);
		sprintf(szInfo,"Verifying Web site: %s\n",strFileURL); //"正在校验下载地址..."
#ifdef USE_ONEKEY_DOWNLOAD
		PostMessage(MY_PRINT_MESSAGE,MPTMSG_WORKINFO,(LPARAM)szInfo);
#else
		PostMessage(MY_PRINT_MESSAGE,1,(LPARAM)szInfo);
#endif
		fTargFile = netSession.OpenURL(strFileURL,1,INTERNET_FLAG_TRANSFER_BINARY | INTERNET_FLAG_RELOAD);

		COleDateTime dlStart = COleDateTime::GetCurrentTime();
		int filesize = fTargFile->SeekToEnd();
		fTargFile->SeekToBegin();
		outfs = filesize / 1024;		// 计算文件大小（千字节）
//		FileSize.Format("%d",outfs);	// 以KB为单位格式文件大小
		
		//strOutFile=fTargFile->GetFileName();
//		strOutFile.Format("%s\\%s",theApp.m_szRunDir,fTargFile->GetFileName());
		strOutFile=strFile;
//		pEditFile->SetWindowText(strOutFile);

		// 在当前目录创建新的目标文件
		bOpen=fDestFile.Open(strOutFile,CFile::modeCreate | CFile::modeWrite | CFile::typeBinary);
		if(!bOpen)
		{
			strcpy(szInfo,"Create file failed!\n");
#ifdef USE_ONEKEY_DOWNLOAD
			PostMessage(MY_PRINT_MESSAGE,MPTMSG_WORKINFO,(LPARAM)szInfo);
#else
			PostMessage(MY_PRINT_MESSAGE,1,(LPARAM)szInfo);
#endif
			goto exitdf;
		}
		int byteswrite;		// 写入文件的字节数
		int pos = 0;		// 当前进度条的位置
		int nperc,kbrecv;	// 进度条的百分比,获取到的数据大小（Kbs为单位）
		double secs,kbsec;	// 记录秒数, 速度（KB/秒）

//		SetDlgItemText(IDC_STATIC_PROMPT,"正在下载...");
//		strcpy(szInfo,"Downloading...\n");
		sprintf(szInfo,"Downloading from Web site: %s\n",strFileURL); //"正在校验下载地址..."
#ifdef USE_ONEKEY_DOWNLOAD
		PostMessage(MY_PRINT_MESSAGE,MPTMSG_WORKINFO,(LPARAM)szInfo);
#else
		PostMessage(MY_PRINT_MESSAGE,1,(LPARAM)szInfo);
#endif
		m_ProgressCtrl.SetRange32(0,filesize);

		while (byteswrite = fTargFile->Read(filebuf, 512))	// 读取文件
		{
			if(m_bExit)						// 如果点击取消下载
			{
//				SetDlgItemText(IDC_STATIC_PROMPT,"下载时已被用户取消！"); // Set satus bar text
				strcpy(szInfo,"User break!\n");
				PostMessage(MY_PRINT_MESSAGE,1,(LPARAM)szInfo);
//				AfxEndThread(0);					// 结束下载线程
				nDownloaded=0;
				break;
			}
			// 根据开始时间与当前时间比较，获取秒数
			COleDateTimeSpan dlElapsed = COleDateTime::GetCurrentTime() - dlStart;
			secs = dlElapsed.GetTotalSeconds();
			pos = pos + byteswrite;					// 设置新的进度条位置
			fDestFile.Write(filebuf, byteswrite);	// 将实际数据写入文件

			kbrecv = pos / 1024;					// 获取收到的数据
			kbsec = kbrecv / secs;					// 获取每秒下载多少（KB）

//			Perc.Format("%d",nperc);				// 格式化进度百分比
//			KBin.Format("%d",kbrecv);				// 格式化已下载数据大小（KB）
			KBsec.Format("%d",(int)kbsec);			// 格式化下载速度（KB/秒）
			SetProgressInfo(m_nDownloadItem,pos);
		}
	}
	catch(CInternetException *IE)
	{
// 		TCHAR error[255];
// 		IE->GetErrorMessage(error,255); // 获取错误消息
//		sprintf(szInfo,"%s\n",strerror);
		strcpy(szInfo,"Download error!\n");
#ifdef USE_ONEKEY_DOWNLOAD
		PostMessage(MY_PRINT_MESSAGE,MPTMSG_WORKINFO,(LPARAM)szInfo);
#else
		PostMessage(MY_PRINT_MESSAGE,1,(LPARAM)szInfo);
#endif
//		SetDlgItemText(IDB_BTN_STOP,"Exit");
		nDownloaded = 0;
		delete fTargFile; fTargFile=NULL;
		IE->Delete();					// 删除异常对象，以防止泄漏
	}
exitdf:
	if(bOpen)
	{
		fDestFile.Close();	bOpen=FALSE;	// 关闭目标文件
	}
	if(fTargFile)
	{
		fTargFile->Close();					// 关闭远程文件
		delete fTargFile;					// 删除CStdioFile对象，以防止泄漏
		fTargFile=NULL;
	}
	if(nDownloaded == 1)
	{
//		SetDlgItemText(IDC_STATIC_PROMPT,"下载完成！");
		strcpy(szInfo,"Download file OK.\n");
#ifdef USE_ONEKEY_DOWNLOAD
		PostMessage(MY_PRINT_MESSAGE,MPTMSG_WORKINFO,(LPARAM)szInfo);
#else
		PostMessage(MY_PRINT_MESSAGE,1,(LPARAM)szInfo);
#endif
//		bStart->EnableWindow(TRUE);
	}
	else if(strOutFile.GetLength()>0) //不成功时删除目标文件
	{
		CFile::Remove(strOutFile); //删除不成功的文件
		pEditFile->SetWindowText(NULL);
	}
//	PRINTPROMPT("\n");
	return nDownloaded;
}
void CDfuToolDlg::SetProgressInfo(int nItem,int pos)
{
	CString strTemp;
	int nMin,nMax;
	if(nItem<0||nItem>=MAX_UPDATE_NUM*2)
		return;
	m_ProgressCtrl.GetRange(nMin,nMax);
	if(nMax==0)
		return;
	m_ProgressCtrl.SetPos(pos);
	int	nperc = pos * 100 / nMax;			// 进度百分比
	int	kbrecv = (pos+1023)/1024;					// 获取收到的数据
	strTemp.Format(_T("%d/%d KB %d%%"),kbrecv,(nMax+1023)/1024,nperc);
	SetDlgItemText(IDC_STATIC_LENINFO1+nItem,strTemp);
}

LRESULT CDfuToolDlg::OnMyPrintMessage(WPARAM wParam,LPARAM lParam)
{
//	CEdit *pEdit=(CEdit *)GetDlgItem(IDC_EDIT_PROMPT);
	TCHAR *pp=(TCHAR *)lParam;
	CString strTemp,ss;
#ifdef USE_ONEKEY_DOWNLOAD
	CWnd *pWnd;
	if(wParam==MPTMSG_WORKINFO)
	{
		pWnd=GetDlgItem(IDC_STATIC_WORKINFO);
		if(pWnd && pp)
			pWnd->SetWindowText(pp);
		return TRUE;
	}
	else
	{
		pWnd=GetDlgItem(IDC_STATIC_WORKINFO);
		if(pWnd && pp)
			pWnd->SetWindowText(pp);
		return TRUE;
	}
#endif
	m_EditPrompt.GetWindowText(strTemp);
	ss=pp;
	ss.Replace(_T("\n"),_T("\xd\xa"));
	strTemp+=ss;
	m_EditPrompt.SetWindowText(strTemp);
//	m_EditPrompt.SetSel(-1,-1);
	m_EditPrompt.LineScroll(m_EditPrompt.GetLineCount(),0);  
//	PrintMsgInfo("OK");
	return TRUE;
}

LRESULT CDfuToolDlg::OnMyDfuModeMessage(WPARAM wParam,LPARAM lParam)
{
	if(wParam==MDMSG_DFU)
	{
	}
	else if(wParam==MDMSG_RUNTIME)
	{
	}
	else	//MDMSG_NO
	{

	}
	m_nDfuMode=wParam;
	UINT idv=IDB_LAMPGRAY24;
	if(m_nDfuMode==MDMSG_DFU)
		idv=IDB_LAMPYELLOW24;
	if(m_nDfuMode==MDMSG_RUNTIME)
		idv=IDB_LAMPBLUE24;
	SetLamp(&m_btnLampDfu,idv);
	return TRUE;
}

LRESULT CDfuToolDlg::OnMySetProgressMessage(WPARAM wParam,LPARAM lParam)
{
	if(wParam==MPMSG_RANGE)
	{
		m_ProgressCtrl.SetRange32(0,lParam);
//		m_ProgressCtrl.SetPos(0);
		SetProgressInfo(m_nUpdateItem+MAX_UPDATE_NUM,0);
		SetLamp(&m_btnLampUpdate[m_nUpdateItem+1],IDB_LAMPYELLOW16);
	}
	if(wParam==MPMSG_CURPOS)
	{
		//m_ProgressCtrl.SetPos(lParam);
		SetProgressInfo(m_nUpdateItem+MAX_UPDATE_NUM,lParam);
		int nMin,nMax;
		m_ProgressCtrl.GetRange(nMin,nMax);
		if(nMax<=lParam)
		{
			SetLamp(&m_btnLampUpdate[m_nUpdateItem+1],IDB_LAMPGREEN16);
			if(m_bAutoUpdating)
				m_nAutoUpdateItemN++;
			if(m_nAutoUpdateItemN==MAX_UPDATE_NUM)
				SetLamp(&m_btnLampUpdate[0],IDB_LAMPGREEN16);
		}
	}
	return TRUE;
}
static char szCmd[100]="dfu-util -l";

void CDfuToolDlg::OnTimer(UINT_PTR nIDEvent)
{
	if(nIDEvent==1)
	{
		if(!m_bUpdating)
		{
//			static int m=0; m=(m+1)%3;
 			int m=GetDfuMode(m_hWnd,m_szDfuModeInfo,sizeof(m_szDfuModeInfo)-1);
 			PostMessage(MY_DFUMODE_MESSAGE,m,0);
// 			static char ss[100];
// 			strcpy(ss,szCmd);
// 			DoDfuUtil(m_hWnd,ss);
		}
	}
	CDialog::OnTimer(nIDEvent);
}
void CDfuToolDlg::EnableControls()
{
	BOOL bb=FALSE;
	UINT idv[]={
		IDC_EDIT1,IDC_EDIT2,IDC_EDIT3,IDC_BTN_BROWSE1,IDC_BTN_BROWSE2,IDC_BTN_BROWSE3,
		IDC_BTN_DOWNLOAD1,IDC_BTN_DOWNLOAD2,IDC_BTN_DOWNLOAD3,
		IDC_BTN_UPDATE1,IDC_BTN_UPDATE2,IDC_BTN_UPDATE3,
		IDC_BTN_INSTALLLIBUSBK,IDC_BTN_AUTO,
#ifdef USE_ONEKEY_DOWNLOAD
		IDC_BTN_FIXCUBE,
#endif
	};
	bb=!(m_bAutoUpdating||m_bUpdating||m_bDownloading);
	::EnableControl(this,idv,sizeof(idv)/sizeof(UINT),bb);
}
//将文件名中的当前程序目录名去掉，置到pWnd标题,以便简洁
void CDfuToolDlg::SetFileNameNoCurrentDir(CWnd *pWnd,LPCTSTR strFileName)
{
	CString strFile,strPath;
	TCHAR szFileName[MAX_PATH];
	TCHAR szExtName[MAX_PATH];
	strPath=GetFolderFromFullpath(strFileName,szFileName,szExtName);
	if(strPath.CompareNoCase(theApp.m_szRunDir)==0)
	{
		strFile.Format("%s%s",szFileName,szExtName); //=g_strUpdateFile[i];	
		pWnd->SetWindowText(strFile);
	}
	else
		pWnd->SetWindowText(strFileName);
}

#ifdef USE_ONEKEY_DOWNLOAD
//一键下载
void CDfuToolDlg::OnBnClickedBtnFixcube()
{
	if(m_bAutoUpdating || m_bDownloading||m_bUpdating)
	{
		return;
	}
	m_bExit=FALSE;
//	UpdateParam(TRUE);
	ClearItemInfo();
	SetLamp(&m_btnLampUpdate[0],IDB_LAMPYELLOW16);
	AfxBeginThread(AutoFixcubeThread,this);

}
UINT CDfuToolDlg::AutoFixcubeThread(LPVOID pParam)
{
	CDfuToolDlg *pParent=(CDfuToolDlg *)pParam;
	pParent->m_bAutoUpdating=TRUE;
	pParent->EnableControls();
	pParent->DoFixcube();
	pParent->m_bAutoUpdating=FALSE;
	pParent->EnableControls();
	pParent->m_bDownloading=FALSE;
	pParent->m_bDownloading=FALSE;
	return 0;
}
BOOL CDfuToolDlg::DoFixcube()
{
	BOOL bOK=FALSE;
#if 0
	if(!IsExistLibusbk())
	{
// 		if(MessageBox(_T("Driver not found,now install?(Y/N)"),NULL,MB_ICONQUESTION|MB_YESNO)==IDYES)
// 		{	}
		if(DoInstallLibusbk())
		{
			while(!m_bExit)
			{
				if(IsExistLibusbk())	//已安装
				{
					bOK=TRUE; break;
				}
				WaitForSingleObject(m_hWaitEvent,1000);	//30
			}
		}			
	}
	else 
		bOK=TRUE;
	if(m_pHelpFrame&&m_pHelpFrame->IsWindowVisible())
	{
		m_pHelpFrame->ShowWindow(SW_HIDE);
	}
	if(!bOK)
	{
		PrintMsgWarning(_T("Driver not found!"));
		return FALSE;
	}
	bOK=DoCheckDfuMode(FALSE); 
	if(!bOK) //DoCheckDfuMode())
	{
//		return;
		//DoDfuHelp();
		PostMessage(WM_COMMAND,IDM_DFUHELP);
		int k=0;
		while(!m_bExit&&!bOK)
		{
			WaitForSingleObject(m_hWaitEvent,1000);	//30
			bOK=DoCheckDfuMode(FALSE); 
// 			if(k>100)
// 				break;
			k++;
		}
		if(m_pDfuHelpFrame&&m_pDfuHelpFrame->IsWindowVisible())
		{
			m_pDfuHelpFrame->ShowWindow(SW_HIDE);
		}
	}
/*
	int len=GetWebFileTotalLen();
	if(len>0)
	{
		m_nCurWebFileTotalLen=len;

	}
*/
#endif
	int dnum=0,cnum=0;
	while(1)
	{
		dnum=DoListAll();
		cnum=DoScanComm();
		bOK=DoCheckDfuMode(FALSE); 
		if(bOK)	//是DFU mode 则退出循环,进行更新固件
		{
			PRINTWORKINFO(_T("The photon is in DFU mode."));
			break;
		}
		if(m_bExit)	//有用户中断,则退出安装
			break;
		if(dnum==0)	//列表中没有
		{
			//加提示请插入photon
			PRINTWORKINFO(_T("Please insert the photon!"));
			//等待一会
			WaitForSingleObject(m_hWaitEvent,1000);	//30
			continue;
		}
		if(!bOK && dnum>0 && cnum>0 && CheckDescNameIsInDevList(DESCNAME_USBSER))
		{
			PRINTWORKINFO(_T("Switching to DFU mode,waiting..."));
			if(SwitchToDfuMode())
			{
				WaitForSingleObject(m_hWaitEvent,1000);	
				continue;
			}
		}
		//检查是否有Dfu模式的驱动，无则安装
		bOK=CheckInstallDfuModeDriver();	
		if(!bOK)
		{
			//检查是否有Usb2com的驱动，无则安装
			bOK=CheckInstallUsb2comDriver();
		}
		WaitForSingleObject(m_hWaitEvent,1000);	//30
	}
	if(!m_bExit)
		AutoUpdateFile();
	return TRUE;
}


void CDfuToolDlg::OnNMClickSyslink(NMHDR *pNMHDR, LRESULT *pResult)
{
	PNMLINK pNMLink = (PNMLINK) pNMHDR;   
	ShellExecuteW(NULL, L"open", pNMLink->item.szUrl, NULL, NULL, SW_SHOWNORMAL);  //在浏览器中打开        
	*pResult = 0;  
}
#endif
