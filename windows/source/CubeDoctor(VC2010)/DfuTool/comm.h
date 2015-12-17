#if !defined(AFX_COMM_H__7742400D_6F8D_11D5_8D6F_00105A83C2FF__INCLUDED_)
#define AFX_COMM_H__7742400D_6F8D_11D5_8D6F_00105A83C2FF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif 


#define MAXRECEBLOCK	512		//4096
#define MAXSENDBLOCK	512		//4096

class CComm
{
//protected:
private:
	COMSTAT		ComState;   //ͨ���豸״̬
	BOOL		m_bOpen;
//	CWinThread *m_pThread;		//���մ�����߳�
	DCB			dcb;         //ͨ���豸��DCB�ṹ
public:
	HANDLE		m_hCom;       //ͨ���豸���
	OVERLAPPED	m_osWrite;
	OVERLAPPED	m_osRead;
	volatile HANDLE m_hWaitEvent;	//���ڵȴ�,��ֹ����
//	volatile HANDLE m_hWriteEvent;	//����ͨ���������̼߳�ͬ�����¼����
	int			m_nComPort;			//���ں�(>=1)
public:
	CComm();
	virtual ~CComm();

private:
	BOOL DoInitialize();	//��ʼ��(m_osWrite,m_osRead,m_hWriteEvent...)

public:


	HANDLE GetHandle();//ȡ��ͨ���豸���
	//�Ƿ��Ѵ�
	BOOL IsOpen();
	//��������ʼ�����п�
	BOOL OpenComm(int m_PortNo=1,DWORD m_BaudRate=9600,BYTE m_ByteSize=8,
		BYTE m_Parity=ODDPARITY,BYTE m_StopBits=ONESTOPBIT);
	BOOL OpenComm(LPCSTR lpszCom,LPCSTR lpszDCB="COM1:9600,o,8,1");
	void CloseCommPort();  //�رմ��п�
	//���ô���ͨ���豸��DCB�ṹ
	BOOL SetCommDCB(DWORD m_BaudRate,BYTE m_ByteSize,BYTE m_Parity,BYTE m_StopBits);
	BOOL FlushComm(BOOL bRece,BOOL bSend);	//������շ��ͻ���
//	BOOL CreateCommThread();				//ͨѶ�߳�
	//����У��λ,bn:0-4=no,odd,even,mark,space,3��4���ڶ��ͨѶ
	//������ǰ��ֵ,0xff���ʾʧ��
	BYTE SetParityBit(BYTE bn);	
	//���ص�ǰУ��λֵ,0-4=no,odd,even,mark,space
	//0xff���ʾʧ��
	BYTE GetParityBit();

	//����WriteBuffer�е�dwWriteLength���ֽ�����
	BOOL Write(LPBYTE WriteBuffer,DWORD dwWriteLength,DWORD dwWriteTimeLim=100);
	//����dwMaxNumber���ֽ����ݷ���ReadBuffer��
	DWORD Read(LPBYTE ReadBuffer,DWORD dwMaxNumber,long dms=0);

	int GetReadLen();

};

#endif
