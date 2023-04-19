#include "stdafx.h"
#include "utils.h"

#include <map>
#include <sstream>
#include <fstream>
#include <Shlwapi.h>

#pragma comment(lib,"Shlwapi.lib")

namespace httputils
{
  std::string TrimA(const std::string & str)
  {
    //去除首尾空格
    std::string s = str;
    s = s.erase(0, s.find_first_not_of(" "));

    s = s.erase(s.find_last_not_of(" ") + 1);

    return s;
  }

  std::string GetHostsPathA()
  {
    char system32Path[MAX_PATH];
    ExpandEnvironmentStringsA("%SystemRoot%\\System32", system32Path, MAX_PATH);
    std::string path = system32Path;
    //获取hosts 得路径 //drivers\etc\hosts
    std::string hosts_path;
    hosts_path.resize(MAX_PATH);

    PathCombineA((LPSTR)hosts_path.c_str(), path.c_str(), "drivers\\etc\\hosts");

    return hosts_path;
  }

  std::map<std::string, std::string> LoadHostsFile()
  {
    std::map<std::string, std::string> hosts;

    std::string hosts_path = GetHostsPathA();

    std::ifstream file(hosts_path.c_str());
    if (file.is_open())
    {
      std::string line;
      while (std::getline(file, line))
      {
        //去除首尾多余得空格
        line = TrimA(line);
        if (line.empty() || line[0] == '#')
          continue;

        std::istringstream iss(line);
        std::string ip, hostname;
        iss >> ip >> hostname;
        hosts[hostname] = ip;
      }

      file.close();
    }

    return hosts;
  }
}

namespace conversion
{
  wstring Utf2U(const string& strUtf8)
  {
    wstring wstrRet(L"");
    int nLen = MultiByteToWideChar(CP_UTF8, 0, strUtf8.c_str(), -1, NULL, 0);
    if (nLen == ERROR_NO_UNICODE_TRANSLATION)
      throw "Utf8ToUnicode出错：无效的UTF-8字符串。";
    wstrRet.resize(nLen + 1, '\0');
    MultiByteToWideChar(CP_UTF8, 0, strUtf8.c_str(), -1, (LPWSTR)wstrRet.c_str(), nLen);
    return wstrRet;
  }

  string U2A(const wstring& str)
  {
    string strDes;
    if (str.empty())
      goto __end;
    int nLen = ::WideCharToMultiByte(CP_ACP, 0, str.c_str(), str.size(), NULL, 0, NULL, NULL);
    if (0 == nLen)
      goto __end;
    char* pBuffer = new char[nLen + 1];
    memset(pBuffer, 0, nLen + 1);
    ::WideCharToMultiByte(CP_ACP, 0, str.c_str(), str.size(), pBuffer, nLen, NULL, NULL);
    pBuffer[nLen] = '\0';
    strDes.append(pBuffer);
    delete[] pBuffer;
  __end:
    return strDes;
  }

  wstring A2U(const string& str)
  {
    wstring strDes;
    if (str.empty())
      goto __end;
    int nLen = ::MultiByteToWideChar(CP_ACP, 0, str.c_str(), str.size(), NULL, 0);
    if (0 == nLen)
      goto __end;
    wchar_t* pBuffer = new wchar_t[nLen + 1];
    memset(pBuffer, 0, nLen + 1);
    ::MultiByteToWideChar(CP_ACP, 0, str.c_str(), str.size(), pBuffer, nLen);
    pBuffer[nLen] = '\0';
    strDes.append(pBuffer);
    delete[] pBuffer;
  __end:
    return strDes;
  }
}

