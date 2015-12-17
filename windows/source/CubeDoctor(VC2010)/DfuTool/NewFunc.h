
#include <afxtempl.h>

#pragma once

#ifndef GET_CHANBIT
#define GET_CHANBIT(bitbuf,chan)	(bitbuf[(chan)/8]&(1<<((chan)%8)))
#define SET_CHANBIT(bitbuf,chan)	(bitbuf[(chan)/8]|=(1<<((chan)%8)))
#define CLR_CHANBIT(bitbuf,chan)	(bitbuf[(chan)/8]&=~(1<<((chan)%8)))
#endif

#ifndef GET_BYTEBIT
#define GET_BYTEBIT(bv,n)			((bv)&(1<<n))
#define SET_BYTEBIT(bv,n)			(bv)|=(1<<n)
#define	CLR_BYTEBIT(bv,n)			(bv)&=~(1<<n)
#endif

enum {TIMEMODE_HOUR=0,TIMEMODE_MIN=1,TIMEMODE_SEC=2,
	TIMEMODE_MARK_UNIT1=0x8000,	//ֻ�������д(һ���ַ�)��ʾ��λ
	TIMEMODE_MARK_UNIT2=0x4000	//ֻ��������д(һ������ַ�)��ʾ��λ
};
LPTSTR ToTimeStr(time_t tv,int nTimeMode,LPTSTR szOutStr);
LPTSTR ToTimeStr(time_t tv,int nTimeMode,CString & strOut);

void AddStrToFile(LPCTSTR lpfname,LPCTSTR szStr,BOOL bAddCurTime=TRUE);

//��nValue������str,��nValue=0:���,����ת��intֵ
void SetStringInt(CString & str,int nValue);
//ת����strΪUINTֵ,Ϊ����ʱ,�þ���ֵ
UINT GetStringUInt(CString & str);

//ȡ�������ִ�������GetTimeDigitStr,m=0
char *GetDateStr(time_t tTime,char sdate[]);
//ȡʱ������ִ�,����szTime[],m=0:����,��:"070203"(2007-02-03),m=1:����ʱ��,"070203123000"
//m=2:û���������ʱ�� "0702031230"
char *GetTimeDigitStr(time_t tTime,char szTime[],int m);
//ȡ��ǰʱ�䴮���õĸ�ʽ"%Y-%m-%d %H:%M:%S"
CString GetCurrentTimeStr();
CString GetCurrentTimeStr(time_t tTime);
//����ttime������ʱ�䴮(��"%Y-%m-%d %H:%M:%S"��ʽ) 
char *GetDateTimeStr(time_t ttime,char *str);

int PrintMsgWarning(LPCTSTR fmt, ...);
int PrintMsgError(LPCTSTR fmt, ...);
int PrintMsgInfo(LPCTSTR fmt, ...);
//void PrintPrompt(LPCTSTR fmt, ...);	
void PrintMsgToFile(LPCTSTR fname,LPCTSTR fmt, ...);

//ȡcBitByte[]λ��,nMaxBitNum:���λ��
int GetBitNum(BYTE cBitByte[],int nMaxBitNum);
//ȡλ������Ч�ֽ���(�����һ����1��λռ���ֽ���),nMaxBitNum:���λ��
int GetValidBitByteNum(BYTE cBitByte[],int nMaxBitNum);

void PrintHexMessage(LPCTSTR lpBuf,int nlen);
CString ToHexStr(BYTE buf[],int len);

void EnableControl(CWnd *pWnd,UINT id[],int num,BOOL bEnable=TRUE);
void ShowControl(CWnd *pWnd,UINT id[],int num,BOOL bShow=TRUE);

void LocateWindow(CWnd *pWnd,CWnd *pLocationWnd,CWnd *pParent);
void LocateWindow(UINT nID,UINT nLocationID,CWnd *pParent);


CString OpenDir(LPCTSTR  cDlgName);  
BOOL IsFileNameValid(LPCTSTR lpFileName);
int Touch(LPCTSTR lpPath, BOOL bValidate);
//void BackupOldChlDateFile(const char *pszChlDateFile);
int round(double fv);

LPITEMIDLIST ParsePidlFromPath(LPCSTR path);

//��szStr������','�Ÿ�������ȡ��������fResult[]��,nMaxN:���ȡ�ĸ���,����ʵ�ʸ���
//�� 1.23,45.6,12.45 ��
int GetFloatFromStr(IN LPCTSTR szStr,OUT float fResult[],IN int nMaxN);
//��fResult[]�еĸ�����ת���ڴ�����szStr��(��','�Ÿ���),����ʵ�ʸ���,szStr[]���MAX_PATH��,�������򷵻�0
//�� 1.23,45.6,12.45 ��
int SetFloatToStr(IN float fResult[],IN int nNum,OUT LPTSTR szStr);

