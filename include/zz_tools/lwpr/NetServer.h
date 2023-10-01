#pragma once

#include "LWPRType.h"
#include "Socket.h"
#include "Exception.h"
#include <string>

namespace ZZTools
{
	class NetServer;

	const INT32 NET_SERVER_RESERVED_FDS = 32;

	/**
	 * NetServer����
	 */
	typedef enum net_server_type_e
	{
		NET_SERVER_ST,	// ���߷�ʽ
		NET_SERVER_MT	// ���߷�ʽ
	} NET_SERVER_TYPE_E;

	/**
	 * ���û�����̳У������������ݴ���
	 */
	class NetServerHandler
	{
	public:
		NetServerHandler() {};
		virtual ~NetServerHandler() {};

		inline void SetNetServer(NetServer* server)
		{
			assert(server != NULL);
			m_pNetServer = server;
		}

		inline NetServer* GetNetServer()
		{
			assert(m_pNetServer != NULL);
			return m_pNetServer;
		}

		/**
		 * �յ��ͻ�����������
		 */
		virtual SOCKET_RET_TYPE_E DoReceiveNormalData(SOCKET_FD_T fd);

		/**
		 * �رճ�ʱ�ͻ���֮ǰ
		 * ע�⣺EventNetDispatch�ڼ���״̬��ִ�У�����DoCloseTimeoutSocketִ��ʱ��Ӧ������
		 *		Ĭ�ϲ�ִ�У���Ҫ����bExecuteCloseHandlerѡ���ִ��
		 */
		virtual SOCKET_RET_TYPE_E DoCloseExpiredSocket(SOCKET_FD_T fd);

		/**
		 * selectϵͳ���ó�ʱ�����ִ��
		 */
		virtual void DoSelectTimeout();

		/**
		 * �ͻ�����Ŀ�б仯���ִ��
		 */
		virtual void DoClientAmountUpdate(int amount, SOCKET_FD_T fd, bool increase);

		/**
		 * �߳�����
		 */
		virtual void DoThreadStart();

		/**
		 * �߳̽���
		 */
		virtual void DoThreadEnd();

	private:
		NetServer* m_pNetServer;
	};

	/**
	 * NetServer�������Ϊ����
	 */
	typedef struct net_server_option
	{
		std::string strAddr;			// ������ַ��Ĭ��ֵΪ�գ���ʽ-IP:PORT
		SOCKET_FD_T nFdListen;			// ����SOCKET��������Ĭ��Ϊ-1������ⲿ��������>=0��
		bool bDropListenFD;				// �����������˺���ȥaccept ������������Ĭ��Ϊfalse
		int nThreadPoolMin;				// �̳߳��������߳���
		int nThreadPoolMax;				// �̳߳�������߳���
		int nThreadPoolMaxIdle;			// �̳߳����������߳���
		int nConnectionMax;				// ���������������Ĭ��ֵ��0��������
		int nSelectTimeout;				// SLECT��ʱʱ�䣨Ĭ��Ϊ0�����Ϊ0����ʾ��Զ����ʱ, ���select��Զ����������׼ȷ���socket���ڣ�
		int nSocketExpiredTime;			// SOCKET����ʱ�䣬������ʱ���û�����ݣ���ǿ�ƹر�SOCKET�����Ϊ0����ʾ��Զ�����ڣ�
		NetServerHandler* pHandler;		// ��������������¼� ���û��������ã�

		/**
		 * ���캯��
		 */
		net_server_option();
	} NET_SERVER_OPTION_T;

	class NetServer
	{
	protected:

		NetServer();

	public:

		virtual ~NetServer();

		static NetServer* CreateNetServer(NET_SERVER_TYPE_E type, const NET_SERVER_OPTION_T& opt);

		virtual void ActivateService() = 0;

		virtual void Shutdown() = 0;

		virtual void Go() = 0;

		virtual void EnableSocket(SOCKET_FD_T fd) = 0;

		virtual void WriteSocketPositive(SOCKET_FD_T fd, char *buf, int length) = 0;

		virtual void WriteSocketPositive(const SOCKETFDList& fds, char *buf, int length) = 0;

		virtual bool IsServiceBusy() = 0;

		virtual std::string GetListenAddr() = 0;

		virtual void DestroySocket(SOCKET_FD_T fd) = 0;

		virtual INT32 ClientTotal() = 0;

	private:
	};

	DEFINE_EXCEPTION(LWPR_EVENT_NET_CREATE_SERVER_SOCKET_ERROR);
	DEFINE_EXCEPTION(LWPR_EVENT_NET_CONNECT_SELEF_ERROR);
	DEFINE_EXCEPTION(LWPR_EVENT_NET_GETSOCKNAME_ERROR);
	DEFINE_EXCEPTION(LWPR_EVENT_NET_GETLOCALIP_ERROR);
	DEFINE_EXCEPTION(LWPR_EVENT_NET_CREATE_PIPE_ERROR);
}
