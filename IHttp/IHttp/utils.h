#pragma once
#include <string>
#include <map>

namespace httputils
{
  std::string TrimA(const std::string & str);
  std::string GetHostsPathA();
  std::map<std::string, std::string> LoadHostsFile();
}

namespace conversion
{
  using namespace std;
  wstring Utf2U(const string& strUtf8);

  string U2A(const wstring& str);

  wstring A2U(const string& str);
}
