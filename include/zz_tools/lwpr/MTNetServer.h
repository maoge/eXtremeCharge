#pragma once

#include "LWPRType.h"
#include "Exception.h"
#include "Socket.h"
#include "Thread.h"
#include "Mutex.h"
#include "Resource.h"
#include "NetServer.h"
#include "AtomicInteger.h"
#include <time.h>
#include <map>
#include <vector>
#include <list>

namespace ZZTools
{
	/**
	 * MTNetServerģ���ڲ����䴫�ݵĿ�����Ϣ����
	 */
	typedef enum socket_self_msg_type
	{
		SOCKET_SELF_MSG_ADD_FD,
		SOCKET_SELF_MSG_REMOVE_FD,
		SOCKET_SELF_MSG_EXIT_THREAD,
		SOCKET_SELF_MSG_WRITE_DATA_TO_CLIENT,
	} SOCKET_SELF_MSG_TYPE_E;

	/**
	 * MTNetServerģ���ڲ����䴫�ݵĿ�����Ϣ�ṹ
	 */
	typedef struct socket_self_msg
	{
		SOCKET_SELF_MSG_TYPE_E nMsgType;
		SOCKET_FD_T nSocketFd;
		INT32 nLength;
	} SOCKET_SELF_MSG_T;

	/**
	 * TODO: ��һ�����ǽ�������Socketд����ͨ��ͬ��������ʽ��֪MTNetServer���󣬶���ͨ���ܵ���֪��
	 */
	class MTNetServer : public NetServer,
		public Thread,
		public Resource
	{
		class WorkThread : public Thread
		{
		public:
			WorkThread(MTNetServer& server);
			virtual void Run();

		private:
			MTNetServer& m_MTNetServer;
		};

		typedef std::vector< SOCKET_FD_T > SocketVector;
		typedef std::map< SOCKET_FD_T, Buffer_ptr> SocketList;
		typedef std::map< SOCKET_FD_T, time_t > SocketMap;
		typedef std::list< WorkThread* > WorkThreadList;

		friend class WorkThread;

	public:

		MTNetServer(const NET_SERVER_OPTION_T& opt);
		~MTNetServer();

		virtual void ActivateService();

		virtual void Shutdown();

		virtual void Go();

		virtual void EnableSocket(SOCKET_FD_T fd);

		virtual void WriteSocketPositive(SOCKET_FD_T fd, char *buf, int length);

		virtual void WriteSocketPositive(const SOCKETFDList& fds, char *buf, int length);

		virtual bool IsServiceBusy();

		virtual std::string GetListenAddr();

		virtual void DestroySocket(SOCKET_FD_T fd);

		virtual INT32 ClientTotal();

	private:
		virtual void Run();

		inline void ClearDeadThread();

		inline bool IsIdleThreadTooLittle();

		inline bool IsIdleThreadTooMuch();

		inline bool IsNeededToCreateThread();

		inline bool IsNeededToDestroyThread();

		inline void DestroyOneThread();

		void DestroySocketInternal(SOCKET_FD_T fd);

	private:
		SOCKET_FD_T			m_fdListen;			// �����׽ӿ�
		SOCKET_FD_T			m_fdSelfServer;		// ����������SOCKET��SERVER��ɫ
		SOCKET_FD_T			m_fdSelfClient;		// ����������SOCKET��CLIENT��ɫ
		SOCKET_FD_T			m_fdMax;			// ���SOCKET
		SOCKET_SET_T		m_setActiveFd;		// ��ǰ���л��SOCKET

		SocketVector		m_ReadableSockets;	// ��ǰ�ɶ���SOCKET������ЩSOCKET�пɶ�����
		SocketList			m_WriteableSockets;	// ��ǰ��д��SOCKET������Ҫ����ЩSOCKET
		SocketMap			m_mapSocketTime;	// ��ǰ���л��SOCKET������Ծʱ��
		Mutex				m_MutexEvent;		// for WorkerThread
		Mutex				m_MutexClient;		// for m_fdConnSelfClient
		Mutex				m_MutexWrite;		// for m_WriteableSockets
		NET_SERVER_OPTION_T	m_NetServerOption;	// ѡ������
		std::string			m_strListenAddr;	// ������ַ
		WorkThreadList		m_WorkThreadList;	// �����߳��б�
		AtomicInteger		m_nIdleThread;		// �����߳�����
		AtomicInteger		m_nTotalThread;		// �����߳�����
		AtomicInteger		m_nTotalClient;		// �ͻ�������
	};

}
