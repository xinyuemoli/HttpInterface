#pragma once
#include <string>
#include "utils.h"

using std::wstring;
using std::string;

inline bool FileExistA(LPCSTR lpFile)
{
  HANDLE hFile = CreateFileA(lpFile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
  if (hFile == INVALID_HANDLE_VALUE)
    return false;
  CloseHandle(hFile);
  return true;
}

inline bool FileExistW(LPCWSTR lpFile)
{
  HANDLE hFile = CreateFileW(lpFile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
  if (hFile == INVALID_HANDLE_VALUE)
    return false;
  CloseHandle(hFile);
  return true;
}

inline bool isEmptyString(LPCSTR lpStr)
{
  if (NULL == lpStr || strlen(lpStr) == 0)
  {
    return true;
  }
  return false;
}

inline void MyParseUrlW( LPCWSTR lpUrl, wstring& strHostName, wstring& strPage, WORD& sPort )
{
	sPort = 80;
	wstring strTemp(lpUrl);
	int nPos=strTemp.find(L"http://");
	if (wstring::npos != nPos )
		strTemp=strTemp.substr(nPos+7, strTemp.size()-nPos-7);
	else
	{
		nPos = strTemp.find(L"https://");
		if(wstring::npos != nPos)
		{
			sPort = 443;//INTERNET_DEFAULT_HTTPS_PORT;
			strTemp=strTemp.substr(nPos+8, strTemp.size()-nPos-8);
		}
	}
	nPos = strTemp.find('/');
	if ( wstring::npos == nPos )//没有找到 /
		strHostName=strTemp;
	else
		strHostName = strTemp.substr(0, nPos);

  //这里支持下hosts文件
  std::map<std::string, std::string> hosts = httputils::LoadHostsFile();
  for (auto item:hosts)
  {
    std::string host_name = item.first;
    std::wstring w_host_name = conversion::A2U(host_name);
    int find_pos = strHostName.find(w_host_name);
    if (find_pos != std::wstring::npos)
    {
      
      std::string ip = item.second;
      std::wstring w_ip = conversion::A2U(ip);
      //替换
      strHostName = strHostName.replace(strHostName.begin() + find_pos, strHostName.begin() + find_pos + w_host_name.length(), w_ip.c_str());
    }
  }

	int nPos1 = strHostName.find(':');
	if ( nPos1 != wstring::npos )
	{
		wstring strPort = strTemp.substr(nPos1+1, strHostName.size()-nPos1-1);
		strHostName = strHostName.substr(0, nPos1);
		sPort = (WORD)_wtoi(strPort.c_str());
	}
	if (wstring::npos == nPos) {
		strPage = '/';
		return;
	}
	strPage = strTemp.substr(nPos, strTemp.size() - nPos);
}

inline void MyParseUrlA( LPCSTR lpUrl, string& strHostName, string& strPage, WORD& sPort )
{
	sPort = 80;
	string strTemp(lpUrl);
	int nPos	= strTemp.find("http://");
	if ( string::npos != nPos )
		strTemp	= strTemp.substr(nPos+7, strTemp.size()-nPos-7);
	else
	{
		nPos = strTemp.find("https://");
		if (wstring::npos != nPos)
		{
			sPort = 443;//INTERNET_DEFAULT_HTTPS_PORT;
			strTemp = strTemp.substr(nPos + 8, strTemp.size() - nPos - 8);
		}
	}
	nPos = strTemp.find('/');
	if ( string::npos == nPos )//没有找到 /
		strHostName	= strTemp;
	else
		strHostName = strTemp.substr(0, nPos);
	int nPos1 = strHostName.find(':');
	if ( nPos1 != string::npos )
	{
		string strPort = strTemp.substr(nPos1+1, strHostName.size()-nPos1-1);
		strHostName = strHostName.substr(0, nPos1);
		sPort = (WORD)atoi(strPort.c_str());
	}
	if (string::npos == nPos) {
		strPage = '/';
		return;
	}
	strPage = strTemp.substr(nPos, strTemp.size() - nPos);
}