//���ļ�ȫ����ȡ�ļ�������(Ŀ¼��)
CString GetFolderFromFullpath(LPCTSTR lpFullpath,OUT TCHAR *pFileName=NULL,OUT TCHAR *pExtName=NULL);
//ȡ�ļ�����lpszStr·��������ļ���,���ظõ�ַ,������(��������չ��)��pLen��
TCHAR *GetFileName(LPCTSTR lpszStr,int *pLen);

/* Helper functions to access DLLs */
static __inline HMODULE GetDLLHandle(char* szDLLName)
{
	HANDLE h = GetModuleHandleA(szDLLName);
	if (h == NULL) {
		// coverity[alloc_fn][var_assign]
		h = LoadLibraryA(szDLLName);
	}
	return (HMODULE)h;
}

BOOL is_x64(void);

// Windows versions
enum WindowsVersion {
	WINDOWS_UNDEFINED = -1,
	WINDOWS_UNSUPPORTED = 0,
	WINDOWS_XP = 0x51,
	WINDOWS_2003 = 0x52,	// Also XP x64
	WINDOWS_VISTA = 0x60,
	WINDOWS_7 = 0x61,
	WINDOWS_8 = 0x62,
	WINDOWS_8_1_OR_LATER = 0x63,
	WINDOWS_MAX
};

#define safe_free(p) do {if ((void*)p != NULL) {free((void*)p); p = NULL;}} while(0)
#define safe_min(a, b) min((size_t)(a), (size_t)(b))
#define safe_strcp(dst, dst_max, src, count) do {memcpy(dst, src, safe_min(count, dst_max)); \
	((char*)dst)[safe_min(count, dst_max)-1] = 0;} while(0)
#define safe_strcpy(dst, dst_max, src) safe_strcp(dst, dst_max, src, safe_strlen(src)+1)
#define safe_strncat(dst, dst_max, src, count) strncat(dst, src, safe_min(count, dst_max - safe_strlen(dst) - 1))
#define safe_strcat(dst, dst_max, src) safe_strncat(dst, dst_max, src, safe_strlen(src)+1)
#define safe_strcmp(str1, str2) strcmp(((str1==NULL)?"<NULL>":str1), ((str2==NULL)?"<NULL>":str2))
#define safe_stricmp(str1, str2) _stricmp(((str1==NULL)?"<NULL>":str1), ((str2==NULL)?"<NULL>":str2))
#define safe_strncmp(str1, str2, count) strncmp(((str1==NULL)?"<NULL>":str1), ((str2==NULL)?"<NULL>":str2), count)
#define safe_closehandle(h) do {if (h != INVALID_HANDLE_VALUE) {CloseHandle(h); h = INVALID_HANDLE_VALUE;}} while(0)
#define safe_sprintf(dst, count, ...) do {_snprintf(dst, count, __VA_ARGS__); (dst)[(count)-1] = 0; } while(0)
#define safe_strlen(str) ((((char*)str)==NULL)?0:strlen(str))
#define safe_strdup _strdup
#define MF_CHECK(cond) ((cond)?MF_CHECKED:MF_UNCHECKED)
#define IGNORE_RETVAL(expr) do { (void)(expr); } while(0)

#if defined(_MSC_VER)
#define safe_vsnprintf(buf, size, format, arg) _vsnprintf_s(buf, size, _TRUNCATE, format, arg)
#else
#define safe_vsnprintf vsnprintf
#endif

const char* PrintWinVersion(OUT int & nWindowsVersion);

bool LoadImageFromResource(IN CImage* pImage,IN UINT nResID, IN LPCTSTR lpTyp=_T("PNG"));

// Struct used when enumerating the available serial ports
// Holds information about an individual serial port.
struct SSerInfo {
	SSerInfo() : bUsbDevice(FALSE) {}
	CString strDevPath;          // Device path for use with CreateFile()
	CString strPortName;         // Simple name (i.e. COM1)
	CString strFriendlyName;     // Full name to be displayed to a user
	BOOL bUsbDevice;             // Provided through a USB connection?
	CString strPortDesc;         // friendly name without the COMx
	int		nComPort;			 // 1~
};
BOOL EnumSerialPorts(CArray<SSerInfo,SSerInfo&> &asi, BOOL bIgnoreBusyPorts);
