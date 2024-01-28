#include "HttpClient.h"

HttpClient::HttpClient()
	: m_bDebug(false)
{

}

HttpClient::~HttpClient(void)
{

}

static int OnDebug(CURL *, curl_infotype itype, char * pData, size_t size, void *)
{
	if(itype == CURLINFO_TEXT)
	{
		printf("[TEXT]%s\n", pData);
	}
	else if(itype == CURLINFO_HEADER_IN)
	{
		printf("[HEADER_IN]%s\n", pData);
	}
	else if(itype == CURLINFO_HEADER_OUT)
	{
		printf("[HEADER_OUT]%s\n", pData);
	}
	else if(itype == CURLINFO_DATA_IN)
	{
		printf("[DATA_IN]%s\n", pData);
	}
	else if(itype == CURLINFO_DATA_OUT)
	{
		printf("[DATA_OUT]%s\n", pData);
	}
	return 0;
}

static size_t OnWriteData(void* buffer, size_t size, size_t nmemb, void* lpVoid)
{
	std::string* str = dynamic_cast<std::string*>((std::string *)lpVoid);
	if( NULL == str || NULL == buffer )
	{
		return -1;
	}

	char* pData = (char*)buffer;
	str->append(pData, size * nmemb);
	return nmemb;
}

bool HttpClient::Post(const string& strUrl, const string& strPost, int connTimeout, int readTimeout, string& strResponse)
{
	CURLcode res;
	CURL* curl = curl_easy_init();
	if(NULL == curl)
	{
		return CURLE_FAILED_INIT;
	}
	if(m_bDebug)
	{
		curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
		curl_easy_setopt(curl, CURLOPT_DEBUGFUNCTION, OnDebug);
	}
	curl_easy_setopt(curl, CURLOPT_URL, strUrl.c_str());
	curl_easy_setopt(curl, CURLOPT_POST, 1);
	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, strPost.c_str());
	curl_easy_setopt(curl, CURLOPT_READFUNCTION, NULL);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, OnWriteData);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&strResponse);
	curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
	curl_easy_setopt(curl, CURLOPT_FORBID_REUSE, 1);
	curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, connTimeout > 0 ? connTimeout : 5);
	curl_easy_setopt(curl, CURLOPT_TIMEOUT, readTimeout > 0 ? readTimeout : 5);
	
	res = curl_easy_perform(curl);
	curl_easy_cleanup(curl);

	return res == CURLE_OK;
}

bool HttpClient::Post(const string& strUrl, const map<string, string>& headers, const string& strPost, int connTimeout, int readTimeout, string& strResponse)
{
	CURLcode res;

	struct curl_slist *list = NULL;
	CURL* curl = curl_easy_init();
	if(NULL == curl)
	{
		return CURLE_FAILED_INIT;
	}
	if(m_bDebug)
	{
		curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
		curl_easy_setopt(curl, CURLOPT_DEBUGFUNCTION, OnDebug);
	}
	curl_easy_setopt(curl, CURLOPT_URL, strUrl.c_str());
	if (!headers.empty())
	{
		list = assembleHeaders(headers);
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, list);
	}
	curl_easy_setopt(curl, CURLOPT_POST, 1);
	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, strPost.c_str());
	curl_easy_setopt(curl, CURLOPT_READFUNCTION, NULL);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, OnWriteData);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&strResponse);
	curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
	curl_easy_setopt(curl, CURLOPT_FORBID_REUSE, 1);
	curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, connTimeout > 0 ? connTimeout : 5);
	curl_easy_setopt(curl, CURLOPT_TIMEOUT, readTimeout > 0 ? readTimeout : 5);
	
	res = curl_easy_perform(curl);
	curl_easy_cleanup(curl);

	if (list != NULL)
	{
		curl_slist_free_all(list);
	}

	return res == CURLE_OK;
}

bool HttpClient::Get(const string & strUrl, int connTimeout, int readTimeout, string & strResponse)
{
	CURLcode res;
	CURL* curl = curl_easy_init();
	if(NULL == curl)
	{
		return CURLE_FAILED_INIT;
	}
	if(m_bDebug)
	{
		curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
		curl_easy_setopt(curl, CURLOPT_DEBUGFUNCTION, OnDebug);
	}
	curl_easy_setopt(curl, CURLOPT_URL, strUrl.c_str());
	curl_easy_setopt(curl, CURLOPT_READFUNCTION, NULL);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, OnWriteData);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&strResponse);
	/**
	* 当多个线程都使用超时处理的时候，同时主线程中有sleep或是wait等操作。
	* 如果不设置这个选项，libcurl将会发信号打断这个wait从而导致程序退出。
	*/
	curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
	curl_easy_setopt(curl, CURLOPT_FORBID_REUSE, 1);
	curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, connTimeout > 0 ? connTimeout : 5);
	curl_easy_setopt(curl, CURLOPT_TIMEOUT, readTimeout > 0 ? readTimeout : 5);

	res = curl_easy_perform(curl);
	curl_easy_cleanup(curl);

	return res == CURLE_OK;
}

