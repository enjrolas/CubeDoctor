/*
by 王步军 2015-10-23
 */

#ifndef MAIN_H
#define MAIN_H

#include <Windows.h>

extern HWND g_hWnd; //=NULL;

#define MY_PROGRESS_MESSAGE	WM_USER+101	//进度条用
#define MPMSG_RANGE		0		//进度条范围(最大值)
#define MPMSG_CURPOS	1		//进度条当前位置值
#define MY_PRINT_MESSAGE	WM_USER+102	//显示信息用
#define MY_DFUMODE_MESSAGE	WM_USER+103	//DFU mode信息
#define MDMSG_NO		0		//没查到
#define MDMSG_DFU		1		//DFU mode
#define MDMSG_RUNTIME	2		//Runtime

#define SEND_DFU_MESSAGE(m)	SendMessage(g_hWnd,MY_DFUMODE_MESSAGE,m,0)

#define SET_PROGRESS_RANGE_MESSAGE(x)	SendMessage(g_hWnd,MY_PROGRESS_MESSAGE,MPMSG_RANGE,(LPARAM)x)
#define SET_PROGRESS_CURPOS_MESSAGE(x)	SendMessage(g_hWnd,MY_PROGRESS_MESSAGE,MPMSG_CURPOS,(LPARAM)x)

//#define PRINTF_MESSAGE(x)	printf(x)
#define PRINTF_MESSAGE(x)	SendMessage(g_hWnd,MY_PRINT_MESSAGE,1,(LPARAM)x)

//#define PRINTF_STDERR_MESSAGE(x)	fprintf(stderr,x)
#define PRINTF_STDERR_MESSAGE(x)	SendMessage(g_hWnd,MY_PRINT_MESSAGE,2,(LPARAM)x)

//#define MYPRINTF(x)	printf(x)
#define MYPRINTF(...) do { char ss[2048];\
	sprintf(ss, __VA_ARGS__); PRINTF_MESSAGE(ss); } while (0)


#ifdef	__cplusplus
extern "C" {
#endif
	//根据类似程序命令行参数串，执行DFU相应功能
	int DoDfuUtil(HWND hWnd,char szCmdLine[]);
	//取dfu模式信息到pszInfo,pszInfo=NULL时,只返回值,nMaxLen:最大长度,=-1:时不限
	//返回模式代号MDMSG_NO(0):no,MDMSG_DFU(1):DFU,MDMSG_RUNTIME(2):Runtime
	int GetDfuMode(HWND hWnd,__out char *pszInfo,int nMaxLen);
#ifdef	__cplusplus
}
#endif

#endif /* MAIN_H */
