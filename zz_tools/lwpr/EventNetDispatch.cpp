#include "EventNetDispatch.h"
#include "Synchronized.h"
#include "Utility.h"
#include <assert.h>

namespace ZZTools
{
	//----------------------------------------------------------------------------------
	// class EventNetHandler
	//----------------------------------------------------------------------------------
	SOCKET_RET_TYPE_E EventNetHandler::DoReceiveNormalData(SOCKET_FD_T fd)
	{
		assert(fd >= 0);

		return SOCKET_RET_OK;
	}

	SOCKET_RET_TYPE_E EventNetHandler::DoReceiveConnection(SOCKET_FD_T fd)
	{
		assert(fd >= 0);

		return SOCKET_RET_OK;
	}

	LWPR::SOCKET_RET_TYPE_E EventNetHandler::DoCloseExpiredSocket(SOCKET_FD_T fd)
	{
		assert(fd >= 0);

		return SOCKET_RET_OK;
	}

	//----------------------------------------------------------------------------------
	// struct event_net_option
	//----------------------------------------------------------------------------------
	event_net_option::event_net_option()
		: strIP("0.0.0.0"),
		  nMinPort(0),
		  nMaxPort(0),
		  nThreadPoolMin(5),
		  nThreadPoolMax(300),
		  nThreadPoolMaxIdle(100),
		  nHousekeepInterval(60 * 10),
		  nSocketExpiredTime(60 * 20),
		  bAllowDoHousekeep(false),
		  bAllowDoCloseHandler(false),
		  bAllowDoRcvConnHandler(false),
		  pHandler(NULL)
	{
	}

	//----------------------------------------------------------------------------------
	// class EventNetDispatch
	//----------------------------------------------------------------------------------
	EventNetDispatch::EventNetDispatch(EVENT_NET_OPTION_T& opt)
		: m_EventNetOption(opt),
		  m_nIdleThread(0),
		  m_nTotalThread(0)
	{
		if(m_EventNetOption.nMaxPort == 0)
		{
			m_EventNetOption.nMaxPort = m_EventNetOption.nMinPort;
		}

		assert(m_EventNetOption.nMinPort >= 0);
		assert(m_EventNetOption.nMaxPort >= 0);
		assert(m_EventNetOption.nThreadPoolMin > 0);
		assert(m_EventNetOption.nThreadPoolMax > 0);
		assert(m_EventNetOption.nThreadPoolMaxIdle > 0);
		assert(m_EventNetOption.nHousekeepInterval > 0);
		assert(m_EventNetOption.nSocketExpiredTime > 0);
		assert(m_EventNetOption.pHandler != NULL);

		FD_ZERO(&m_setActiveFd);
		m_listReadableSocket.reserve(FD_SETSIZE + 1);

		opt.pHandler->SetEventNetDispatch(this);
	}

	EventNetDispatch::~EventNetDispatch()
	{
		if(m_EventNetOption.pHandler != NULL)
		{
			delete m_EventNetOption.pHandler;
		}
	}

