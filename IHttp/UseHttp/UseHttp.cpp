// UseHttp.cpp : 定义控制台应用程序的入口点。
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

//下载文件的回调类，显示下载进度&控制下载
class CMyCallback
	: public IHttpCallback
{
public:
	virtual void OnDownloadCallback(void* pParam, DownloadState state, double nTotalSize, double nLoadSize)
	{
		if (nTotalSize>0)
		{
			int nPercent = (int)( 100*(nLoadSize/nTotalSize) );
			printf("下载进度：%d%%\n", nPercent);
		}
	}
	virtual bool IsNeedStop(void* Params)
	{
		//如果需要在外部终止下载，返回true
		return false;//继续下载
	}
};

int _tmain(int argc, _TCHAR* argv[])
{
	//TestWinInet();		//测试使用WinInet实现的HTTP接口
	TestWinHttp();		//测试使用WinHttp实现的HTTP接口
	//TestSocketHttp();		//测试使用Socket实现的HTTP接口
	//TestDownloadFile();	//测试下载文件，使用回调接口获取下载进度

	system("pause");
	//打印出内存泄漏信息
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
  //预转换，得到所需空间的大小;
  int wcsLen = ::MultiByteToWideChar(CP_UTF8, NULL, szU8, strlen(szU8), NULL, 0);

  //分配空间要给'\0'留个空间，MultiByteToWideChar不会给'\0'空间
  wchar_t* wszString = new wchar_t[wcsLen + 1];

  //转换
  ::MultiByteToWideChar(CP_UTF8, NULL, szU8, strlen(szU8), wszString, wcsLen + 1);

  //最后加上'\0'
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
	//添加自定义http头信息
	pHttp->AddHeader("name", "Jelin");
	pHttp->AddHeader("address", "Shanghai");
	string str = pHttp->Request(pUrl, HttpGet);
	int http_code = pHttp->GetResponseCode();
	if (str.empty())
	{
		//请求失败
		pHttp->FreeInstance();
		return false;
	}
	if (pHttp->DownloadToMem(pUrl, (void**)&pMem, &nSize))
	{//下载到内存中，与下载到本地相比效率更高，不用读写文件（仅适用于文件小文件）
		
		//用完之后一定要释放这块内存空间
		free(pMem);
	}
	else
	{
		//下载失败，获取错误信息
		DWORD dwCode = GetLastError();
		HttpInterfaceError code = pHttp->GetErrorCode();
		pHttp->FreeInstance();
		return false;
	}
	//测试post请求
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
	////添加自定义http头信息
	///*pHttp->AddHeader("name", "Jelin");
	//pHttp->AddHeader("address", "Shanghai");*/
	//string strHtml = pHttp->Request(L"http://dk.mdown.net/v1/count"/*pUrl*/, HttpPost,"111");
	//int http_code = pHttp->GetResponseCode();
	//if (strHtml.empty())
	//{
	//	//请求失败
	//	pHttp->FreeInstance();
	//	return false;
	//}
	//else
	//{
	//	printf("%s html : %s\n", pUrl, strHtml.c_str());
	//}
	//测试post请求
  pHttp->AddHeader("Content-type", "text/plain");
  pHttp->AddHeader("User-Agent", "PostmanRuntime/7.29.2");
  std::wstring data = LR"({"keys":[{"mainkey":3,"subkey":"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\NpcapInst","is64":1},{"mainkey":3,"subkey":"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\TIM","is64":1},{"mainkey":3,"subkey":"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\ToDesk","is64":1},{"mainkey":3,"subkey":"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\WeChat","is64":1},{"mainkey":3,"subkey":"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\Wireshark","is64":1},{"mainkey":3,"subkey":"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\{3746f21b-c990-4045-bb33-1cf98cff7a68}","is64":1},{"mainkey":3,"subkey":"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\{5fe95b9d-9219-4d8b-a031-71323ae48a81}","is64":1},{"mainkey":3,"subkey":"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\{a98dc6ff-d360-4878-9f0a-915eba86eaf3}","is64":1},{"mainkey":3,"subkey":"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\{fcb5e4cc-2739-4812-97fd-f3b7c5d151ae}","is64":1},{"mainkey":3,"subkey":"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\百度云管家","is64":1},{"mainkey":2,"subkey":"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\Fiddler2","is64":1},{"mainkey":2,"subkey":"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\OneDriveSetup.exe","is64":1},{"mainkey":2,"subkey":"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\winManager","is64":1},{"mainkey":2,"subkey":"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\{371d0d73-d418-4ffe-b280-58c3e7987525}","is64":1},{"mainkey":2,"subkey":"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\{771FD6B0-FA20-440A-A002-3B3BAC16DC50}_is1","is64":1},{"mainkey":2,"subkey":"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\{a0cd04c4-102d-469b-81ff-9d4f18b061eb}","is64":1}]})";
	
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
	//使用winsock之前需要进行初始化
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
	//添加自定义http头信息
	pHttp->AddHeader("name", "Jelin");
	pHttp->AddHeader("address", "Shanghai");
	//下载网页内容到内存中，该内存由malloc动态申请，使用后需要手动释放
	if (!pHttp->DownloadToMem(pUrl, (void**)&pHtml, &nSize))
	{
		//下载失败
		int http_code = pHttp->GetResponseCode();
		pHttp->FreeInstance();
		UninitWSASocket();
		return false;
	}
	printf("html: %s\n", pHtml);
	//释放内存空间
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
		//下载失败
		DWORD dwCode = GetLastError();
		HttpInterfaceError error = pHttp->GetErrorCode();
		pHttp->FreeInstance();
		return false;
	}
	pHttp->FreeInstance();
	return true;
}