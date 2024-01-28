#pragma once

#include <string>
#include <map>

#include "curl/curl.h"

using namespace std;

class HttpClient
{
public:
	HttpClient();
	~HttpClient();

public:
    bool Post(const string& strUrl, const string& strPost, int connTimeout, int readTimeout, string& strResponse);
	bool Post(const string& strUrl, const map<string, string>& headers, const string& strPost, int connTimeout, int readTimeout, string& strResponse);

    bool Get(const string& strUrl, int connTimeout, int readTimeout, string& strResponse);
	bool Get(const string& strUrl, const map<string, string>& headers, int connTimeout, int readTimeout, string& strResponse);

    bool PostSSL(const string& strUrl, const string& strPost, const char* pCaPath, int connTimeout, int readTimeout, string& strResponse);
	bool PostSSL(const string& strUrl, const map<string, string>& headers, const string& strPost, const char* pCaPath, int connTimeout, int readTimeout, string& strResponse);

    bool GetSSL(const string& strUrl, const char* pCaPath, int connTimeout, int readTimeout, string& strResponse);
	bool GetSSL(const string& strUrl, const map<string, string>& headers, const char* pCaPath, int connTimeout, int readTimeout, string& strResponse);
	
	bool GetLocalIP(const string& strUrl, string& strIP);
	
public:
	void SetDebug(bool bDebug);
	struct curl_slist* assembleHeaders(const map<string, string>& headers);

private:
	bool m_bDebug;
	
};
