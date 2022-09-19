#include "stdafx.h"
#include "HttpClient.h"
#include "Winhttp.h"
#pragma comment(lib, "Winhttp")
#include <assert.h>
#include "../Common.h"



inline void CloseInternetHandle(HINTERNET* hInternet)
{
	if (*hInternet)
	{
		WinHttpCloseHandle(*hInternet);
		*hInternet = NULL;
	}
}


CWinHttp::CWinHttp(void)
	: m_hInternet(NULL)
	, m_hConnect(NULL)
	, m_hRequest(NULL)
	, m_nConnTimeout(5000)
	, m_nSendTimeout(5000)
	, m_nRecvTimeout(5000)
	, m_bHttps(false)
	, m_nResponseCode(0)
  , user_data_(nullptr)
{
	memset(&m_paramsData, 0, sizeof(HttpParamsData));
	Init();
}

CWinHttp::~CWinHttp(void)
{
	Release();
}

bool CWinHttp::Init()
{
	m_hInternet = ::WinHttpOpen(
		L"Microsoft Internet Explorer",
		WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
		WINHTTP_NO_PROXY_NAME,
		WINHTTP_NO_PROXY_BYPASS,
		0);
	if (NULL == m_hInternet)
	{
		m_paramsData.errcode = HttpErrorInit;
		return false;
	}
	::WinHttpSetTimeouts(m_hInternet, 0, m_nConnTimeout, m_nSendTimeout, m_nRecvTimeout);
	return true;
}

void CWinHttp::Release()
{
	CloseInternetHandle(&m_hRequest);
	CloseInternetHandle(&m_hConnect);
	CloseInternetHandle(&m_hInternet);
}

bool CWinHttp::ConnectHttpServer(LPCWSTR lpIP, WORD wPort)
{
	m_hConnect = ::WinHttpConnect(m_hInternet, lpIP, wPort, 0);
	return m_hConnect != NULL;
}

bool CWinHttp::CreateHttpRequest(LPCWSTR lpPage, HttpRequest type, DWORD dwFlag/*=0*/)
{
  wchar_t* pVerb = NULL;
  if (type == HttpGet)
  {
    pVerb = L"GET";
  }
  else if (type == HttpPost)
  {
    pVerb = L"POST";
  }
  else if (type == HTTPHead)
  {
    pVerb = L"HEAD";
  }

	m_hRequest = ::WinHttpOpenRequest(
		m_hConnect,
		pVerb,
		lpPage,
		NULL,
		WINHTTP_NO_REFERER,
		WINHTTP_DEFAULT_ACCEPT_TYPES,
		dwFlag);
	return m_hRequest != NULL;
}

void CWinHttp::SetTimeOut(int dwConnectTime, int dwSendTime, int dwRecvTime)
{
	m_nConnTimeout = dwConnectTime;
	m_nSendTimeout = dwSendTime;
	m_nRecvTimeout = dwRecvTime;
}

