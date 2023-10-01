#pragma once

#include "LWPRType.h"
#include "Exception.h"
#include "Socket.h"
#include "Thread.h"
#include "Mutex.h"
#include <time.h>
#include <map>
#include <vector>
#include <list>

// for select
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

namespace ZZTools
{
	class EventNetDispatch;

	/**
	 * ���û�����̳У������������ݴ���
	 */
	class EventNetHandler
	{
	public:
		/**
		 * ���캯��
		 */
		EventNetHandler() {};

		/**
		 * ��������
		 */
		virtual ~EventNetHandler() {};

		inline void SetEventNetDispatch(EventNetDispatch* dispatch)
		{
			assert(dispatch != NULL);
			m_pEventNetDispatch = dispatch;
		}

		inline EventNetDispatch* GetEventNetDispatch()
		{
			assert(m_pEventNetDispatch != NULL);
			return m_pEventNetDispatch;
		}

		/**
		 * �յ��ͻ�����������
		 */
		virtual SOCKET_RET_TYPE_E DoReceiveNormalData(SOCKET_FD_T fd);

		/**
		 * �յ��ͻ����µ�����
		 * ע�⣺EventNetDispatch�ڼ���״̬��ִ�У�����DoReceiveConnectionִ��ʱ��Ӧ�����̡�
		 *		Ĭ�ϲ�ִ�У���Ҫ����bExecuteRcvConnHandlerѡ���ִ��
		 */
		virtual SOCKET_RET_TYPE_E DoReceiveConnection(SOCKET_FD_T fd);

		/**
		 * �رճ�ʱ�ͻ���֮ǰ
		 * ע�⣺EventNetDispatch�ڼ���״̬��ִ�У�����DoCloseTimeoutSocketִ��ʱ��Ӧ������
		 *		Ĭ�ϲ�ִ�У���Ҫ����bExecuteCloseHandlerѡ���ִ��
		 */
		virtual SOCKET_RET_TYPE_E DoCloseExpiredSocket(SOCKET_FD_T fd);

	protected:
		EventNetDispatch* m_pEventNetDispatch;

	};

	/**
	 * ѡ�����ã����û��������ı�EventNetDispatch����Ϊ
	 */
	typedef struct event_net_option
	{
		std::string strIP;				// ����IP��Ĭ��ֵ��"0.0.0.0"���������ã�
		int nMinPort;					// ����PORT��Ĭ��ֵ��0���������ã�
		int nMaxPort;					// ����PORT��Ĭ��ֵ��0
		int nThreadPoolMin;				// �̳߳��������߳���
		int nThreadPoolMax;				// �̳߳�������߳���
		int nThreadPoolMaxIdle;			// �̳߳����������߳���
		int nHousekeepInterval;			// ���SOCKET�Ƿ���ڵļ��ʱ��
		int nSocketExpiredTime;			// Socket����ʱ�䣬������ʱ���û�����ݣ���ǿ�ƹر�SOCKET
		bool bAllowDoHousekeep;			// �Ƿ�����ִ��-���SOCKET�Ƿ����
		bool bAllowDoCloseHandler;		// �Ƿ�����ִ��-DoCloseExpiredSocket
		bool bAllowDoRcvConnHandler;	// �Ƿ�����ִ��-DoReceiveConnection
		EventNetHandler* pHandler;		// ��������������¼� ���û��������ã�

		/**
		 * Ĭ�Ϲ��캯��
		 */
		event_net_option();
	} EVENT_NET_OPTION_T;

	/**
	 * EventNetDispatchģ���ڲ����䴫�ݵĿ�����Ϣ����
	 */
	typedef enum socket_self_msg_type
	{
		SOCKET_SELF_MSG_ADD_FD,
		SOCKET_SELF_MSG_EXIT_THREAD,
	} SOCKET_SELF_MSG_TYPE_E;

	/**
	 * EventNetDispatchģ���ڲ����䴫�ݵĿ�����Ϣ�ṹ
	 */
	typedef struct socket_self_msg
	{
		SOCKET_SELF_MSG_TYPE_E nMsgType;
		SOCKET_FD_T nSocketFd;
	} SOCKET_SELF_MSG_T;

	class EventNetWorkThread;

	// �����������ڿ���״̬ʱ��
	// ���ÿ��SOCKET�Ļ�Ծ�̶ȣ���SOCKET��ʱ��û�����ݣ���ɾ������
	class EventNetDispatch : public Thread
	{
		friend class EventNetWorkThread;

		typedef std::vector<SOCKET_FD_T> SocketList;

	public:
		/**
		 * ���캯��
		 */
		EventNetDispatch(EVENT_NET_OPTION_T& opt);

		/**
		 * ��������
		 */
		~EventNetDispatch();

		/**
		 * ���������˿ڡ������߳�
		 */
		void Activate();

		/**
		 * ֹͣ�̡߳��ر�socket
		 */
		void Halt();

		/**
		 * ��SOCKET�ǿɷ���ģ�����SOCKET���뵽SELECT������
		 */
		void MakeSocketDispatching(SOCKET_FD_T fd);

		/**
		 * �̳߳��Ƿ���æ״̬
		 */
		bool IsServiceBusy();

	private:
		/**
		 * �����߳����к���
		 */
		virtual void Run();

		/**
		 * ������ȥ���߳�
		 */
		void ClearDeadThread();

	private:

		SOCKET_FD_T m_fdListen;			// �����׽ӿ�
		SOCKET_FD_T m_fdConnSelfServer;	// ����������SOCKET��Server��ɫ
		SOCKET_FD_T m_fdMax;			// ���socket
		fd_set m_setActiveFd;			// ��ǰ���л��socket
		SocketList m_listReadableSocket;// ��ǰ�ɶ���socket������Щsocket�пɶ�����
		std::map< SOCKET_FD_T, time_t > m_mapActiveFd; // ��ǰ���л��socket������Ծʱ��
		Mutex m_MutexEvent;

		SOCKET_FD_T m_fdConnSelfClient;	// ����������SOCKET��Client��ɫ
		Mutex m_MutexClient;

		EVENT_NET_OPTION_T m_EventNetOption;
		std::list< EventNetWorkThread* > m_listThread;
		AtomicInteger m_nIdleThread;
		AtomicInteger m_nTotalThread;
	};

	// ������̴߳�����
	class EventNetWorkThread : public Thread
	{
	public:
		/**
		 * ���캯��
		 */
		EventNetWorkThread(EventNetDispatch& dispatch);
		/**
		 * ��������
		 */
		~EventNetWorkThread();

		/**
		 * �߳�Run����
		 */
		void Run();

	private:
		EventNetDispatch& m_EventNetDispatch;
	};

	DEFINE_EXCEPTION(LWPR_EVENT_NET_CREATE_SERVER_SOCKET_ERROR);
	DEFINE_EXCEPTION(LWPR_EVENT_NET_CONNECT_SELEF_ERROR);
};
