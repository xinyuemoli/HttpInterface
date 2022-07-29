/*****************************************************************
*HTTP�����࣬��Ҫ����HTTP GET/POST�����أ�֧���ض��򣩹���
*Author��	JelinYao
*Date��		2015/2/14 12:11
*Email��	mailto://jelinyao@163.com
*/
/*****************************************************************
*/
#pragma once
#include <stdio.h>
#include <tchar.h>
#include <string>
using std::string;
using std::wstring;


enum HttpRequest
{
	HttpGet = 0,
	HttpPost,
};
//ö������״̬
enum DownloadState
{
	HttpLoading = 0,
	HttpFialed,
	HttpFinished,
};

/******************************************************
*���������Ϣ
*
******************************************************/
enum HttpInterfaceError
{
	HttpErrorSuccess = 0,		//�ɹ�
	HttpErrorInit,				//��ʼ��ʧ��
	HttpErrorConnect,			//����HTTP������ʧ��
	HttpErrorSend,				//��������ʧ��
	HttpErrorQuery,				//��ѯHTTP����ͷʧ��
	HttpError404,				//ҳ�治����
	HttpErrorIllegalUrl,		//��Ч��URL
	HttpErrorCreateFile,		//�����ļ�ʧ��
	HttpErrorDownload,			//����ʧ��
	HttpErrorQueryIP,			//��ȡ������Ӧ�ĵ�ַʧ��
	HttpErrorSocket,			//�׽��ִ���
	HttpErrorUserCancel,		//�û�ȡ������
	HttpErrorBuffer,			//�ļ�̫�󣬻���������
	HttpErrorHeader,			//HTTP����ͷ����
	HttpErrorParam,				//�������󣬿�ָ�룬���ַ�����
	HttpErrorWriteFile,			//д���ļ�ʧ��
	HttpErrorUnknow,			//δ֪����

};




//���صĻص�
class IHttpCallback
{
public:
	virtual void	OnDownloadCallback(void* pParam, DownloadState state, double nTotalSize, double nLoadSize) = 0;
	virtual bool	IsNeedStop() = 0;
};

class IHttpBase
{
public:
	virtual void SetDownloadCallback(IHttpCallback* pCallback, void* pParam) = 0;
	virtual bool DownloadFile(LPCWSTR lpUrl, LPCWSTR lpFilePath) = 0;
	virtual bool DownloadToMem(LPCWSTR lpUrl, OUT void** ppBuffer, OUT int* nSize) = 0;
	virtual void FreeInstance() = 0;
	virtual HttpInterfaceError GetErrorCode() = 0;
	virtual void AddHeader(LPCSTR key, LPCSTR value) = 0;//���HTTP����ͷ
	virtual int GetResponseCode() = 0;// ��ȡHTTP������������
};

////////////////////////////////////////////////////////////////////////////////////
//HTTP����ӿ���
class IWininetHttp
	:public IHttpBase
{
public:
	//HTTP������
	virtual string	Request(LPCSTR lpUrl, HttpRequest type, LPCSTR lpPostData = NULL, LPCSTR lpHeader = NULL) = 0;
	virtual string	Request(LPCWSTR lpUrl, HttpRequest type, LPCSTR lpPostData = NULL, LPCWSTR lpHeader = NULL) = 0;
};


///////////////////////////////////////////////////////////////////////////////////////
//HTTP socket��

class ISocketHttp
	:public IHttpBase
{
public:
	virtual LPCWSTR	GetIpAddr()const = 0;
};

///////////////////////////////////////////////////////////////////////////////////////
//WinHttp��
class IWinHttp
	: public IWininetHttp
{
public:
	//���ó�ʱʱ�䣬��λ������
	virtual void	SetTimeOut(int dwConnectTime, int dwSendTime, int dwRecvTime) = 0;
};



/////////////////////////////////////////////////////////////////////////////////
#ifndef USE_SATIC
  //DLL�ĵ�����������
  #ifdef _USRDLL//������
  #define LIB_FUNCTION extern "C" __declspec(dllexport)
  #else
  #define LIB_FUNCTION extern "C" __declspec(dllimport)
  #endif
#else 
#define LIB_FUNCTION
#endif

/***********************************************************
*����������������
*
************************************************************
*/

enum InterfaceType
{
	TypeSocket = 0,
	TypeWinInet,
	TypeWinHttp,
};

LIB_FUNCTION bool CreateInstance(IHttpBase** pBase, InterfaceType flag);

//��ʼ��Windowsϵͳ����⣺Winsocket2
LIB_FUNCTION void InitWSASocket();

//ж��ϵͳ�����
LIB_FUNCTION void UninitWSASocket();



