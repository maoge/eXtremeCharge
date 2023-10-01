#include "MTNetServer.h"
#include "Synchronized.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

namespace ZZTools
{
	MTNetServer::MTNetServer(const NET_SERVER_OPTION_T& opt)
		: m_fdMax(0),
		  m_NetServerOption(opt),
		  m_nIdleThread(0),
		  m_nTotalThread(0),
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

		FD_ZERO(&m_setActiveFd);
		m_ReadableSockets.reserve(m_NetServerOption.nConnectionMax + 1);

		opt.pHandler->SetNetServer(this);
	}

	MTNetServer::~MTNetServer()
	{
		delete m_NetServerOption.pHandler;
	}

	void MTNetServer::ActivateService()
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

		// �����ܵ�
		int fdpipe[2] = {0};
		int nCode = pipe(fdpipe);
		if(-1 == nCode)
		{
			throw LWPR_EVENT_NET_CREATE_PIPE_ERROR(EXCEPTION_TRACE, "ConnectRemoteHost error");
		}
		m_fdSelfServer = fdpipe[0];
		m_fdSelfClient = fdpipe[1];

		// ȡ���socket
		m_fdMax = (m_fdSelfClient > m_fdListen) ? m_fdSelfClient : m_fdListen;
		m_fdMax = (m_fdSelfServer > m_fdMax) ? m_fdSelfServer : m_fdMax;

		// ����m_fdActive
		FD_SET(m_fdListen, &m_setActiveFd);
		FD_SET(m_fdSelfServer, &m_setActiveFd);

		// �����̶߳��󣬲������߳�
		for(int i = 0; i < m_NetServerOption.nThreadPoolMin; i++)
		{
			WorkThread* pThread = new WorkThread(*this);
			pThread->Start();
			m_WorkThreadList.push_back(pThread);
		}

		// ���������߳�
		this->Start();
	}

	void MTNetServer::Shutdown()
	{
		if(IsContinue())
		{
			SOCKET_SELF_MSG_T msg;
			msg.nMsgType = SOCKET_SELF_MSG_EXIT_THREAD;
			msg.nSocketFd = 0;

			{
				Synchronized syn(m_MutexClient);
				Socket::WriteSocket(m_fdSelfClient, (const char*)&msg, sizeof(msg));
			}

			Socket::CloseSocket(m_fdListen);
			this->StopRunning();
		}
	}

	void MTNetServer::Go()
	{
		while(IsContinue() && !IsExited())
		{
			Thread::Sleep(3);
		}
	}

	void MTNetServer::EnableSocket(SOCKET_FD_T fd)
	{
		assert(fd >= 0);

		SOCKET_SELF_MSG_T msg;
		msg.nMsgType = SOCKET_SELF_MSG_ADD_FD;
		msg.nSocketFd = fd;

		Synchronized syn(m_MutexClient);
		SOCKET_RET_TYPE_E nRet = Socket::WriteSocket(m_fdSelfClient, (const char*)&msg, sizeof(msg));
		if(nRet != SOCKET_RET_OK)
		{
			// �źŷ���ʱ����ִ�е�����
			Socket::CloseSocket(fd);
		}
	}

	bool MTNetServer::IsServiceBusy()
	{
		return IsIdleThreadTooLittle();
	}

	std::string MTNetServer::GetListenAddr()
	{
		return m_strListenAddr;
	}

	void MTNetServer::Run()
	{
		while(IsContinue())
		{
			// ÿ��10����һ��
			Wait(10);

			// ������ȥ���߳�
			ClearDeadThread();

			// ��̬�����߳�
			if(IsNeededToCreateThread())
			{
				WorkThread* pThread = new WorkThread(*this);
				pThread->Start();
				m_WorkThreadList.push_back(pThread);
			}

			// ��̬�����߳�
			else if(IsNeededToDestroyThread())
			{
				DestroyOneThread();
			}
		}

		// ���̳߳����߳��˳�
		WorkThreadList::iterator it = m_WorkThreadList.begin();
		for(; it != m_WorkThreadList.end(); it++)
		{
			(*it)->StopRunning();
		}

		// ������Select���߳��˳�
		DestroyOneThread();

		// �����߳���Դ
		for(int i = 0; i < 5 && m_WorkThreadList.size() > 0; i++)
		{
			ClearDeadThread();
			Thread::Sleep(1);
		}

		Socket::CloseSocket(m_fdSelfServer);
		Socket::CloseSocket(m_fdSelfClient);
	}

	void MTNetServer::ClearDeadThread()
	{
		WorkThreadList::iterator it = m_WorkThreadList.begin();
		for(; it != m_WorkThreadList.end();)
		{
			if(!(*it)->IsContinue())
			{
				WorkThread* p = *it;
				p->DecRef();
				it = m_WorkThreadList.erase(it);
			}
			else
			{
				it++;
			}
		}
	}

	bool MTNetServer::IsIdleThreadTooLittle()
	{
		return (m_nIdleThread < 3);
	}

	bool MTNetServer::IsIdleThreadTooMuch()
	{
		return (m_nIdleThread > m_NetServerOption.nThreadPoolMaxIdle);
	}

	bool MTNetServer::IsNeededToCreateThread()
	{
		return IsIdleThreadTooLittle() && (m_nTotalThread < m_NetServerOption.nThreadPoolMax);
	}

	bool MTNetServer::IsNeededToDestroyThread()
	{
		return IsIdleThreadTooMuch() && (m_nIdleThread > m_NetServerOption.nThreadPoolMaxIdle);
	}

	void MTNetServer::DestroyOneThread()
	{
		SOCKET_SELF_MSG_T msg;
		msg.nMsgType = SOCKET_SELF_MSG_EXIT_THREAD;
		msg.nSocketFd = 0;

		Synchronized syn(m_MutexClient);
		Socket::WriteSocket(m_fdSelfClient, (const char*)&msg, sizeof(msg));
	}

	void MTNetServer::DestroySocket(SOCKET_FD_T fd)
	{
		SOCKET_SELF_MSG_T msg;
		msg.nMsgType = SOCKET_SELF_MSG_REMOVE_FD;
		msg.nSocketFd = fd;

		Synchronized syn(m_MutexClient);
		if (SOCKET_RET_OK != Socket::WriteSocket(m_fdSelfClient, (const char*)&msg, sizeof(msg), 60))
		{
			Socket::ClearSocket(fd);
		}
		m_NetServerOption.pHandler->DoClientAmountUpdate(--m_nTotalClient, fd, false);
	}

	// TODO:	�������Ǵ洢ÿ��socket��Ӧ�Ŀͻ��˵�ַ������Ψһ��ʶ��SOCKET��
	//			���Ա��⵱��SOCKET����رգ������µ�SOCKETռ������������������Ϣ���󷢡�
	void MTNetServer::WriteSocketPositive(SOCKET_FD_T fd, char *buf, int length)
	{
		assert(fd >= 0);
		assert(buf != NULL);
		assert(length > 0);

		// д�����������߳�ģ�Ͷ���
		{
			Synchronized syn(m_MutexWrite);
			Buffer_var vBuf = new Buffer(length);
			memcpy(vBuf->Inout(), buf, length);
			vBuf->Size(length);

			// �����ǰsocket��δ�´����Ϣ���򸲸ǵ�
			SocketList::iterator it = m_WriteableSockets.find(fd);
			if(it != m_WriteableSockets.end())
			{
				it->second->DecRef();
				m_WriteableSockets.erase(it);
			}

			// �����µ���Ϣ
			m_WriteableSockets[fd] = Buffer_var::Duplicate(vBuf);
		}

		// ֪ͨ�����߳�ģ��
		{
			SOCKET_SELF_MSG_T msg;
			msg.nMsgType = SOCKET_SELF_MSG_WRITE_DATA_TO_CLIENT;
			Synchronized syn(m_MutexClient);
			Socket::WriteSocket(m_fdSelfClient, (const char*)&msg, sizeof(msg));
		}
	}

	void MTNetServer::WriteSocketPositive(const SOCKETFDList& fds, char *buf, int length)
	{
		assert(buf != NULL);
		assert(length > 0);

		// д�����������߳�ģ�Ͷ���
		{
			Synchronized syn(m_MutexWrite);
			Buffer_var vBuf = new Buffer(length);
			memcpy(vBuf->Inout(), buf, length);
			vBuf->Size(length);

			for(size_t i = 0; i < fds.size(); i++)
			{
				// �����ǰsocket��δ�´����Ϣ���򸲸ǵ�
				SocketList::iterator it = m_WriteableSockets.find(fds[i]);
				if(it != m_WriteableSockets.end())
				{
					it->second->DecRef();
					m_WriteableSockets.erase(it);
				}

				// �����µ���Ϣ
				m_WriteableSockets[fds[i]] = Buffer_var::Duplicate(vBuf);
			}
		}

		// ֪ͨ�����߳�ģ��
		{
			SOCKET_SELF_MSG_T msg;
			msg.nMsgType = SOCKET_SELF_MSG_WRITE_DATA_TO_CLIENT;
			Synchronized syn(m_MutexClient);
			Socket::WriteSocket(m_fdSelfClient, (const char*)&msg, sizeof(msg));
		}
	}

	void MTNetServer::DestroySocketInternal(SOCKET_FD_T fd)
	{
		m_NetServerOption.pHandler->DoClientAmountUpdate(--m_nTotalClient, fd, false);
		Socket::CloseSocket(fd);
	}

	INT32 MTNetServer::ClientTotal()
	{
		return m_nTotalClient;
	}

	//----------------------------------------------------------------------------------
	// class WorkThread
	//----------------------------------------------------------------------------------
	MTNetServer::WorkThread::WorkThread(MTNetServer& server)
		: m_MTNetServer(server)
	{
	}

	void MTNetServer::WorkThread::Run()
	{
		AutomaticCount autoTotalThread(m_MTNetServer.m_nTotalThread);

		// ִ���û�ָ�����̳߳�ʼ�����������
		class PairDo
		{
			WorkThread* m_p;
		public:
			PairDo(WorkThread* p) : m_p(p)
			{
				try
				{
					m_p->m_MTNetServer.m_NetServerOption.pHandler->DoThreadStart();
				}
				catch(...)
				{
				}
			}
			~PairDo()
			{
				try
				{
					m_p->m_MTNetServer.m_NetServerOption.pHandler->DoThreadEnd();
				}
				catch(...)
				{
				}
			}
		} _PairDo(this);

		while(IsContinue())
		{
			try
			{
				// �ɶ�socket
				SOCKET_FD_T nReadySocket = -1;
				SOCKET_FD_T nWriteSocket = -1;
				Buffer_var bufWrite;

				{
					// �����̼߳���
					AutomaticCount autoIdleThread(m_MTNetServer.m_nIdleThread);

					// ��������ȡ��Ҫ��д��Socket�������ٽ�����
					Synchronized syn(m_MTNetServer.m_MutexEvent);

					// ���û�пɶ�socket/��дsocket����select
					while(-1 == nReadySocket && -1 == nWriteSocket)
					{
						// �жϵ�ǰ�߳��Ƿ���Լ���ִ��
						if(!m_MTNetServer.IsContinue())
						{
							return;
						}

						// ��һ���ɶ�socket����ʼ����
						if(m_MTNetServer.m_ReadableSockets.size() > 0)
						{
							nReadySocket = m_MTNetServer.m_ReadableSockets.back();
							m_MTNetServer.m_ReadableSockets.pop_back();

							// ���¼���Ƿ���socket�ɹ���
							continue;
						}
						// ��һ����дsocket����ʼ����
						else
						{
							Synchronized syn(m_MTNetServer.m_MutexWrite);
							for(SocketList::iterator it = m_MTNetServer.m_WriteableSockets.begin();
							    it != m_MTNetServer.m_WriteableSockets.end(); it++)
							{
								if(FD_ISSET(it->first, &m_MTNetServer.m_setActiveFd))
								{
									FD_CLR(it->first, &m_MTNetServer.m_setActiveFd);
									nWriteSocket = it->first;
									bufWrite = it->second;
									m_MTNetServer.m_WriteableSockets.erase(it);
									break;
								}
							}

							if(-1 != nWriteSocket)
							{
								continue;
							}
						}

						// Select��������������
						fd_set setReadableFd;
						memcpy(&setReadableFd, &m_MTNetServer.m_setActiveFd, sizeof(m_MTNetServer.m_setActiveFd));

						// Socket�����ﵽ���ֵ��ֹͣ���������ӣ�һ��Ϊ���������£��������ฺ�ؽ���Ľ���ִ�У�
						if(m_MTNetServer.m_nTotalClient >= m_MTNetServer.m_NetServerOption.nConnectionMax
						   && m_MTNetServer.m_NetServerOption.bDropListenFD)
						{
							FD_CLR(m_MTNetServer.m_fdListen, &setReadableFd);
						}

						// Select��ʱʱ������
						struct timeval tv = {0};
						tv.tv_sec = m_MTNetServer.m_NetServerOption.nSelectTimeout;
						tv.tv_usec = 0;
						struct timeval *ptv = &tv;
						ptv = (0 == tv.tv_sec) ? NULL : ptv;

						int nCode = select(m_MTNetServer.m_fdMax + 1, &setReadableFd, NULL, NULL, ptv);
						// ��socket�ɶ�
						if(nCode > 0)
						{
							for(int i = 0; i <= m_MTNetServer.m_fdMax; i++)
							{
								if(FD_ISSET(i, &setReadableFd))
								{
									// ����socket����Ծʱ��
									m_MTNetServer.m_mapSocketTime[i] = time(NULL);

									// ������������
									if(i == m_MTNetServer.m_fdListen)
									{
										SOCKET_FD_T fd = Socket::AcceptSocket(i);
										if(fd != -1)
										{
											// ������������ӳ������õ����������CloseSocket
											if(m_MTNetServer.m_nTotalClient >= m_MTNetServer.m_NetServerOption.nConnectionMax)
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
												FD_SET(fd, &m_MTNetServer.m_setActiveFd);
												m_MTNetServer.m_mapSocketTime[fd] = time(NULL);
												m_MTNetServer.m_fdMax = (fd > m_MTNetServer.m_fdMax) ? fd : m_MTNetServer.m_fdMax;
												try
												{
													m_MTNetServer.m_NetServerOption.pHandler->DoClientAmountUpdate(++m_MTNetServer.m_nTotalClient, fd, true);
												}
												catch(...)
												{
												}
											}
										}
									}
									// �����������ӣ������ڲ�������Ϣ�Ĵ���
									else if(i == m_MTNetServer.m_fdSelfServer)
									{
										Buffer buf;
										SOCKET_RET_TYPE_E res = Socket::ReadSocket(i, buf, sizeof(SOCKET_SELF_MSG_T));
										if(res == SOCKET_RET_OK)
										{
											SOCKET_SELF_MSG_T* pMsg = (SOCKET_SELF_MSG_T*)buf.Inout();

											switch(pMsg->nMsgType)
											{
											case SOCKET_SELF_MSG_REMOVE_FD:
												{
													Synchronized syn(m_MTNetServer.m_MutexWrite);
													SocketList::iterator it = m_MTNetServer.m_WriteableSockets.find(pMsg->nSocketFd);
													if(m_MTNetServer.m_WriteableSockets.end() != it)
													{
														it->second->DecRef();
														m_MTNetServer.m_WriteableSockets.erase(it);
													}
												}

												FD_CLR(pMsg->nSocketFd, &m_MTNetServer.m_setActiveFd);
												Socket::CloseSocket(pMsg->nSocketFd);
												break;
											case SOCKET_SELF_MSG_ADD_FD:
												FD_SET(pMsg->nSocketFd, &m_MTNetServer.m_setActiveFd);
												m_MTNetServer.m_mapSocketTime[pMsg->nSocketFd] = time(NULL);
												break;
											case SOCKET_SELF_MSG_WRITE_DATA_TO_CLIENT:
												break;
											case SOCKET_SELF_MSG_EXIT_THREAD:
												StopRunning();
												break;
											default:
												assert(0);
											}
										}
									}
									// �������пɶ�socket
									else
									{
										m_MTNetServer.m_ReadableSockets.push_back(i);
										FD_CLR(i, &m_MTNetServer.m_setActiveFd);
									}
								}
							}
						}
						// select ��ʱ
						else if(nCode == 0)
						{
							// ��鲻��Ծ��socket
							if(m_MTNetServer.m_NetServerOption.nSocketExpiredTime > 0)
							{
								for(int i = 0; i <= m_MTNetServer.m_fdMax; i++)
								{
									if(FD_ISSET(i, &m_MTNetServer.m_setActiveFd))
									{
										if(i == m_MTNetServer.m_fdListen)
										{
										}
										else if(i == m_MTNetServer.m_fdSelfServer)
										{
										}
										else
										{
											if((time(NULL) - m_MTNetServer.m_mapSocketTime[i]) >= m_MTNetServer.m_NetServerOption.nSocketExpiredTime)
											{
												try
												{
													m_MTNetServer.m_NetServerOption.pHandler->DoCloseExpiredSocket(i);
												}
												catch(...)
												{
												}

												m_MTNetServer.DestroySocket(i);
												FD_CLR(i, &m_MTNetServer.m_setActiveFd);
											}
										}
									}
								}
							}

							// �����û���ҵ����
							try
							{
								m_MTNetServer.m_NetServerOption.pHandler->DoSelectTimeout();
							}
							catch(...)
							{
							}
						}
					} //while(-1 == nReadySocket && -1 == nWriteSocket)
				} // end of Lock

				// ׼�������ҵ�������
				SOCKET_RET_TYPE_E nRet = SOCKET_RET_OK;
				if(-1 != nWriteSocket)
				{
					nRet = Socket::WriteSocket(nWriteSocket, bufWrite->Inout(), bufWrite->Size(), 3);
					switch(nRet)
					{
					case SOCKET_RET_FAILED:
					case SOCKET_RET_TIMEOUT:
					case SOCKET_RET_NOT_WRABLE:
						m_MTNetServer.DestroySocketInternal(nWriteSocket);
						break;
					case SOCKET_RET_OK:
						m_MTNetServer.EnableSocket(nWriteSocket);
						break;
					case SOCKET_RET_FREE:
						break;
					default:
						assert(0);
					}
				}
				else if(-1 != nReadySocket)
				{
					try
					{
						nRet = m_MTNetServer.m_NetServerOption.pHandler->DoReceiveNormalData(nReadySocket);
					}
					catch(...)
					{
						nRet = SOCKET_RET_FAILED;
					}

					switch(nRet)
					{
					case SOCKET_RET_FAILED:
					case SOCKET_RET_TIMEOUT:
					case SOCKET_RET_NOT_WRABLE:
						m_MTNetServer.DestroySocketInternal(nReadySocket);
						break;
					case SOCKET_RET_OK:
						m_MTNetServer.EnableSocket(nReadySocket);
						break;
					case SOCKET_RET_FREE:
						break;
					default:
						assert(0);
					}
				}

				// �����̳߳�
				if(m_MTNetServer.IsNeededToCreateThread() || m_MTNetServer.IsNeededToDestroyThread())
				{
					m_MTNetServer.Notify();
				}
			}
			catch(const Exception& e)
			{
				fprintf(stderr, "%s\n", e.what());
			}
		}
	}
}
