#include "StdAfx.h"

#include <io.h>
#include <ShlObj.h>
//#include <odbcinst.h>
#include "NewFunc.h"

#include <devguid.h>
// The next 3 includes are needed for serial port enumeration
#include <objbase.h>
#include <initguid.h>

#include <Setupapi.h>

int EnumDevice(IN LPGUID lpGuid,OUT CStringArray & strArrayDevName,BOOL bPresent=TRUE,OUT  CStringArray *pstrArrayHID=NULL,OUT  CStringArray *pstrArrayHandle=NULL);//bOnlyListHiden=FALSE);

static char *fmt0[3]={"%.3f","%.2f","%d"};
static char *fmt1[3]={"%.3f h","%.2f m","%d s"};
static char *fmt2[3]={"%.3f h","%.2f min","%d sec"};
static float fxs[3]={1/3600.0f,1/60.0f,1.0f};
/*
enum {TIMEMODE_HOUR=0,TIMEMODE_MIN=1,TIMEMODE_SEC=2,
	TIMEMODE_MARK_UNIT1=0x8000,	//ֻ�������д(һ���ַ�)��ʾ��λ
	TIMEMODE_MARK_UNIT2=0x4000	//ֻ��������д(һ������ַ�)��ʾ��λ
};
*/
LPTSTR ToTimeStr(time_t tv,int nTimeMode,LPTSTR szOutStr)
{
	if(szOutStr==NULL)
		return NULL;
	char * *pp;
	pp=fmt0;
	if(nTimeMode&TIMEMODE_MARK_UNIT1)	
		pp=fmt1;
	if(nTimeMode&TIMEMODE_MARK_UNIT1)	
		pp=fmt2;
	int idx=2;
	if((nTimeMode&0xFF)==TIMEMODE_HOUR)
		idx=0;
	else if((nTimeMode&0xFF)==TIMEMODE_MIN)
		idx=1;
	if(idx==2)
		sprintf(szOutStr,pp[idx],tv);
	else
		sprintf(szOutStr,pp[idx],(float)tv*fxs[idx]);
	return szOutStr;
}
LPTSTR ToTimeStr(time_t tv,int nTimeMode,CString & strOut)
{
	char * *pp;
	pp=fmt0;
	if(nTimeMode&TIMEMODE_MARK_UNIT1)	
		pp=fmt1;
	if(nTimeMode&TIMEMODE_MARK_UNIT2)	
		pp=fmt2;
	int idx=2;
	if((nTimeMode&0xFF)==TIMEMODE_HOUR)
		idx=0;
	else if((nTimeMode&0xFF)==TIMEMODE_MIN)
		idx=1;
	if(idx==2)
		strOut.Format(pp[idx],tv);
	else
		strOut.Format(pp[idx],(float)tv*fxs[idx]);
	return (LPTSTR)&strOut;
}

//��nValue������str,��nValue=0:���,����ת��intֵ
void SetStringInt(CString & str,int nValue)
{
	if(nValue!=0)
		str.Format("%d",nValue);
	else
		str.Empty();
}
//ת����strΪUINTֵ,Ϊ����ʱ,�þ���ֵ
UINT GetStringUInt(CString & str)
{
	if(str.IsEmpty())
		return 0;
	int n=atoi(str);
	return abs(n);
}

char *GetDateStr(time_t tTime,char sdate[])
{
	return GetTimeDigitStr(tTime,sdate,0);
}
//ȡʱ������ִ�,����szTime[],m=0:����,��:"070203"(2007-02-03),m=1:����ʱ��,"070203123000"
//m=2:û���������ʱ�� "0702031230"
char *GetTimeDigitStr(time_t tTime,char szTime[],int m)
{
	CTime ct(tTime);
//	ct=tTime;
	if(m==0)	//������
		sprintf(szTime,"%02d%02d%02d",ct.GetYear()%100,ct.GetMonth(),ct.GetDay());
	else if(m==1)	//ȫ��
		sprintf(szTime,"%02d%02d%02d%02d%02d%02d",ct.GetYear()%100,ct.GetMonth(),ct.GetDay(),ct.GetHour(),ct.GetMinute(),ct.GetSecond());
	else	//û����
		sprintf(szTime,"%02d%02d%02d%02d%02d",ct.GetYear()%100,ct.GetMonth(),ct.GetDay(),ct.GetHour(),ct.GetMinute());
	return szTime;
}

CString GetCurrentTimeStr()
{
	CTime Time=CTime::GetCurrentTime();
	return Time.Format("%Y-%m-%d %H:%M:%S");
}

CString GetCurrentTimeStr(time_t tTime)
{
	CTime Time(tTime);
	return Time.Format("%Y-%m-%d %H:%M:%S");
}


