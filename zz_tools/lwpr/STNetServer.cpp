#include "STNetServer.h"
#include <stdio.h>
#include <string.h>

namespace ZZTools
{
	STNetServer::STNetServer(const NET_SERVER_OPTION_T& opt)
		: m_bStoped(false),
		  m_fdListen(-1),
		  m_NetServerOption(opt),
		  m_nTotalClient(0)
	{
		assert(m_NetServerOption.nThreadPoolMin > 0);
		assert(m_NetServerOption.nThreadPoolMax > 0);
		assert(m_NetServerOption.nThreadPoolMaxIdle > 0);
		assert(m_NetServerOption.nConnectionMax >= 0);
		assert(m_NetServerOption.nSelectTimeout >= 0);
		assert(m_NetServerOption.nSocketExpiredTime >= 0);
		assert(m_NetServerOption.pHandler != NULL);

		m_NetServerOption.nConnectionMax = (m_NetServerOption.nConnectionMax == 0 || m_NetServerOption.nConnectionMax >= FD_SETSIZE)
		                                   ? (FD_SETSIZE - NET_SERVER_RESERVED_FDS) : m_NetServerOption.nConnectionMax;

		opt.pHandler->SetNetServer(this);
	}

	STNetServer::~STNetServer()
	{
	}

	void STNetServer::ActivateService()
	{
		// ���������˿�
		if(m_NetServerOption.nFdListen != -1)
		{
			m_fdListen = m_NetServerOption.nFdListen;
		}
		else
		{
			m_fdListen = Socket::CreateServerTCPSocket(m_NetServerOption.strAddr.c_str());
			if(m_fdListen == -1)
			{
				throw LWPR_EVENT_NET_CREATE_SERVER_SOCKET_ERROR(EXCEPTION_TRACE, "CreateServerTCPSocket error");
			}
		}

		// ��ȡ������ַ
		std::string strHost;
		int nPort = 0;
		if(!Socket::GetSocketAddress(m_fdListen, strHost, nPort))
		{
			throw LWPR_EVENT_NET_GETSOCKNAME_ERROR(EXCEPTION_TRACE, "GetSocketAddress error");
		}

		char bufTmp[64] = {0};
		if(strHost == "0.0.0.0")
		{
			std::string strTmp;
			if(!Socket::GetLocalHostIP(strTmp))
			{
				throw LWPR_EVENT_NET_GETLOCALIP_ERROR(EXCEPTION_TRACE, "GetLocalHostIP error");
			}

			sprintf(bufTmp, "%s:%d", strTmp.c_str(), nPort);
		}
		else
		{
			sprintf(bufTmp, "%s:%d", strHost.c_str(), nPort);
		}

		m_strListenAddr = bufTmp;
	}

	void STNetServer::Shutdown()
	{
		m_bStoped = true;
	}