bool HttpClient::Get(const string & strUrl, const map<string, string>& headers, int connTimeout, int readTimeout, string & strResponse)
{
	CURLcode res;

	struct curl_slist *list = NULL;
	CURL* curl = curl_easy_init();
	if(NULL == curl)
	{
		return CURLE_FAILED_INIT;
	}
	if(m_bDebug)
	{
		curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
		curl_easy_setopt(curl, CURLOPT_DEBUGFUNCTION, OnDebug);
	}
	curl_easy_setopt(curl, CURLOPT_URL, strUrl.c_str());
	if (!headers.empty())
	{
		list = assembleHeaders(headers);
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, list);
	}
	curl_easy_setopt(curl, CURLOPT_READFUNCTION, NULL);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, OnWriteData);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&strResponse);
	/**
	* 当多个线程都使用超时处理的时候，同时主线程中有sleep或是wait等操作。
	* 如果不设置这个选项，libcurl将会发信号打断这个wait从而导致程序退出。
	*/
	curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
	curl_easy_setopt(curl, CURLOPT_FORBID_REUSE, 1);
	curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, connTimeout > 0 ? connTimeout : 5);
	curl_easy_setopt(curl, CURLOPT_TIMEOUT, readTimeout > 0 ? readTimeout : 5);

	res = curl_easy_perform(curl);
	curl_easy_cleanup(curl);

	if (list != NULL)
	{
		curl_slist_free_all(list);
	}

	return res == CURLE_OK;
}

bool HttpClient::PostSSL(const string& strUrl, const string& strPost, const char* pCaPath, int connTimeout, int readTimeout, string& strResponse)
{
	CURLcode res;
	CURL* curl = curl_easy_init();
	if(NULL == curl)
	{
		return CURLE_FAILED_INIT;
	}
	if(m_bDebug)
	{
		curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
		curl_easy_setopt(curl, CURLOPT_DEBUGFUNCTION, OnDebug);
	}
	curl_easy_setopt(curl, CURLOPT_URL, strUrl.c_str());
	curl_easy_setopt(curl, CURLOPT_POST, 1);
	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, strPost.c_str());
	curl_easy_setopt(curl, CURLOPT_READFUNCTION, NULL);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, OnWriteData);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&strResponse);
	curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
	if(NULL == pCaPath)
	{
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, false);
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, false);
	}
	else
	{
		//缺省情况就是PEM，所以无需设置，另外支持DER
		//curl_easy_setopt(curl,CURLOPT_SSLCERTTYPE,"PEM");
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, true);
		curl_easy_setopt(curl, CURLOPT_CAINFO, pCaPath);
	}
	curl_easy_setopt(curl, CURLOPT_FORBID_REUSE, 1);
	curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, connTimeout > 0 ? connTimeout : 5);
	curl_easy_setopt(curl, CURLOPT_TIMEOUT, readTimeout > 0 ? readTimeout : 5);
	
	res = curl_easy_perform(curl);
	curl_easy_cleanup(curl);
	
	return res == CURLE_OK;
}

bool HttpClient::PostSSL(const string& strUrl, const map<string, string>& headers, const string& strPost, const char* pCaPath, int connTimeout, int readTimeout, string& strResponse)
{
	CURLcode res;

	struct curl_slist *list = NULL;
	CURL* curl = curl_easy_init();
	if(NULL == curl)
	{
		return CURLE_FAILED_INIT;
	}
	if(m_bDebug)
	{
		curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
		curl_easy_setopt(curl, CURLOPT_DEBUGFUNCTION, OnDebug);
	}
	curl_easy_setopt(curl, CURLOPT_URL, strUrl.c_str());
	if (!headers.empty())
	{
		list = assembleHeaders(headers);
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, list);
	}
	curl_easy_setopt(curl, CURLOPT_POST, 1);
	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, strPost.c_str());
	curl_easy_setopt(curl, CURLOPT_READFUNCTION, NULL);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, OnWriteData);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&strResponse);
	curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
	if(NULL == pCaPath)
	{
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, false);
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, false);
	}
	else
	{
		//缺省情况就是PEM，所以无需设置，另外支持DER
		//curl_easy_setopt(curl,CURLOPT_SSLCERTTYPE,"PEM");
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, true);
		curl_easy_setopt(curl, CURLOPT_CAINFO, pCaPath);
	}
	curl_easy_setopt(curl, CURLOPT_FORBID_REUSE, 1);
	curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, connTimeout > 0 ? connTimeout : 5);
	curl_easy_setopt(curl, CURLOPT_TIMEOUT, readTimeout > 0 ? readTimeout : 5);
	
	res = curl_easy_perform(curl);
	curl_easy_cleanup(curl);

	if (list != NULL)
	{
		curl_slist_free_all(list);
	}

	return res == CURLE_OK;
}