int PrintMsgWarning(LPCTSTR fmt, ...)
{	char buffer[2048];
	va_list argptr;
	int cnt;
	va_start(argptr, fmt);
	cnt = vsprintf(buffer, fmt, argptr);
	va_end(argptr);
	return AfxMessageBox(buffer,MB_OK|MB_ICONWARNING);
}

int PrintMsgError(LPCTSTR fmt, ...)
{	char buffer[2048];
	va_list argptr;
	int cnt;
	va_start(argptr, fmt);
	cnt = vsprintf(buffer, fmt, argptr);
	va_end(argptr);
	return AfxMessageBox(buffer,MB_OK|MB_ICONERROR);
}
int PrintMsgInfo(LPCTSTR fmt, ...)
{	char buffer[2048];
	va_list argptr;
	int cnt;
	va_start(argptr, fmt);
	cnt = vsprintf(buffer, fmt, argptr);
	va_end(argptr);
	return AfxMessageBox(buffer,MB_OK|MB_ICONINFORMATION);
}
/*void PrintPrompt(LPCTSTR fmt, ...)
{
	CMainFrame *pFrame;
	char buffer[2048];
	va_list argptr;
	int cnt;
	va_start(argptr, fmt);
	cnt = vsprintf(buffer, fmt, argptr);
	va_end(argptr);
	pFrame=(CMainFrame *)theApp.m_pMainWnd; // AfxGetMainWnd();
	if(pFrame==NULL)
		return ;
	pFrame->SetMessageText(buffer);
}
*/
void PrintMsgToFile(LPCTSTR fname,LPCTSTR fmt, ...)
{
	char buffer[2048];
	va_list argptr;
	int cnt;
	va_start(argptr, fmt);
	cnt = vsprintf(buffer, fmt, argptr);
	va_end(argptr);
	AddStrToFile(fname,buffer,TRUE);
}

void AddStrToFile(LPCTSTR lpfname,LPCTSTR szStr,BOOL bAddCurTime)
{
	CFile file;
	BOOL bb=FALSE;
	bb=file.Open(lpfname,CFile::modeReadWrite);
	if(!bb)
	{
		bb=file.Open(lpfname,CFile::modeWrite|CFile::modeCreate);
	}
	if(!bb)
		return;

	CString str,ss;
	if(bAddCurTime)
	{
		CTime ct=CTime::GetCurrentTime();
		ss=ct.Format("[%Y-%m-%d %H:%M:%S]");
		str.Format("%s %s\xd\xa",ss,szStr);
	}
	else
		str.Format("%s\xd\xa",szStr);
	file.SeekToEnd();
	file.Write(str,str.GetLength());
	file.Close();
}

//ȡλ��,nMaxBitNum:���λ��
int GetBitNum(BYTE cBitByte[],int nMaxBitNum)
{
	int i,m=0;
	for (i=0;i<nMaxBitNum;i++)
	{
		if(GET_CHANBIT(cBitByte,i))
			m++;
	}
	return m;
}
//ȡλ������Ч�ֽ���(�����һ����1��λռ���ֽ���),nMaxBitNum:���λ��
int GetValidBitByteNum(BYTE cBitByte[],int nMaxBitNum)
{
	int i,m=-1;
	for (i=0;i<nMaxBitNum;i++)
	{
		if(GET_CHANBIT(cBitByte,i))
			m=i;
	}
	if(m==-1)
		return 0;
	return m/8+1;
}

void PrintHexMessage(LPCTSTR lpBuf,int nlen)
{
	CString strTemp="";
//	BYTE *pBuf=(BYTE *)&stQBI;
	int len=16,n=(nlen+15)/16;
	for(int i=0;i<n;i++)
	{
		if(i==n-1 && (nlen%16)!=0)
			len=nlen%16;
		strTemp+=ToHexStr((BYTE *)lpBuf,len);
		strTemp+="\xd\xa";
		lpBuf+=len;
	}
	AfxMessageBox(strTemp);
}
CString ToHexStr(BYTE buf[],int len)
{
	CString str;
	char ss[20];
	int i;
	str="";
	for(i=0;i<len;i++)
	{
		sprintf(ss,"%02X ",buf[i]);
		str+=ss;
	}
	return str;
}

void EnableControl(CWnd *pWnd,UINT id[],int num,BOOL bEnable)
{
	int i;
	CWnd *pCWnd;	
	if(pWnd==NULL)
		return;
	for(i=0;i<num;i++)
	{
		pCWnd=pWnd->GetDlgItem(id[i]);
		if(pCWnd)
		{
			if(pCWnd->IsWindowEnabled()!=bEnable)
				pCWnd->EnableWindow(bEnable);
		}
	}
}
void ShowControl(CWnd *pWnd,UINT id[],int num,BOOL bShow)
{
	int i;
	CWnd *pCWnd;	
	if(pWnd==NULL)
		return;
	int nShow=SW_SHOW;
	if(!bShow)
		nShow=SW_HIDE;
	for(i=0;i<num;i++)
	{
		pCWnd=pWnd->GetDlgItem(id[i]);
		if(pCWnd) pCWnd->ShowWindow(nShow);
	}
}