	void EventNetDispatch::Activate()
	{
		// ���������˿�
		m_fdListen = Socket::CreateServerTCPSocket(m_EventNetOption.nMinPort, m_EventNetOption.nMaxPort, m_EventNetOption.strIP.c_str());
		if(m_fdListen == -1)
		{
			throw LWPR_EVENT_NET_CREATE_SERVER_SOCKET_ERROR(EXCEPTION_TRACE, "CreateServerTCPSocket error");
		}

		// ��������������Server��socket�����˿ڽ�����client�˵�connect�����ͻ�˳�������·���֣������أ�
		std::string strSelfIP = (m_EventNetOption.strIP == "0.0.0.0" ? "127.0.0.1" : m_EventNetOption.strIP);
		m_fdConnSelfClient = Socket::ConnectRemoteHost(strSelfIP.c_str(), m_EventNetOption.nMinPort);
		if(m_fdConnSelfClient == -1)
		{
			throw LWPR_EVENT_NET_CONNECT_SELEF_ERROR(EXCEPTION_TRACE, "ConnectRemoteHost error");
		}

		// ��������
		m_fdConnSelfServer = Socket::AcceptSocket(m_fdListen);
		if(m_fdConnSelfClient == -1)
		{
			throw LWPR_EVENT_NET_CONNECT_SELEF_ERROR(EXCEPTION_TRACE, "AcceptSocket error");
		}

		// У������Socket
		// ͨ��m_fdConnSelfClientд�뱾���̺ţ���ͨ��m_fdConnSelfServer���������δ�ܳɹ�������
		// ���߶����Ľ��̺���д��Ĳ�ͬ������Ϊm_fdConnSelfClient��m_fdConnSelfServer������
		PID_T nPid = Utility::GetPid();
		SOCKET_RET_TYPE_E nRet = Socket::WriteSocket(m_fdConnSelfClient, (const char*)&nPid, sizeof(nPid));
		if(nRet != SOCKET_RET_OK)
		{
			throw LWPR_EVENT_NET_CONNECT_SELEF_ERROR(EXCEPTION_TRACE, "WriteSocket error");
		}

		Buffer buf;
		nRet = Socket::ReadSocket(m_fdConnSelfServer, buf, sizeof(nPid), 0);
		if(nRet != SOCKET_RET_OK)
		{
			throw LWPR_EVENT_NET_CONNECT_SELEF_ERROR(EXCEPTION_TRACE, "ReadSocket error");
		}

		PID_T* pPid = (PID_T*)buf.Inout();
		if(*pPid != nPid)
		{
			throw LWPR_EVENT_NET_CONNECT_SELEF_ERROR(EXCEPTION_TRACE, "m_fdConnSelfServer accept other process request");
		}

		// ȡ���socket
		m_fdMax = (m_fdConnSelfClient > m_fdListen) ? m_fdConnSelfClient : m_fdListen;

		// ����m_fdActive
		FD_SET(m_fdListen, &m_setActiveFd);
		FD_SET(m_fdConnSelfServer, &m_setActiveFd);

		// �����̶߳��󣬲������߳�
		for(int i = 0; i < m_EventNetOption.nThreadPoolMin; i++)
		{
			EventNetWorkThread* pThread = new EventNetWorkThread(*this);
			pThread->Start();
			m_listThread.push_back(pThread);
		}

		// ���������߳�
		this->Start();
	}

	void EventNetDispatch::Halt()
	{
		SOCKET_SELF_MSG_T msg;
		msg.nMsgType = SOCKET_SELF_MSG_EXIT_THREAD;
		msg.nSocketFd = 0;

		{
			Synchronized syn(m_MutexClient);
			Socket::WriteSocket(m_fdConnSelfClient, (const char*)&msg, sizeof(msg));
		}

		Socket::CloseSocket(m_fdListen);
		this->StopRunning();
	}

	void EventNetDispatch::Run()
	{
		while(IsContinue())
		{
			// �Ժ���滻��wait
			Thread::Sleep(3);

			// ������ȥ���߳�
			ClearDeadThread();

			// ��̬�����̳߳�
			if(IsServiceBusy())
			{
				if(m_nTotalThread < m_EventNetOption.nThreadPoolMax)
				{
					EventNetWorkThread* pThread = new EventNetWorkThread(*this);
					pThread->Start();
					m_listThread.push_back(pThread);
				}
			}
			else
			{
				if(m_nIdleThread > m_EventNetOption.nThreadPoolMaxIdle)
				{
					EventNetWorkThread* pThread = m_listThread.front();
					if(pThread != NULL)
					{
						pThread->StopRunning();
					}
				}
			}
		}

		// ���̳߳����߳��˳�
		std::list< EventNetWorkThread* >::iterator it = m_listThread.begin();
		for(; it != m_listThread.end(); it++)
		{
			(*it)->StopRunning();
		}

		// �����߳���Դ
		for(int i = 0; i < 5 && m_listThread.size() > 0; i++)
		{
			ClearDeadThread();
			Thread::Sleep(1);
		}

		Socket::CloseSocket(m_fdConnSelfServer);
		Socket::CloseSocket(m_fdConnSelfClient);
	}