	void STNetServer::Go()
	{
		SOCKET_SET_T setActiveFd;		// ��ǰ���л��SOCKET
		SOCKET_FD_T fdMax = m_fdListen;	// ���SOCKET

		// ��ʼ��
		FD_ZERO(&setActiveFd);
		FD_SET(m_fdListen, &setActiveFd);

		// �������
		while(!m_bStoped)
		{
			SOCKET_SET_T setReadableFd;
			memcpy(&setReadableFd, &setActiveFd, sizeof(setActiveFd));

			// Socket�����ﵽ���ֵ��ֹͣ���������ӣ�һ��Ϊ���������£��������ฺ�ؽ���Ľ���ִ�У�
			if(m_nTotalClient >= m_NetServerOption.nConnectionMax
				&& m_NetServerOption.bDropListenFD)
			{
				FD_CLR(m_fdListen, &setReadableFd);
			}

			struct timeval tv = {0};
			tv.tv_sec = m_NetServerOption.nSelectTimeout;
			tv.tv_usec = 0;
			struct timeval *ptv = &tv;
			ptv = (0 == tv.tv_sec) ? NULL : ptv;

			int nCode = select(fdMax + 1, &setReadableFd, NULL, NULL, ptv);
			// ��socket�ɶ�
			if(nCode > 0)
			{
				for(int i = 0; i <= fdMax; i++)
				{
					if(FD_ISSET(i, &setReadableFd))
					{
						// ����socket����Ծʱ��
						m_mapSocketTime[i] = time(NULL);

						// ������������
						if(i == m_fdListen)
						{
							while(Socket::IsSocketReadable(i, 0) == SOCKET_RET_OK)
							{
								SOCKET_FD_T fd = Socket::AcceptSocket(i);
								if(fd != -1)
								{
									// ������������ӳ������õ����������CloseSocket
									if(m_nTotalClient >= m_NetServerOption.nConnectionMax)
									{
										Socket::CloseSocket(fd);
									}
#ifndef WIN32
									// ��Unixƽ̨����ֹfd����������select�ɴ�����Χ��
									else if(fd >= FD_SETSIZE)
									{
										Socket::CloseSocket(fd);
									}
#endif
									else
									{
										FD_SET(fd, &setActiveFd);
										m_mapSocketTime[fd] = time(NULL);
										fdMax = (fd > fdMax) ? fd : fdMax;
										try
										{
											m_NetServerOption.pHandler->DoClientAmountUpdate(++m_nTotalClient, fd, true);
										}
										catch(...)
										{
										}
									}
								}
								else
								{
									break;
								}
							}
						}
						// �����ɶ�socket
						else
						{
							FD_CLR(i, &setActiveFd);

							SOCKET_RET_TYPE_E nRet = SOCKET_RET_OK;
							try
							{
								nRet = m_NetServerOption.pHandler->DoReceiveNormalData(i);
							}
							catch(...)
							{
								nRet = SOCKET_RET_FAILED;
							}

							switch(nRet)
							{
							case SOCKET_RET_FAILED:
							case SOCKET_RET_TIMEOUT:
								DestroySocket(i);
								break;
							case SOCKET_RET_OK:
								FD_SET(i, &setActiveFd);
								break;
							case SOCKET_RET_FREE:
								break;
							default:
								assert(0);
							}
						}
					}
				} // end of for
			}
			// select ��ʱ
			else if(nCode == 0)
			{
				// ��鲻��Ծ��socket
				for(int i = 0; i <= fdMax; i++)
				{
					if(FD_ISSET(i, &setActiveFd))
					{
						if(i != m_fdListen)
						{
							if((time(NULL) - m_mapSocketTime[i]) >= m_NetServerOption.nSocketExpiredTime)
							{
								try
								{
									m_NetServerOption.pHandler->DoCloseExpiredSocket(i);
								}
								catch(...)
								{
								}

								DestroySocket(i);
								FD_CLR(i, &setActiveFd);
							}
						}
					}
				}

				// �����û���ҵ����
				try
				{
					m_NetServerOption.pHandler->DoSelectTimeout();
				}
				catch(...)
				{
				}
			}
		}

		// �˳�����
		for(int i = 0; i <= fdMax; i++)
		{
			if(FD_ISSET(i, &setActiveFd))
			{
				DestroySocket(i);
			}
		}
	}

	void STNetServer::EnableSocket(SOCKET_FD_T fd)
	{
		assert(fd >= 0);
	}

	bool STNetServer::IsServiceBusy()
	{
		return true;
	}

	std::string STNetServer::GetListenAddr()
	{
		return m_strListenAddr;
	}

	void STNetServer::DestroySocket(SOCKET_FD_T fd)
	{
		Socket::CloseSocket(fd);
		m_NetServerOption.pHandler->DoClientAmountUpdate(--m_nTotalClient, fd, false);
	}

	void STNetServer::WriteSocketPositive(SOCKET_FD_T fd, char *buf, int length)
	{
	}

	void STNetServer::WriteSocketPositive(const SOCKETFDList& fds, char *buf, int length)
	{

	}

	INT32 STNetServer::ClientTotal()
	{
		return m_nTotalClient;
	}
}