WORD GetWordChkSum(BYTE buf[],int n)
{
	int i;
	WORD sw=0;
	for(i=0;i<n;i++)
	{
		sw+=buf[i];
	}
	return sw;
}

void LocateWindow(CWnd *pWnd,CWnd *pLocationWnd,CWnd *pParent)
{
	CRect rectCaptionBar;
	if(pParent==NULL || pWnd==NULL || pLocationWnd==NULL)
		return;
	pLocationWnd->GetWindowRect (&rectCaptionBar);
	pParent->ScreenToClient (&rectCaptionBar);
	pWnd->SetWindowPos (&pParent->wndTop, rectCaptionBar.left, rectCaptionBar.top, 
		rectCaptionBar.Width(), 
		rectCaptionBar.Height(),
		SWP_NOACTIVATE);
}
void LocateWindow(UINT nID,UINT nLocationID,CWnd *pParent)
{
	CWnd *pWnd,*pLocationWnd;
	if(pParent==NULL)
		return;
	pWnd=pParent->GetDlgItem(nID);
	pLocationWnd=pParent->GetDlgItem(nLocationID);
	LocateWindow(pWnd,pLocationWnd,pParent);
}

char *GetDateTimeStr(time_t ttime,char *str)
{
	struct tm* ptmTemp = localtime(&ttime);
	if (ptmTemp == NULL ||
		!_tcsftime(str, 30, "%Y-%m-%d %H:%M:%S", ptmTemp))
		str[0] = '\0';
	return str;
}

CString  OpenDir(LPCTSTR  cDlgName)  
{  

	char   Mycom[MAX_PATH]="E:\\";  
	BROWSEINFO   Myfold;  
	Myfold.hwndOwner=NULL;  
	Myfold.pidlRoot=NULL;  
	Myfold.pszDisplayName=Mycom;  
	Myfold.lpszTitle=cDlgName;  
	Myfold.ulFlags=BIF_RETURNONLYFSDIRS;  
	Myfold.lpfn=NULL;  
	Myfold.lParam=NULL;  
	Myfold.iImage=NULL;  
	Mycom[0]='\0';  
	LPITEMIDLIST lpIIL=SHBrowseForFolder(&Myfold);
	//	if(lpIIL==NULL)

	SHGetPathFromIDList(lpIIL,Mycom);  
	return(Mycom);  
}
int Touch(LPCTSTR lpPath, BOOL bValidate)
{
	if(lpPath==NULL)
	{
		return 1;
	}

	TCHAR szPath[MAX_PATH];
	_tcscpy(szPath, lpPath);
	int nLen = _tcslen(szPath);

	//path must be "x:\..."
	if( ( nLen<3 ) || 
		( ( szPath[0]<_T('A') || _T('Z')<szPath[0] ) && 
		  ( szPath[0]<_T('a') || _T('z')<szPath[0] ) ||
		( szPath[1]!=_T(':') )|| 
		( szPath[2]!=_T('\\') )
		)
	  )
	{
		return 1;
	}

	int i;
	if(nLen==3)
	{
		if(!bValidate)
		{
			if(_access(szPath, 0)!=0)
			{
				return 2;
			}
		}
		return 0;
	}

	i = 3;
	BOOL bLastOne=TRUE;
	LPTSTR lpCurrentName;
	while(szPath[i]!=0)
	{
		lpCurrentName = &szPath[i];
		while( (szPath[i]!=0) && (szPath[i]!=_T('\\')) )
		{
			i++;
		}

		bLastOne =(szPath[i]==0);
		szPath[i] = 0;

		if( !IsFileNameValid(lpCurrentName) )
		{
			return 1;
		}

		if(!bValidate)
		{
			CreateDirectory(szPath, NULL);
			if(_taccess(szPath, 0)!=0)
			{
				return 2;
			}
		}

		if(bLastOne)
		{
			break; //it's done
		}
		else
		{
			szPath[i] = _T('\\');
		}

		i++;
	}

	return (bLastOne?0:1);
}