	void EventNetDispatch::MakeSocketDispatching(SOCKET_FD_T fd)
	{
		assert(fd >= 0);

		SOCKET_SELF_MSG_T msg;
		msg.nMsgType = SOCKET_SELF_MSG_ADD_FD;
		msg.nSocketFd = fd;

		Synchronized syn(m_MutexClient);
		LWPR::SOCKET_RET_TYPE_E nRet = Socket::WriteSocket(m_fdConnSelfClient, (const char*)&msg, sizeof(msg));
		if(nRet != LWPR::SOCKET_RET_OK)
		{
			// �źŷ���ʱ����ִ�е�����
			LWPR::Socket::CloseSocket(fd);
		}
	}

	bool EventNetDispatch::IsServiceBusy()
	{
		// ��Ϊ�̳߳صĸ�������
		bool bResult = (m_nIdleThread >= 3);
		return !bResult;
	}

	void EventNetDispatch::ClearDeadThread()
	{
		std::list< EventNetWorkThread* >::iterator it = m_listThread.begin();
		for(; it != m_listThread.end();)
		{
			if(!(*it)->IsContinue() && (*it)->IsExited())
			{
				delete(*it);
				m_listThread.erase(it);
			}
			else
			{
				it++;
			}
		}
	}

	//----------------------------------------------------------------------------------
	// class EventNetThreadImpl
	//----------------------------------------------------------------------------------
	EventNetWorkThread::EventNetWorkThread(EventNetDispatch& dispatch)
		: m_EventNetDispatch(dispatch)
	{
	}

	EventNetWorkThread::~EventNetWorkThread()
	{
	}

