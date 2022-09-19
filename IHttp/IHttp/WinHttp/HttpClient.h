#pragma once
#include "../httpHeader.h"

class CWinHttp
	: public IWinHttp
{
public:
	CWinHttp(void);
	virtual ~CWinHttp(void);
	
	virtual void	SetTimeOut(int dwConnectTime,  int dwSendTime, int dwRecvTime);
	virtual string	Request(LPCSTR lpUrl, HttpRequest type, LPCSTR lpPostData = NULL, LPCSTR lpHeader=NULL);
	virtual string	Request(LPCWSTR lpUrl, HttpRequest type, LPCSTR lpPostData = NULL, LPCWSTR lpHeader=NULL);
	virtual void	FreeInstance()													{ delete this;		}
	virtual bool	DownloadFile(LPCWSTR lpUrl, LPCWSTR lpFilePath);
	virtual bool	DownloadToMem(LPCWSTR lpUrl, OUT void** ppBuffer, OUT int* nSize);

  //�������ݽ��� ���÷�
  virtual void SetDownLoadCallBack(COMMONCALLBACKTYPE& callback,void* userData);
  virtual void SetDownLoadCallBack(COMMONCALLBACK& callback, void* userData);
  virtual bool  DownLoad(LPCWSTR lpUrl);

	virtual void	SetDownloadCallback(IHttpCallback* pCallback, void* pParam);
	virtual HttpInterfaceError GetErrorCode() { return m_paramsData.errcode; }
	virtual void AddHeader(LPCSTR key, LPCSTR value);
	virtual int GetResponseCode() { return m_nResponseCode; }

protected:
	bool	Init();
	void	Release();
	//init
	bool	InitConnect(LPCWSTR lpUrl, HttpRequest type, LPCSTR lpPostData=NULL, LPCWSTR lpHeader=NULL);
	bool	ConnectHttpServer(LPCWSTR lpIP, WORD wPort);
	bool	CreateHttpRequest(LPCWSTR lpPage, HttpRequest type, DWORD dwFlag=0);
	bool	SendHttpRequest(LPCSTR lpPostData=NULL, LPCWSTR lpHeader=NULL);
	//query 
	bool	QueryRawHeaders(OUT wstring& strHeaders);
	bool	QueryContentLength(OUT DWORD& dwLength);
	int  QueryStatusCode();

private:
	int m_nResponseCode;
	bool		m_bHttps;
	HINTERNET	m_hInternet;
	HINTERNET	m_hConnect;
	HINTERNET	m_hRequest;
	int			m_nConnTimeout;
	int			m_nSendTimeout;
	int			m_nRecvTimeout;
	CHttpHeader m_header;
	HttpParamsData m_paramsData;

  COMMONCALLBACK download_callback_;
  void* user_data_;
};