BOOL IsFileNameValid(LPCTSTR lpFileName)
{
	if(lpFileName==NULL)
	{
		return FALSE;
	}

	int nLen = _tcslen(lpFileName);
	if(nLen<=0)
	{
		return FALSE;
	}

	//check first char
	switch(lpFileName[0])
	{
	case _T('.'):
	case _T(' '):
	case _T('\t'):
		return FALSE;
	}

	//check last char
	switch(lpFileName[nLen-1])
	{
	case _T('.'):
	case _T(' '):
	case _T('\t'):
		return FALSE;
	}

	//check all
	int i=0;
	while(lpFileName[i]!=0)
	{
		switch(lpFileName[i])
		{
		case _T('\\'):
		case _T('/'):
		case _T(':'):
		case _T('*'):
		case _T('?'):
		case _T('\"'):
		case _T('<'):
		case _T('>'):
		case _T('|'):
			return FALSE;
		}
		i++;
	}
	return TRUE;
}
/*
void BackupOldChlDateFile(const char *pszChlDateFile)
{
	char szFullPathName[200], szDirTemp[200], szFileName[50], *pLastPos;

	if(_access(pszChlDateFile, 00) != 0)	return;
	strcpy(szFullPathName, pszChlDateFile);
	pLastPos = strrchr(szFullPathName, '\\');
	ASSERT(pLastPos != NULL);
	strcpy(szFileName, pLastPos+1);
	*pLastPos = NULL;	
	pLastPos = strrchr(szFullPathName, '\\');
	ASSERT(pLastPos != NULL);
	*(pLastPos+1) = NULL;
	strcat(szFullPathName, m_szBakDataSubDir);
	strcat(szFullPathName, szFileName);
	if(_access(szFullPathName, 00) == 0)	_unlink(szFullPathName);
	else{
		strcpy(szDirTemp, szFullPathName);
		pLastPos = strrchr(szDirTemp, '\\');
		*pLastPos = '\0';
		strcat(szDirTemp, "\\NUL");
		if(_access(szDirTemp, 00) != 0){
			pLastPos = strrchr(szDirTemp, '\\');
			*pLastPos = '\0';
			_mkdir(szDirTemp);
		}
	}
	rename(pszChlDateFile, szFullPathName);	
}*/

int round(double fv)
{
	int fi=0;
	if(fv==0.0f)
		return 0;
	fi=int(fv);
	if(double(fi)==fv)
		return fi;
	if(fv>0.0f)
	{
		return fi+1;
	}
	return fi-1;
}
//ȡ���ʵĻ�ͼ����y�᷶Χ,m=0:����,=1:��ѹ,=2:����
void GetFixYRange(int m,float & fmax,float & fmin)
{
	float fv1=fmin,fv2=fmax;
	float ff=max(fabs(fv1),fabs(fv2));
	float fa=200.0f;
 	if(m==0)
 		fa=ff*10/100;
 	else
		fa=ff*5/100;
	fa=floor(fa/100.0f+0.5)*100.0f;
	if(fa<200)
		fa=200;
	if(m==1)
		fa=100.0f;

	fv2=floor(fv2/100.0f+0.5)*100.0f+fa;
	fv1=floor(fv1/100.0f-0.5)*100.0f-fa;
	if(m==0)	//I
	{
		fv2=max(fabs(fv1),fabs(fv2));
		fv1=-fv2;
	}
	else if(m==1)	//V
	{
		if(fv2<4500) 
			fv2=4500;
		if(fv1<0)
			fv1=0;
		if(fv2-fv1<1000)
		{
			fv1=fv2-1000;
			if(fv1<0)
				fv1=0;
			fv2=fv1+1000;
		}
	}
	else	//C
	{
		if(fv1<0)
			fv1=0;
		if(fv2-fv1<1000)
			fv2=fv1+1000;
	}
	fmax=fv2; fmin=fv1;
}
//�ļ�Ŀ¼ת��LPITEMIDLIST
LPITEMIDLIST ParsePidlFromPath(LPCSTR path)
{    
	OLECHAR szOleChar[MAX_PATH];    
	LPSHELLFOLDER IpsfDeskTop;    
	LPITEMIDLIST lpifq;    
	ULONG ulEaten, ulAttribs;    
	HRESULT hres;    
	SHGetDesktopFolder(&IpsfDeskTop);    
	MultiByteToWideChar(CP_ACP,MB_PRECOMPOSED,path,-1,szOleChar,sizeof(szOleChar));    
	hres = IpsfDeskTop ->ParseDisplayName(NULL, NULL, szOleChar, &ulEaten, &lpifq, &ulAttribs);    
	hres=IpsfDeskTop->Release( );        
	if(FAILED(hres))
		return NULL;
	return lpifq;
}