bool CWinHttp::DownloadFile(LPCWSTR lpUrl, LPCWSTR lpFilePath)
{
	Release();
	if (!Init())
		return false;
	bool bRet = false;
	DWORD dwBytesToRead = 0, dwFileSize = 0, dwReadSize = 0, dwRecvSize = 0;
	if (!InitConnect(lpUrl, HttpGet))
		return false;
	if (!QueryContentLength(dwFileSize))
	{
		m_paramsData.errcode = HttpErrorQuery;
		return false;
	}
	m_nResponseCode = QueryStatusCode();
	if (m_nResponseCode == HTTP_STATUS_NOT_FOUND){
		m_paramsData.errcode = HttpError404;
		return false;
	}

	HANDLE hFile = CreateFile(lpFilePath, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (INVALID_HANDLE_VALUE == hFile)
	{
		m_paramsData.errcode = HttpErrorCreateFile;
		return false;
	}
	SetFilePointer(hFile, dwFileSize, 0, FILE_BEGIN);
	SetEndOfFile(hFile);
	SetFilePointer(hFile, 0, NULL, FILE_BEGIN);
	if (!::WinHttpQueryDataAvailable(m_hRequest, &dwBytesToRead))
	{
		CloseHandle(hFile);
		DeleteFile(lpFilePath);
		return false;
	}
	void* lpBuff = malloc(READ_BUFFER_SIZE);
	while (true)
	{
    if (m_paramsData.callback && m_paramsData.callback->IsNeedStop(m_paramsData.lpparam))
      break;
		if (dwBytesToRead > READ_BUFFER_SIZE)
		{
			free(lpBuff);
			lpBuff = malloc(dwBytesToRead);
		}
		if (!::WinHttpReadData(m_hRequest, lpBuff, dwBytesToRead, &dwReadSize))
			break;
		DWORD dwWriteByte;
		if (!WriteFile(hFile, lpBuff, dwReadSize, &dwWriteByte, NULL) || (dwReadSize != dwWriteByte))
			break;
		dwRecvSize += dwReadSize;
		if (m_paramsData.callback)
			m_paramsData.callback->OnDownloadCallback(m_paramsData.lpparam, HttpLoading, dwFileSize, dwRecvSize);
		if (!::WinHttpQueryDataAvailable(m_hRequest, &dwBytesToRead))
			break;
		if (dwBytesToRead <= 0)
		{
			bRet = true;
      //�����������
      if (m_paramsData.callback)
        m_paramsData.callback->OnDownloadCallback(m_paramsData.lpparam, HttpFinished, dwFileSize, dwRecvSize);

			break;
		}
	}
	free(lpBuff);
	CloseHandle(hFile);
	if (!bRet)
	{//����ʧ�ܣ�ɾ���ļ�
		DeleteFile(lpFilePath);
	}
	return bRet;
}

bool CWinHttp::DownloadToMem(LPCWSTR lpUrl, OUT void** ppBuffer, OUT int* nSize)
{
	bool bResult = false;
	BYTE* lpFileMem = NULL;
	void* lpBuff = NULL;
	DWORD dwLength = 0, dwBytesToRead = 0, dwReadSize = 0, dwRecvSize = 0;
	try
	{
		if (!InitConnect(lpUrl, HttpGet))
			throw HttpErrorInit;
		if (!QueryContentLength(dwLength))
			throw HttpErrorQuery;
		m_nResponseCode = QueryStatusCode();
		if (m_nResponseCode == HTTP_STATUS_NOT_FOUND) {
			throw HttpError404;
		}
		if (!::WinHttpQueryDataAvailable(m_hRequest, &dwBytesToRead))
			throw HttpErrorQuery;
		if (dwLength > DOWNLOAD_BUFFER_SIZE)
			throw HttpErrorBuffer;//�ļ���С����Ԥ�����ֵ�����������ص��ڴ�
		lpFileMem = (BYTE*)malloc(dwLength);
		lpBuff = malloc(READ_BUFFER_SIZE);
		while (true)
		{
			if (dwBytesToRead > READ_BUFFER_SIZE)
			{
				free(lpBuff);
				lpBuff = malloc(dwBytesToRead);
			}
			if (!::WinHttpReadData(m_hRequest, lpBuff, dwBytesToRead, &dwReadSize))
				throw HttpErrorDownload;
			memcpy(lpFileMem + dwRecvSize, lpBuff, dwReadSize);
			dwRecvSize += dwReadSize;
			if (!::WinHttpQueryDataAvailable(m_hRequest, &dwBytesToRead))
				throw HttpErrorDownload;
			if (dwBytesToRead <= 0)
			{
				bResult = true;
				break;
			}
		}
	}
	catch (HttpInterfaceError error)
	{
		m_paramsData.errcode = error;
	}
	if (lpBuff)
		free(lpBuff);
	if (bResult)
	{
		*ppBuffer = lpFileMem;
		*nSize = dwRecvSize;
	}
	else
		free(lpFileMem);
	return bResult;
}

void CWinHttp::SetDownLoadCallBack(COMMONCALLBACKTYPE & callback, void * userData)
{
  download_callback_ = callback;
  user_data_ = userData;
}

void CWinHttp::SetDownLoadCallBack(COMMONCALLBACK & callback, void * userData)
{
  download_callback_ = callback;
  user_data_ = userData;
}

bool CWinHttp::DownLoad(LPCWSTR lpUrl)
{
  Release();
  if (!Init())
    return false;

  DWORD dwFileSize = 0, dwBytesToRead = 0, dwReadSize=0;

  if (!InitConnect(lpUrl, HttpGet))
    return false;
  if (!QueryContentLength(dwFileSize))
  {
    m_paramsData.errcode = HttpErrorQuery;
    return false;
  }

  m_nResponseCode = QueryStatusCode();
  if (m_nResponseCode == HTTP_STATUS_NOT_FOUND)
  {
    m_paramsData.errcode = HttpError404;
    return false;
  }

  //��ѯ�Ƿ�ɶ�
  if (!::WinHttpQueryDataAvailable(m_hRequest, &dwBytesToRead))
  {
    return false;
  }

  void* lpBuff = malloc(READ_BUFFER_SIZE);
  bool bSuc = false;
  while (true)
  {
    if (dwBytesToRead > READ_BUFFER_SIZE)
    {
      free(lpBuff);
      lpBuff = malloc(dwBytesToRead);
    }
    if (!::WinHttpReadData(m_hRequest, lpBuff, dwBytesToRead, &dwReadSize))
      break;
    
    //�������÷Ŵ���
    if (download_callback_)
    {
      if (!download_callback_(lpBuff, dwReadSize, dwFileSize, user_data_))
      {
        bSuc = true;
        break;
      }
    }

    if (!::WinHttpQueryDataAvailable(m_hRequest, &dwBytesToRead))
      break;
    if (dwBytesToRead <= 0)
    {
      bSuc = true;
      break;
    }
  }
  free(lpBuff);

  return true;
}

void CWinHttp::SetDownloadCallback(IHttpCallback* pCallback, void* pParam)
{
	m_paramsData.callback = pCallback;
	m_paramsData.lpparam = pParam;
}

void CWinHttp::AddHeader(LPCSTR key, LPCSTR value)
{
	if (isEmptyString(key) || isEmptyString(value)) {
		return;
	}
	m_header.addHeader(std::string(key), std::string(value));
}

string CWinHttp::Request(LPCSTR lpUrl, HttpRequest type, LPCSTR lpPostData /*= NULL*/, LPCSTR lpHeader/*=NULL*/)
{
	string strRet;
	wstring strUrl = A2U(string(lpUrl));
	if (!InitConnect(strUrl.c_str(), type, lpPostData, (lpHeader == NULL) ? NULL : A2U(string(lpHeader)).c_str()))
		return strRet;
	m_nResponseCode = QueryStatusCode();
	if (m_nResponseCode == HTTP_STATUS_NOT_FOUND) {
		throw HttpError404;
	}
	DWORD dwBytesToRead, dwReadSize;
	void* lpBuff = malloc(READ_BUFFER_SIZE);
	bool bFinish = false;
	while (true)
	{
		if (!::WinHttpQueryDataAvailable(m_hRequest, &dwBytesToRead))
			break;
		if (dwBytesToRead <= 0)
		{
			bFinish = true;
			break;
		}
		if (dwBytesToRead > READ_BUFFER_SIZE)
		{
			free(lpBuff);
			lpBuff = malloc(dwBytesToRead);
		}
		if (!::WinHttpReadData(m_hRequest, lpBuff, dwBytesToRead, &dwReadSize))
			break;
		strRet.append((const char*)lpBuff, dwReadSize);
	}
	free(lpBuff);
	if (!bFinish)
		strRet.clear();
	return strRet;
}

string CWinHttp::Request(LPCWSTR lpUrl, HttpRequest type, LPCSTR lpPostData /*= NULL*/, LPCWSTR lpHeader/*=NULL*/)
{
	string strRet;
	if (!InitConnect(lpUrl, type, lpPostData, lpHeader))
		return strRet;
	m_nResponseCode = QueryStatusCode();
	if (m_nResponseCode == HTTP_STATUS_NOT_FOUND) {
		throw HttpError404;
	}
	DWORD dwBytesToRead, dwReadSize;
	void* lpBuff = malloc(READ_BUFFER_SIZE);
	bool bFinish = false;

  if (type == HTTPHead)
  {
    DWORD dwSize = 0;
    LPVOID lpOutBuffer = NULL;
    BOOL  bResults = FALSE;
    WinHttpQueryHeaders(m_hRequest, WINHTTP_QUERY_RAW_HEADERS_CRLF,
      WINHTTP_HEADER_NAME_BY_INDEX, NULL,
      &dwSize, WINHTTP_NO_HEADER_INDEX);

    // Allocate memory for the buffer.
    if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
    {
      lpOutBuffer = new WCHAR[dwSize / sizeof(WCHAR)];

      // Now, use WinHttpQueryHeaders to retrieve the header.
      bResults = WinHttpQueryHeaders(m_hRequest,
        WINHTTP_QUERY_RAW_HEADERS_CRLF,
        WINHTTP_HEADER_NAME_BY_INDEX,
        lpOutBuffer, &dwSize,
        WINHTTP_NO_HEADER_INDEX);
     
      if (bResults)
      {
        //���ֽ�תխ�ֽ�
        std::wstring wstrHeaders = (wchar_t*)lpOutBuffer;
        
        strRet = U2A(wstrHeaders);
      }

      wchar_t* pBuffer = (wchar_t*)lpOutBuffer;
      delete []pBuffer;
    }

    return strRet;
  }
	while (true)
	{
		if (!::WinHttpQueryDataAvailable(m_hRequest, &dwBytesToRead))
			break;
		if (dwBytesToRead <= 0)
		{
			bFinish = true;
			break;
		}
		if (dwBytesToRead > READ_BUFFER_SIZE)
		{
			free(lpBuff);
			lpBuff = malloc(dwBytesToRead);
		}
		if (!::WinHttpReadData(m_hRequest, lpBuff, dwBytesToRead, &dwReadSize))
			break;
		strRet.append((const char*)lpBuff, dwReadSize);
	}
	free(lpBuff);
	if (!bFinish)
		strRet.clear();
	return strRet;
}

bool CWinHttp::QueryRawHeaders(OUT wstring& strHeaders)
{
	bool bRet = false;
	DWORD dwSize;
	BOOL bResult = ::WinHttpQueryHeaders(m_hRequest, WINHTTP_QUERY_RAW_HEADERS, WINHTTP_HEADER_NAME_BY_INDEX, NULL, &dwSize, WINHTTP_NO_HEADER_INDEX);
	if (ERROR_INSUFFICIENT_BUFFER == GetLastError())
	{
		wchar_t* lpData = (wchar_t*)malloc(dwSize);
		bResult = ::WinHttpQueryHeaders(m_hRequest, WINHTTP_QUERY_RAW_HEADERS, WINHTTP_HEADER_NAME_BY_INDEX, lpData, &dwSize, WINHTTP_NO_HEADER_INDEX);
		if (bResult)
		{
			strHeaders = lpData;
			bRet = true;
		}
		free(lpData);
	}
	return bRet;
}

bool CWinHttp::QueryContentLength(OUT DWORD& dwLength)
{
	bool bRet = false;
	wchar_t szBuffer[24] = { 0 };
	DWORD dwSize = 24 * sizeof(wchar_t);
	if (::WinHttpQueryHeaders(m_hRequest, WINHTTP_QUERY_CONTENT_LENGTH, WINHTTP_HEADER_NAME_BY_INDEX, szBuffer, &dwSize, WINHTTP_NO_HEADER_INDEX))
	{
		TCHAR *p = NULL;
		dwLength = wcstoul(szBuffer, &p, 10);
		bRet = true;
	}
	return bRet;
}

int CWinHttp::QueryStatusCode()
{
	int http_code = 0;
	wchar_t szBuffer[24] = { 0 };
	DWORD dwSize = 24 * sizeof(wchar_t);
	if (::WinHttpQueryHeaders(m_hRequest, WINHTTP_QUERY_STATUS_CODE, WINHTTP_HEADER_NAME_BY_INDEX, szBuffer, &dwSize, WINHTTP_NO_HEADER_INDEX)) {
		wchar_t *p = NULL;
		http_code = wcstoul(szBuffer, &p, 10);
	}
	return http_code;
}

bool CWinHttp::InitConnect(LPCWSTR lpUrl, HttpRequest type, LPCSTR lpPostData/*=NULL*/, LPCWSTR lpHeader/*=NULL*/)
{
	Release();
	if (!Init())
		return false;
	wstring strHostName, strPage;
	WORD wPort;
	MyParseUrlW(lpUrl, strHostName, strPage, wPort);
	if (wPort == INTERNET_DEFAULT_HTTPS_PORT)
		m_bHttps = true;
	if (!ConnectHttpServer(strHostName.c_str(), wPort))
	{
		m_paramsData.errcode = HttpErrorConnect;
		return false;
	}
	DWORD dwFlag = m_bHttps ? WINHTTP_FLAG_SECURE : 0;
	if (!CreateHttpRequest(strPage.c_str(), type, dwFlag))
	{
		m_paramsData.errcode = HttpErrorInit;
		return false;
	}
	if (m_bHttps)
	{
		DWORD dwFlags = SECURITY_FLAG_SECURE |
			SECURITY_FLAG_IGNORE_UNKNOWN_CA |
			SECURITY_FLAG_IGNORE_CERT_WRONG_USAGE |
			SECURITY_FLAG_IGNORE_CERT_CN_INVALID |
			SECURITY_FLAG_IGNORE_CERT_DATE_INVALID;
		WinHttpSetOption(m_hRequest, WINHTTP_OPTION_SECURITY_FLAGS, &dwFlags, sizeof(dwFlags));
	}
	if (!SendHttpRequest(lpPostData, lpHeader))
	{
		m_paramsData.errcode = HttpErrorSend;
		return false;
	}
	if (!WinHttpReceiveResponse(m_hRequest, NULL))
	{
		m_paramsData.errcode = HttpErrorInit;;
		return false;
	}
	return true;
}

bool CWinHttp::SendHttpRequest(LPCSTR lpPostData/*=NULL*/, LPCWSTR lpHeader/*=NULL*/)
{
	//���HTTPͷ
	std::wstring header = A2U(m_header.toHttpHeaders());
	::WinHttpAddRequestHeaders(m_hRequest, header.c_str(), header.size(), WINHTTP_ADDREQ_FLAG_ADD | WINHTTP_ADDREQ_FLAG_REPLACE);
	DWORD dwSize = (NULL == lpPostData) ? 0 : strlen(lpPostData);
	if (lpHeader == NULL)
		return ::WinHttpSendRequest(m_hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0, (LPVOID)lpPostData, dwSize, dwSize, NULL) == TRUE;
	return ::WinHttpSendRequest(m_hRequest, lpHeader, -1L, (LPVOID)lpPostData, dwSize, dwSize, NULL) == TRUE;
}