bool HttpClient::GetSSL(const string& strUrl, const char* pCaPath, int connTimeout, int readTimeout, string& strResponse)
{
	CURLcode res;
	CURL* curl = curl_easy_init();
	if(NULL == curl)
	{
		return CURLE_FAILED_INIT;
	}
	if(m_bDebug)
	{
		curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
		curl_easy_setopt(curl, CURLOPT_DEBUGFUNCTION, OnDebug);
	}
	curl_easy_setopt(curl, CURLOPT_URL, strUrl.c_str());
	curl_easy_setopt(curl, CURLOPT_READFUNCTION, NULL);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, OnWriteData);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&strResponse);
	curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
	if(NULL == pCaPath)
	{
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, false);
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, false);
	}
	else
	{
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, true);
		curl_easy_setopt(curl, CURLOPT_CAINFO, pCaPath);
	}
	curl_easy_setopt(curl, CURLOPT_FORBID_REUSE, 1);
	curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, connTimeout > 0 ? connTimeout : 5);
	curl_easy_setopt(curl, CURLOPT_TIMEOUT, readTimeout > 0 ? readTimeout : 5);
	
	res = curl_easy_perform(curl);
	curl_easy_cleanup(curl);
	
	return res == CURLE_OK;
}

bool HttpClient::GetSSL(const string& strUrl, const map<string, string>& headers, const char* pCaPath, int connTimeout, int readTimeout, string& strResponse)
{
	CURLcode res;

	struct curl_slist *list = NULL;
	CURL* curl = curl_easy_init();
	if(NULL == curl)
	{
		return CURLE_FAILED_INIT;
	}
	if(m_bDebug)
	{
		curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
		curl_easy_setopt(curl, CURLOPT_DEBUGFUNCTION, OnDebug);
	}
	curl_easy_setopt(curl, CURLOPT_URL, strUrl.c_str());
	if (!headers.empty())
	{
		list = assembleHeaders(headers);
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, list);
	}
	curl_easy_setopt(curl, CURLOPT_READFUNCTION, NULL);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, OnWriteData);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&strResponse);
	curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
	if(NULL == pCaPath)
	{
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, false);
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, false);
	}
	else
	{
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, true);
		curl_easy_setopt(curl, CURLOPT_CAINFO, pCaPath);
	}
	curl_easy_setopt(curl, CURLOPT_FORBID_REUSE, 1);
	curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, connTimeout > 0 ? connTimeout : 5);
	curl_easy_setopt(curl, CURLOPT_TIMEOUT, readTimeout > 0 ? readTimeout : 5);
	
	res = curl_easy_perform(curl);
	curl_easy_cleanup(curl);

	if (list != NULL)
	{
		curl_slist_free_all(list);
	}
	
	return res == CURLE_OK;
}

bool HttpClient::GetLocalIP(const std::string & strUrl, std::string & strIP)
{
	bool ret = false;
	CURLcode res;
	CURL* curl = curl_easy_init();
	if(NULL == curl)
	{
		return ret;
	}
	if(m_bDebug)
	{
		curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
		curl_easy_setopt(curl, CURLOPT_DEBUGFUNCTION, OnDebug);
	}
	curl_easy_setopt(curl, CURLOPT_URL, strUrl.c_str());
	curl_easy_setopt(curl, CURLOPT_READFUNCTION, NULL);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, NULL);
	/**
	* 当多个线程都使用超时处理的时候，同时主线程中有sleep或是wait等操作。
	* 如果不设置这个选项，libcurl将会发信号打断这个wait从而导致程序退出。
	*/
	curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
	curl_easy_setopt(curl, CURLOPT_FORBID_REUSE, 1);
	curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 5);
	curl_easy_setopt(curl, CURLOPT_TIMEOUT, 5);
	
	char* ip;
	
	if (curl_easy_perform(curl) == CURLE_OK) {
		res = curl_easy_getinfo(curl, CURLINFO_LOCAL_IP, &ip);
		
		ret = res == CURLE_OK;
		if (ret) {
			strIP.append(ip);
		}

		curl_easy_cleanup(curl);
	}
	
	return ret;
}

void HttpClient::SetDebug(bool bDebug)
{
	m_bDebug = bDebug;
}

struct curl_slist* HttpClient::assembleHeaders(const map<string, string>& headers)
{
    struct curl_slist *list = NULL;

	map<string, string>::const_iterator it = headers.begin();
	map<string, string>::const_iterator end = headers.end();
	for (; it != end; it++)
	{
		const string& key = it->first;
		const string& val = it->second;
		string attr = key + ": " + val;
		list = curl_slist_append(list, attr.c_str());
	}

	return list;
}