//��szStr�����кŸ�������ȡ��������fResult[]��,nMaxN:���ȡ�ĸ���,����ʵ�ʸ���
//�� 1.23,45.6,12.45 ��
int GetFloatFromStr(IN LPCTSTR szStr,OUT float fResult[],IN int nMaxN)
{
	int ln;
	TCHAR ss[MAX_PATH];
	TCHAR *p=(TCHAR *)szStr;
	if(nMaxN<=0)
		return 0;

	while(*p==_T(' ')) p++;
	ln=_tcslen(p);
	if(ln==0||ln>MAX_PATH-1)
		return 0;
	int m=0;
	for (;;)
	{
		TCHAR *pp=_tcschr(p,_T(','));
		if(pp)
		{
			ln=pp-p;
			_tcsncpy(ss,p,ln); ss[ln]=0;
		}
		else
		{
			_tcscpy(ss,p); ln=_tcslen(p);
		}
		if(ln==0)
			fResult[m]=0.0f;
		else
			fResult[m]=_tstof(ss); 
		m++;
		if(m>=nMaxN)
			return m;
		if(pp==NULL)
			break;
		p=pp+1;
	}
	return m;
}

//��fResult[]�еĸ�����ת���ڴ�����szStr��(��','�Ÿ���),����ʵ�ʸ���,�������򷵻�0
//�� 1.23,45.6,12.45 ��
int SetFloatToStr(IN float fResult[],IN int nNum,OUT LPTSTR szStr)
{
	CString strTemp;
	int ln;
	TCHAR ss[MAX_PATH];
	szStr[0]=0;
	strTemp.Empty();
	for (int i=0;i<nNum;i++)
	{
		sprintf(ss,"%g,",fResult[i]);
		strTemp+=ss;
	}
	ln=strTemp.GetLength();
	if(ln>0) ln--;		//ȥ�����һ��','
	if(ln==0 || ln>MAX_PATH-1)
		return 0;
	_tcsncpy(szStr,(LPCTSTR)strTemp,ln); szStr[ln]=0;
	return nNum;
}

//ȡ�ļ�����lpszStr·��������ļ���,���ظõ�ַ,������(��������չ��)��pLen��
TCHAR *GetFileName(LPCTSTR lpszStr,int *pLen)
{
	int ln;
	TCHAR *p=(TCHAR *)lpszStr;
	TCHAR *pp;
	pp=_tcsrchr(p,_T('\\'));
	if(pp) p=pp+1;
	pp=_tcsrchr(p,_T('.'));
	if(pp==NULL)
	{
		ln=_tcslen(p);
	}
	else
	{
		ln=pp-p;
	}
	if(pLen) *pLen=ln;
	return p;
}
//���ļ�ȫ����ȡ�ļ�������(Ŀ¼��)
CString GetFolderFromFullpath(LPCTSTR lpFullpath,OUT TCHAR *pFileName,OUT TCHAR *pExtName)
{
	TCHAR drive[_MAX_DRIVE];
	TCHAR dir[_MAX_DIR];
	TCHAR fname[_MAX_FNAME];
	TCHAR ext[_MAX_EXT];
	_tsplitpath( lpFullpath, drive, dir, fname, ext);

	CString strPath;
	int ln=_tcslen(dir);
	if(ln>0)
	{
		if(dir[ln-1]==_T('\\'))
		{	ln--; dir[ln]=0; }
	}
	strPath.Format(_T("%s%s"),drive,dir);
	if(pFileName)
		_tcscpy(pFileName,fname);
	if(pExtName)
		_tcscpy(pExtName,ext);
	return strPath;
}

BOOL is_x64(void)
{
	BOOL ret = FALSE;
	BOOL (__stdcall *pIsWow64Process)(HANDLE, PBOOL) = NULL;
	// Detect if we're running a 32 or 64 bit system
	if (sizeof(uintptr_t) < 8) {
		pIsWow64Process = (BOOL (__stdcall *)(HANDLE, PBOOL))
			GetProcAddress(GetDLLHandle("kernel32"), "IsWow64Process");
		if (pIsWow64Process != NULL) {
			(*pIsWow64Process)(GetCurrentProcess(), &ret);
		}
	} else {
		ret = TRUE;
	}
	return ret;
}

