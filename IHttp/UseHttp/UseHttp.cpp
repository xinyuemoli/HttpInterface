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
#include <iostream>


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

std::string UnicodeToUtf8(const wchar_t * unicode)
{
  int len;
  len = WideCharToMultiByte(CP_UTF8, 0, (const wchar_t*)unicode, -1, NULL, 0, NULL, NULL);
  char *szUtf8 = new char[len + 1];
  memset(szUtf8, 0, len + 1);

  WideCharToMultiByte(CP_UTF8, 0, (const wchar_t*)unicode, -1, szUtf8, len, NULL, NULL);

  std::string str = szUtf8;
  delete[] szUtf8;

  return str;
}

std::wstring Utf8ToUnicode(const char * szU8)
{
  //Ԥת�����õ�����ռ�Ĵ�С;
  int wcsLen = ::MultiByteToWideChar(CP_UTF8, NULL, szU8, strlen(szU8), NULL, 0);

  //����ռ�Ҫ��'\0'�����ռ䣬MultiByteToWideChar�����'\0'�ռ�
  wchar_t* wszString = new wchar_t[wcsLen + 1];

  //ת��
  ::MultiByteToWideChar(CP_UTF8, NULL, szU8, strlen(szU8), wszString, wcsLen + 1);

  //������'\0'
  wszString[wcsLen] = '\0';

  std::wstring str = wszString;

  delete[] wszString;
  wszString = NULL;

  return str;
}

std::string WStringToString(const wchar_t * wc_str)
{
  std::string result = "";

  int nwstrlen = WideCharToMultiByte(CP_ACP, 0, wc_str, -1, NULL, 0, NULL, NULL);
  if (nwstrlen > 0)
  {
    std::string str(nwstrlen, 0);
    WideCharToMultiByte(CP_ACP, 0, wc_str, -1, (LPSTR)str.c_str(), nwstrlen, NULL, NULL);
    result = str;
  }
  //delete[] c_str;

  return result.c_str();
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
	//const char* pUrl = "https://www.qq.com";
	////����Զ���httpͷ��Ϣ
	///*pHttp->AddHeader("name", "Jelin");
	//pHttp->AddHeader("address", "Shanghai");*/
	//string strHtml = pHttp->Request(L"http://dk.mdown.net/v1/count"/*pUrl*/, HttpPost,"111");
	//int http_code = pHttp->GetResponseCode();
	//if (strHtml.empty())
	//{
	//	//����ʧ��
	//	pHttp->FreeInstance();
	//	return false;
	//}
	//else
	//{
	//	printf("%s html : %s\n", pUrl, strHtml.c_str());
	//}
	//����post����
  pHttp->AddHeader("Content-type", "text/plain");
  pHttp->AddHeader("User-Agent", "PostmanRuntime/7.29.2");
  std::wstring data = LR"({"keys":[{"mainkey":3,"subkey":"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\NpcapInst","is64":1},{"mainkey":3,"subkey":"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\TIM","is64":1},{"mainkey":3,"subkey":"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\ToDesk","is64":1},{"mainkey":3,"subkey":"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\WeChat","is64":1},{"mainkey":3,"subkey":"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\Wireshark","is64":1},{"mainkey":3,"subkey":"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\{3746f21b-c990-4045-bb33-1cf98cff7a68}","is64":1},{"mainkey":3,"subkey":"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\{5fe95b9d-9219-4d8b-a031-71323ae48a81}","is64":1},{"mainkey":3,"subkey":"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\{a98dc6ff-d360-4878-9f0a-915eba86eaf3}","is64":1},{"mainkey":3,"subkey":"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\{fcb5e4cc-2739-4812-97fd-f3b7c5d151ae}","is64":1},{"mainkey":3,"subkey":"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\�ٶ��ƹܼ�","is64":1},{"mainkey":2,"subkey":"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\Fiddler2","is64":1},{"mainkey":2,"subkey":"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\OneDriveSetup.exe","is64":1},{"mainkey":2,"subkey":"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\winManager","is64":1},{"mainkey":2,"subkey":"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\{371d0d73-d418-4ffe-b280-58c3e7987525}","is64":1},{"mainkey":2,"subkey":"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\{771FD6B0-FA20-440A-A002-3B3BAC16DC50}_is1","is64":1},{"mainkey":2,"subkey":"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\{a0cd04c4-102d-469b-81ff-9d4f18b061eb}","is64":1}]})";
	
  std::string post_data = UnicodeToUtf8(data.c_str());
  std::string ret = pHttp->Request("http://192.168.101.106/softNew/getSoftUpdate", HttpPost, post_data.c_str());

  //std::cout << ret.c_str() << std::endl;
  std::wstring wstr_respone =  Utf8ToUnicode(ret.c_str());

  ret = WStringToString(wstr_respone.c_str());

  
  std::cout << ret.c_str() << std::endl;

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