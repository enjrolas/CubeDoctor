/*
by ������ 2015-10-23
 */

#ifndef MAIN_H
#define MAIN_H

#include <Windows.h>

extern HWND g_hWnd; //=NULL;

#define MY_PROGRESS_MESSAGE	WM_USER+101	//��������
#define MPMSG_RANGE		0		//��������Χ(���ֵ)
#define MPMSG_CURPOS	1		//��������ǰλ��ֵ
#define MY_PRINT_MESSAGE	WM_USER+102	//��ʾ��Ϣ��
#define MY_DFUMODE_MESSAGE	WM_USER+103	//DFU mode��Ϣ
#define MDMSG_NO		0		//û�鵽
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
	//�������Ƴ��������в�������ִ��DFU��Ӧ����
	int DoDfuUtil(HWND hWnd,char szCmdLine[]);
	//ȡdfuģʽ��Ϣ��pszInfo,pszInfo=NULLʱ,ֻ����ֵ,nMaxLen:��󳤶�,=-1:ʱ����
	//����ģʽ����MDMSG_NO(0):no,MDMSG_DFU(1):DFU,MDMSG_RUNTIME(2):Runtime
	int GetDfuMode(HWND hWnd,__out char *pszInfo,int nMaxLen);
#ifdef	__cplusplus
}
#endif

#endif /* MAIN_H */