// From smartmontools os_win32.cpp
const char* PrintWinVersion(OUT int & nWindowsVersion)
{
	OSVERSIONINFOEXA vi, vi2;
	const char* w = 0;
	const char* w64 = "32 bit";
	char* vptr;
	size_t vlen;
	unsigned major, minor;
	ULONGLONG major_equal, minor_equal;
	BOOL ws;
//	int nWindowsVersion;
	static char WindowsVersionStr[128] = "Windows ";

	nWindowsVersion = WINDOWS_UNDEFINED;
	safe_strcpy(WindowsVersionStr, sizeof(WindowsVersionStr), "Windows Undefined");

	memset(&vi, 0, sizeof(vi));
	vi.dwOSVersionInfoSize = sizeof(vi);
	if (!GetVersionExA((OSVERSIONINFOA *)&vi)) {
		memset(&vi, 0, sizeof(vi));
		vi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOA);
		if (!GetVersionExA((OSVERSIONINFOA *)&vi))
			return WindowsVersionStr;
	}

	if (vi.dwPlatformId == VER_PLATFORM_WIN32_NT) {

		if (vi.dwMajorVersion > 6 || (vi.dwMajorVersion == 6 && vi.dwMinorVersion >= 2)) {
			// Starting with Windows 8.1 Preview, GetVersionEx() does no longer report the actual OS version
			// See: http://msdn.microsoft.com/en-us/library/windows/desktop/dn302074.aspx

			major_equal = VerSetConditionMask(0, VER_MAJORVERSION, VER_EQUAL);
			for (major = vi.dwMajorVersion; major <= 9; major++) {
				memset(&vi2, 0, sizeof(vi2));
				vi2.dwOSVersionInfoSize = sizeof(vi2); vi2.dwMajorVersion = major;
				if (!VerifyVersionInfoA(&vi2, VER_MAJORVERSION, major_equal))
					continue;
				if (vi.dwMajorVersion < major) {
					vi.dwMajorVersion = major; vi.dwMinorVersion = 0;
				}

				minor_equal = VerSetConditionMask(0, VER_MINORVERSION, VER_EQUAL);
				for (minor = vi.dwMinorVersion; minor <= 9; minor++) {
					memset(&vi2, 0, sizeof(vi2)); vi2.dwOSVersionInfoSize = sizeof(vi2);
					vi2.dwMinorVersion = minor;
					if (!VerifyVersionInfoA(&vi2, VER_MINORVERSION, minor_equal))
						continue;
					vi.dwMinorVersion = minor;
					break;
				}

				break;
			}
		}

		if (vi.dwMajorVersion <= 0xf && vi.dwMinorVersion <= 0xf) {
			ws = (vi.wProductType <= VER_NT_WORKSTATION);
			nWindowsVersion = vi.dwMajorVersion << 4 | vi.dwMinorVersion;
			switch (nWindowsVersion) {
			case 0x50: w = "2000";
				break;
			case 0x51: w = "XP";
				break;
			case 0x52: w = (!GetSystemMetrics(89)?"2003":"2003_R2");
				break;
			case 0x60: w = (ws?"Vista":"2008");
				break;
			case 0x61: w = (ws?"7":"2008_R2");
				break;
			case 0x62: w = (ws?"8":"2012");
				break;
			case 0x63: w = (ws?"8.1":"2012_R2");
				break;
			case 0x64: w = (ws?"10":"2015");
				break;
			default:
				if (nWindowsVersion < 0x50)
					nWindowsVersion = WINDOWS_UNSUPPORTED;
				else
					w = "11 or later";
				break;
			}
		}
	}

	if (is_x64())
		w64 = "64-bit";

	vptr = &WindowsVersionStr[sizeof("Windows ") - 1];
	vlen = sizeof(WindowsVersionStr) - sizeof("Windows ") - 1;
	if (!w)
		safe_sprintf(vptr, vlen, "%s %u.%u %s", (vi.dwPlatformId==VER_PLATFORM_WIN32_NT?"NT":"??"),
		(unsigned)vi.dwMajorVersion, (unsigned)vi.dwMinorVersion, w64);
	else if (vi.wServicePackMinor)
		safe_sprintf(vptr, vlen, "%s SP%u.%u %s", w, vi.wServicePackMajor, vi.wServicePackMinor, w64);
	else if (vi.wServicePackMajor)
		safe_sprintf(vptr, vlen, "%s SP%u %s", w, vi.wServicePackMajor, w64);
	else
		safe_sprintf(vptr, vlen, "%s %s", w, w64);
	return WindowsVersionStr;
}

//////////////////////////////////////////////////////////////////////////
/// ����Դ�ļ��м���ͼƬ
/// @param [in] pImage ͼƬָ��
/// @param [in] nResID ��Դ��
/// @param [in] lpTyp ��Դ����
//////////////////////////////////////////////////////////////////////////
bool LoadImageFromResource(IN CImage* pImage,
	IN UINT nResID, 
	IN LPCTSTR lpTyp)
{
	if ( pImage == NULL) return false;

	pImage->Destroy();

	// ������Դ
	HRSRC hRsrc = ::FindResource(AfxGetResourceHandle(), MAKEINTRESOURCE(nResID), lpTyp);
	if (hRsrc == NULL) return false;

	// ������Դ
	HGLOBAL hImgData = ::LoadResource(AfxGetResourceHandle(), hRsrc);
	if (hImgData == NULL)
	{
		::FreeResource(hImgData);
		return false;
	}

	// �����ڴ��е�ָ����Դ
	LPVOID lpVoid    = ::LockResource(hImgData);

	LPSTREAM pStream = NULL;
	DWORD dwSize    = ::SizeofResource(AfxGetResourceHandle(), hRsrc);
	HGLOBAL hNew    = ::GlobalAlloc(GHND, dwSize);
	LPBYTE lpByte    = (LPBYTE)::GlobalLock(hNew);
	::memcpy(lpByte, lpVoid, dwSize);

	// ����ڴ��е�ָ����Դ
	::GlobalUnlock(hNew);

	// ��ָ���ڴ洴��������
	HRESULT ht = ::CreateStreamOnHGlobal(hNew, TRUE, &pStream);
	if ( ht != S_OK )
	{
		GlobalFree(hNew);
	}
	else
	{
		// ����ͼƬ
		pImage->Load(pStream);

		GlobalFree(hNew);
	}

	// �ͷ���Դ
	::FreeResource(hImgData);

	return true;
}

