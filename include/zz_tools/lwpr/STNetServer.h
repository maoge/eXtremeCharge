#pragma once

#include "LWPRType.h"
#include "NetServer.h"
#include <vector>
#include <map>

namespace ZZTools
{
	class STNetServer:  public NetServer
	{
		typedef std::vector< SOCKET_FD_T > SocketVector;
		typedef std::map< SOCKET_FD_T, time_t > SocketMap;

	public:

		STNetServer(const NET_SERVER_OPTION_T& opt);
		~STNetServer();

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
		bool				m_bStoped;			// �Ƿ��Ѿ�ֹͣ����
		SOCKET_FD_T			m_fdListen;			// �����׽ӿ�
		SocketMap			m_mapSocketTime;	// ��ǰ���л��SOCKET������Ծʱ��
		NET_SERVER_OPTION_T	m_NetServerOption;	// ѡ������
		std::string			m_strListenAddr;
		AtomicInteger		m_nTotalClient;		// �ͻ�������
	};
}