	void EventNetWorkThread::Run()
	{
		AutomaticCount autoTotalThread(m_EventNetDispatch.m_nTotalThread);

		while(IsContinue())
		{
			try
			{
				// �ɶ�socket
				SOCKET_FD_T nReadySocket = -1;

				{
					// �����̼߳���
					AutomaticCount autoIdleThread(m_EventNetDispatch.m_nIdleThread);

					// ��������ȡ�ɶ���Socket�������ٽ�����
					Synchronized syn(m_EventNetDispatch.m_MutexEvent);

					// ��һ���ɶ�socket����ʼ����
					if(m_EventNetDispatch.m_listReadableSocket.size() > 0)
					{
						nReadySocket = m_EventNetDispatch.m_listReadableSocket.back();
						m_EventNetDispatch.m_listReadableSocket.pop_back();
					}

					// ���û�пɶ�socket����select
					while(-1 == nReadySocket)
					{
						// �жϵ�ǰ�߳��Ƿ���Լ���ִ��
						if(!m_EventNetDispatch.IsContinue())
						{
							return;
						}

						fd_set setReadableFd;
						memcpy(&setReadableFd, &m_EventNetDispatch.m_setActiveFd, sizeof(m_EventNetDispatch.m_setActiveFd));
						struct timeval tv = {0};
						tv.tv_sec = m_EventNetDispatch.m_EventNetOption.nHousekeepInterval;
						tv.tv_usec = 0;
						struct timeval *ptv = &tv;

						if(!m_EventNetDispatch.m_EventNetOption.bAllowDoHousekeep)
						{
							ptv = NULL;
						}

						int nCode = select(m_EventNetDispatch.m_fdMax + 1, &setReadableFd, NULL, NULL, ptv);
						// ��socket�ɶ�
						if(nCode > 0)
						{
							for(int i = 0; i <= m_EventNetDispatch.m_fdMax; i++)
							{
								if(FD_ISSET(i, &setReadableFd))
								{
									// ����socket����Ծʱ��
									m_EventNetDispatch.m_mapActiveFd[i] = time(NULL);

									// ������������
									if(i == m_EventNetDispatch.m_fdListen)
									{
										SOCKET_FD_T fd = LWPR::Socket::AcceptSocket(i);
										if(fd != -1)
										{
											// ������������ӳ���select֧�ֵ����������CloseSocket
											if(fd >= FD_SETSIZE)
											{
												Socket::CloseSocket(fd);
											}
											else
											{
												FD_SET(fd, &m_EventNetDispatch.m_setActiveFd);
												m_EventNetDispatch.m_mapActiveFd[fd] = time(NULL);
												m_EventNetDispatch.m_fdMax = (fd > m_EventNetDispatch.m_fdMax) ? fd : m_EventNetDispatch.m_fdMax;
												if(m_EventNetDispatch.m_EventNetOption.bAllowDoRcvConnHandler)
												{
													try
													{
														m_EventNetDispatch.m_EventNetOption.pHandler->DoReceiveConnection(fd);
													}
													catch(...)
													{
													}
												}
											}
										}
									}
									// �����������ӣ������ڲ�������Ϣ�Ĵ���
									else if(i == m_EventNetDispatch.m_fdConnSelfServer)
									{
										LWPR::Buffer buf;
										SOCKET_RET_TYPE_E res = Socket::ReadSocket(i, buf, sizeof(SOCKET_SELF_MSG_T));
										if(res == SOCKET_RET_OK)
										{
											SOCKET_SELF_MSG_T* pMsg = (SOCKET_SELF_MSG_T*)buf.Inout();

											switch(pMsg->nMsgType)
											{
											case SOCKET_SELF_MSG_ADD_FD:
												FD_SET(pMsg->nSocketFd, &m_EventNetDispatch.m_setActiveFd);
												m_EventNetDispatch.m_mapActiveFd[pMsg->nSocketFd] = time(NULL);
												break;
											case SOCKET_SELF_MSG_EXIT_THREAD:
												break;
											default:
												assert(0);
											}
										}
									}
									// ������һ���ɶ�socket
									else if(-1 == nReadySocket)
									{
										nReadySocket = i;
										FD_CLR(i, &m_EventNetDispatch.m_setActiveFd);
									}
									// ��������ɶ�socket
									else
									{
										m_EventNetDispatch.m_listReadableSocket.push_back(i);
										FD_CLR(i, &m_EventNetDispatch.m_setActiveFd);
									}
								}
							}
						}
						// select ��ʱ
						else if(nCode == 0)
						{
							// ��鲻��Ծ��socket
							if(m_EventNetDispatch.m_EventNetOption.bAllowDoHousekeep)
							{
								for(int i = 0; i <= m_EventNetDispatch.m_fdMax; i++)
								{
									if(FD_ISSET(i, &m_EventNetDispatch.m_setActiveFd))
									{
										if(i == m_EventNetDispatch.m_fdListen)
										{
										}
										else if(i == m_EventNetDispatch.m_fdConnSelfServer)
										{
										}
										else
										{
											if((time(NULL) - m_EventNetDispatch.m_mapActiveFd[i]) >= m_EventNetDispatch.m_EventNetOption.nSocketExpiredTime)
											{
												if(m_EventNetDispatch.m_EventNetOption.bAllowDoCloseHandler)
												{
													try
													{
														m_EventNetDispatch.m_EventNetOption.pHandler->DoCloseExpiredSocket(i);
													}
													catch(...)
													{
													}
												}

												Socket::CloseSocket(i);
												FD_CLR(i, &m_EventNetDispatch.m_setActiveFd);
											}
										}
									}
								}
							}
						}
					} // end of while(-1 == nReadableSocket)
				} // end of Lock

				// ׼�������ҵ�������
				SOCKET_RET_TYPE_E nRet = SOCKET_RET_OK;
				try
				{
					nRet = m_EventNetDispatch.m_EventNetOption.pHandler->DoReceiveNormalData(nReadySocket);
				}
				catch(...)
				{
					nRet = SOCKET_RET_FAILED;
				}

				switch(nRet)
				{
				case SOCKET_RET_FAILED:
				case SOCKET_RET_TIMEOUT:
					Socket::CloseSocket(nReadySocket);
					break;
				case SOCKET_RET_OK:
					m_EventNetDispatch.MakeSocketDispatching(nReadySocket);
					break;
				case SOCKET_RET_FREE:
					break;
				default:
					assert(0);
				}
			}
			catch(const LWPR::Exception& e)
			{
				fprintf(stderr, "%s\n", e.what());
			}
		}
	}
};