//---------------------------------------------------------------
// Routine for enumerating the available serial ports.
// Throws a CString on failure, describing the error that
// occurred. If bIgnoreBusyPorts is TRUE, ports that can't
// be opened for read/write access are not included.

BOOL EnumSerialPorts(CArray<SSerInfo,SSerInfo&> &asi, BOOL bIgnoreBusyPorts)
{
	// Clear the output array
	asi.RemoveAll();
#if 1
	CString strName;
	CStringArray strArrayDev;
	int n=EnumDevice((LPGUID)&GUID_DEVCLASS_PORTS,strArrayDev);
	if(n==0)
		return FALSE;
	for (int i=0;i<n;i++)
	{
		SSerInfo rsi;
		strName=strArrayDev.GetAt(i);
		rsi.strFriendlyName=strName;
		rsi.strPortName = rsi.strFriendlyName;
		int startdex = rsi.strFriendlyName.Find(_T(" ("));
		int enddex = rsi.strFriendlyName.Find(_T(")"));
		if (startdex > 0 && enddex == (rsi.strFriendlyName.GetLength()-1))
		{
			rsi.strPortName=rsi.strFriendlyName.Mid(startdex+2,enddex-startdex-2);
		}
		rsi.strPortName.MakeUpper();
		int idx=rsi.strPortName.Find(_T("COM"));
		if(idx>=0)
			rsi.nComPort=atoi(rsi.strPortName.Mid(idx+3)); //_tstoi
		else 
			rsi.nComPort=-1;
		asi.Add(rsi);
	}
#else
	// Use different techniques to enumerate the available serial
	// ports, depending on the OS we're using
	OSVERSIONINFO vi;
	vi.dwOSVersionInfoSize = sizeof(vi);
	if (!::GetVersionEx(&vi)) {
		return FALSE;
	}
	// Handle windows 9x and NT4 specially
	if (vi.dwMajorVersion < 5) {
		return FALSE;
	}
	else {
		// Win2k and later support a standard API for
		// enumerating hardware devices.
		EnumPortsWdm(asi);
	}

	for (int ii=0; ii<asi.GetSize(); ii++)
	{
		SSerInfo& rsi = asi[ii];    
		if (bIgnoreBusyPorts) {
			// Only display ports that can be opened for read/write
			HANDLE hCom = CreateFile(rsi.strDevPath,
				GENERIC_READ | GENERIC_WRITE,
				0,    /* comm devices must be opened w/exclusive-access */
				NULL, /* no security attrs */
				OPEN_EXISTING, /* comm devices must use OPEN_EXISTING */
				0,    /* not overlapped I/O */
				NULL  /* hTemplate must be NULL for comm devices */
				);
			if (hCom == INVALID_HANDLE_VALUE) {
				// It can't be opened; remove it.
				asi.RemoveAt(ii);
				ii--;               
				continue;
			}
			else {
				// It can be opened! Close it and add it to the list
				::CloseHandle(hCom);
			}
		}

		// Come up with a name for the device.
		// If there is no friendly name, use the port name.
		if (rsi.strFriendlyName.IsEmpty())
			rsi.strFriendlyName = rsi.strPortName;

		// If there is no description, try to make one up from
		// the friendly name.
		if (rsi.strPortDesc.IsEmpty()) 
		{
			// If the port name is of the form "ACME Port (COM3)"
			// then strip off the " (COM3)"
			rsi.strPortDesc = rsi.strFriendlyName;
			int startdex = rsi.strPortDesc.Find(_T(" ("));
			int enddex = rsi.strPortDesc.Find(_T(")"));
			if (startdex > 0 && enddex == (rsi.strPortDesc.GetLength()-1))
			{
				rsi.strPortDesc = rsi.strPortDesc.Left(startdex); // 
				// 				AfxMessageBox(rsi.strPortDesc);
			}
		}
		if (rsi.strPortName.IsEmpty()) 
		{
			// If the port name is of the form "ACME Port (COM3)"
			// then strip off the "COM3"
			rsi.strPortName = rsi.strFriendlyName;
			int startdex = rsi.strFriendlyName.Find(_T(" ("));
			int enddex = rsi.strFriendlyName.Find(_T(")"));
			if (startdex > 0 && enddex == (rsi.strFriendlyName.GetLength()-1))
			{
				rsi.strPortName=rsi.strFriendlyName.Mid(startdex+2,enddex-startdex-2);
			}
		}
		rsi.strPortName.MakeUpper();
		int idx=rsi.strPortName.Find(_T("COM"));
		if(idx>=0)
			rsi.nComPort=_tstoi(rsi.strPortName.Mid(idx+3));
		else 
			rsi.nComPort=-1;
	}
#endif
	return TRUE;
}

