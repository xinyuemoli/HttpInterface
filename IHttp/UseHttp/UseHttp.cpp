// UseHttp.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include "stdafx.h"
#include "..//IHttp/IHttpInterface.h"
#ifdef _DEBUG
#pragma comment(lib, "../lib/IHttpD.lib")
#else
#pragma comment(lib, "../lib/IHttp.lib")
#endif
#include <crtdbg.h>
#include <Windows.h>


bool TestWinInet();
bool TestWinHttp();
bool TestSocketHttp();
bool TestDownloadFile();

//�����ļ��Ļص��࣬��ʾ���ؽ���&��������
class CMyCallback
	: public IHttpCallback
{
public:
	virtual void OnDownloadCallback(void* pParam, DownloadState state, double nTotalSize, double nLoadSize)
	{
		if (nTotalSize>0)
		{
			int nPercent = (int)( 100*(nLoadSize/nTotalSize) );
			printf("���ؽ��ȣ�%d%%\n", nPercent);
		}
	}
	virtual bool IsNeedStop(void* Params)
	{
		//�����Ҫ���ⲿ��ֹ���أ�����true
		return false;//��������
	}
};

int _tmain(int argc, _TCHAR* argv[])
{
	//TestWinInet();		//����ʹ��WinInetʵ�ֵ�HTTP�ӿ�
	TestWinHttp();		//����ʹ��WinHttpʵ�ֵ�HTTP�ӿ�
	//TestSocketHttp();		//����ʹ��Socketʵ�ֵ�HTTP�ӿ�
	//TestDownloadFile();	//���������ļ���ʹ�ûص��ӿڻ�ȡ���ؽ���

	system("pause");
	//��ӡ���ڴ�й©��Ϣ
 	//_CrtDumpMemoryLeaks();
	return 0;
}

bool TestWinInet()
{
	IWininetHttp* pHttp;
	bool bRet = CreateInstance((IHttpBase**)&pHttp, TypeWinInet);
	if (!bRet)
	{
		return false;
	}
	char* pMem = NULL;
	int nSize = 0;
	const wchar_t* pUrl = L"https://blog.csdn.net/mfcing";
	//����Զ���httpͷ��Ϣ
	pHttp->AddHeader("name", "Jelin");
	pHttp->AddHeader("address", "Shanghai");
	string str = pHttp->Request(pUrl, HttpGet);
	int http_code = pHttp->GetResponseCode();
	if (str.empty())
	{
		//����ʧ��
		pHttp->FreeInstance();
		return false;
	}
	if (pHttp->DownloadToMem(pUrl, (void**)&pMem, &nSize))
	{//���ص��ڴ��У������ص��������Ч�ʸ��ߣ����ö�д�ļ������������ļ�С�ļ���
		
		//����֮��һ��Ҫ�ͷ�����ڴ�ռ�
		free(pMem);
	}
	else
	{
		//����ʧ�ܣ���ȡ������Ϣ
		DWORD dwCode = GetLastError();
		HttpInterfaceError code = pHttp->GetErrorCode();
		pHttp->FreeInstance();
		return false;
	}
	//����post����
	std::string ret = pHttp->Request("https://chat.jelinyao.cn/postTest", HttpPost, "{\"name\":\"Jelin\",\"address\":\"Shanghai\"}");
	pHttp->FreeInstance();
	return true;
}

bool TestWinHttp()
{
	IWinHttp* pHttp;
	bool bRet = CreateInstance((IHttpBase**)&pHttp, TypeWinHttp);
	if (!bRet)
	{
		return false;
	}
	const char* pUrl = "https://www.qq.com";
	//����Զ���httpͷ��Ϣ
	/*pHttp->AddHeader("name", "Jelin");
	pHttp->AddHeader("address", "Shanghai");*/
	string strHtml = pHttp->Request(L"http://dk.mdown.net/v1/count"/*pUrl*/, HttpPost,"111");
	int http_code = pHttp->GetResponseCode();
	if (strHtml.empty())
	{
		//����ʧ��
		pHttp->FreeInstance();
		return false;
	}
	else
	{
		printf("%s html : %s\n", pUrl, strHtml.c_str());
	}
	//����post����
	std::string ret = pHttp->Request("https://chat.jelinyao.cn/postTest", HttpPost, "{\"name\":\"Jelin\",\"address\":\"Shanghai\"}");
	pHttp->FreeInstance();
	return true;
}

bool TestSocketHttp()
{
	//ʹ��winsock֮ǰ��Ҫ���г�ʼ��
	InitWSASocket();
	ISocketHttp* pHttp;
	bool bRet = CreateInstance((IHttpBase**)&pHttp, TypeSocket);
	if (!bRet)
	{
		UninitWSASocket();
		return false;
	}
	const wchar_t* pUrl = L"http://www.hbsrsksy.cn/hdhdhd";
	char* pHtml = NULL;
	int nSize = 0;
	//����Զ���httpͷ��Ϣ
	pHttp->AddHeader("name", "Jelin");
	pHttp->AddHeader("address", "Shanghai");
	//������ҳ���ݵ��ڴ��У����ڴ���malloc��̬���룬ʹ�ú���Ҫ�ֶ��ͷ�
	if (!pHttp->DownloadToMem(pUrl, (void**)&pHtml, &nSize))
	{
		//����ʧ��
		int http_code = pHttp->GetResponseCode();
		pHttp->FreeInstance();
		UninitWSASocket();
		return false;
	}
	printf("html: %s\n", pHtml);
	//�ͷ��ڴ�ռ�
	free(pHtml);
	pHttp->FreeInstance();
	UninitWSASocket();
	return true;
}

bool TestDownloadFile()
{
	IWinHttp* pHttp;
	bool bRet = CreateInstance((IHttpBase**)&pHttp, TypeWinHttp);
	if (!bRet)
	{
		return false;
	}
	CMyCallback cb;
	pHttp->SetDownloadCallback(&cb, NULL);
	const wchar_t* pUrl = L"https://pm.myapp.com/invc/xfspeed/qqsoftmgr/QQSoftDownloader_v1.1_webnew_22127@.exe";
	const wchar_t* pSavePath = L"c:\\down.exe";
	if (!pHttp->DownloadFile(pUrl, pSavePath))
	{
		//����ʧ��
		DWORD dwCode = GetLastError();
		HttpInterfaceError error = pHttp->GetErrorCode();
		pHttp->FreeInstance();
		return false;
	}
	pHttp->FreeInstance();
	return true;
}