int EnumDevice(IN LPGUID lpGuid,OUT CStringArray & strArrayDevName,BOOL bPresent,OUT  CStringArray *pstrArrayHID,OUT  CStringArray *pstrArrayHandle)
{
    HDEVINFO   hDevInfo;      
    SP_DEVINFO_DATA   DeviceInfoData;      
    DWORD   i;      
    CString temp;   
    CString str;   
//	strArray.RemoveAll();
	DWORD dflag=0;//DIGCF_PRESENT;
	if(bPresent)
		dflag|=DIGCF_PRESENT;
	if(lpGuid==NULL)
		dflag|=DIGCF_ALLCLASSES;
	
    hDevInfo   =   SetupDiGetClassDevs(lpGuid,   0,   0,dflag);       //|  
    if   (hDevInfo   ==   INVALID_HANDLE_VALUE)      
    {      
        //   Insert   error   handling   here.      
        return  0 ;      
    }      
   
    //   Enumerate   through   all   devices   in   Set.      
   
    DeviceInfoData.cbSize   =   sizeof(SP_DEVINFO_DATA);      
    for   (i=0;SetupDiEnumDeviceInfo(hDevInfo,i,&DeviceInfoData);i++)      
    {      
        DWORD   DataT;      
        //LPTSTR   buffer   =   NULL;      
        TCHAR   buffer[2048];      
        DWORD   buffersize   =sizeof(buffer);     
// 		if(bOnlyListHiden)
// 		{
// 			if(!IsClassHidden(&DeviceInfoData.ClassGuid))
// 				continue;
// 		}
		buffer[0]=0;
        while   (!SetupDiGetDeviceRegistryProperty(      
            hDevInfo,&DeviceInfoData,SPDRP_FRIENDLYNAME,//SPDRP_DEVICEDESC,    
            &DataT,(PBYTE)buffer,buffersize,&buffersize))      
        {      
            if   (GetLastError() != ERROR_INSUFFICIENT_BUFFER)      
                break;      
        }      
		if(buffer[0]==0)
		{
			while   (!SetupDiGetDeviceRegistryProperty(      
				hDevInfo,&DeviceInfoData,SPDRP_DEVICEDESC,    
				&DataT,(PBYTE)buffer,buffersize,&buffersize))      
			{      
				if   (GetLastError() != ERROR_INSUFFICIENT_BUFFER)      
					break;      
			}      
		}
//        temp.Format(_T("<VALUE>%s</VALUE>"),buffer);      
//        str   +=   temp;      
		str.Format(_T("%s"),buffer);
//		str.Format(_T("%d"),DeviceInfoData.DevInst);
		strArrayDevName.Add(str);
		if(pstrArrayHID)
		{
			buffer[0]=0;
			while   (!SetupDiGetDeviceRegistryProperty(
				hDevInfo,&DeviceInfoData,SPDRP_HARDWAREID,//SPDRP_FRIENDLYNAME,//SPDRP_DEVICEDESC,    
				&DataT,(PBYTE)buffer,buffersize,&buffersize))      
			{      
				if   (GetLastError() != ERROR_INSUFFICIENT_BUFFER)      
					break;      
			}
			TCHAR *p1,*p2=NULL;
			p1=buffer;
			for (int k=0;k<buffersize;k++)
			{
				if(k!=0 && buffer[k]==0)
				{
					p2=buffer+k+1; break;
				}
			}
			if(*p2==_T('*'))
				p2=p1;
			if(p2==NULL)
				p2=p1;
			str.Format(_T("%s"),p2); //buffer);
			pstrArrayHID->Add(str);
		}
		if(pstrArrayHandle)
		{
			str.Format(_T("%d"),DeviceInfoData.DevInst);
			pstrArrayHandle->Add(str);
		}
//        if   (buffer)   LocalFree(buffer);      
    }      
    if   (   GetLastError()!=NO_ERROR   &&      
        GetLastError()!=ERROR_NO_MORE_ITEMS   )      
    {      
        return 0  ;      
    }      
   
    //     Cleanup      
    SetupDiDestroyDeviceInfoList(hDevInfo);     
  return strArrayDevName.GetSize();
}
