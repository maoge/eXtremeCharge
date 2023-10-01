#pragma once

#include "LWPRType.h"
#include "Buffer.h"
#include "OutputStream.h"
#include "InputStream.h"
#include <string>

namespace ZZTools
{
	typedef enum socket_ret_type_e
	{
		SOCKET_RET_FAILED = -1,		// Socket����
		SOCKET_RET_OK = 0,			// Socket״̬���������ɶ�����д
		SOCKET_RET_TIMEOUT = 1,		// Socket��д��ʱ
		SOCKET_RET_FREE = 2,		// Socket����EventDispatch����
		SOCKET_RET_NOT_WRABLE = 3,	// Socket�����������ݣ�����д��������.һ������select
	} SOCKET_RET_TYPE_E;

	/**
	 * �˽ṹ���������紫�䣨�洢Ϊ�����ֽ���
	 */
	typedef struct socket_addr_t
	{
		UINT32 ip;
		UINT16 port;

		void Marshal(OutputStream& out);
		void Unmarshal(InputStream& in);
	} SOCKET_ADDR_T;
	
	typedef struct addr_t
	{
		char ip[20];
		int port;
	} ADDR_T;

	class Socket
	{
	public:
		/**
		 * ������ַ��
		 * ��ַ����ʽ1 0
		 * ��ַ����ʽ2 192.168.0.1:26000
		 * ��ַ����ʽ3 0.0.0.0:26000
		 * ��ַ����ʽ4 192.168.0.1:26000~36000
		 * ��ַ����ʽ5 0.0.0.0:26000~36000
		 */
		static bool SplitAddr(const char* addr, std::string& host, int& min_port, int& max_port);

		/**
		 * ��ַת������
		 */
		static bool AddrTransform(const char* addr_src, SOCKET_ADDR_T& addr_dest);
		static void AddrTransform(const SOCKET_ADDR_T& addr_src, std::string& addr_dest);
		static std::string AddrTransform(const SOCKET_ADDR_T& addr_src);
		static bool IsAddrEqual(const char* addr1, const char* addr2);
		
		static void GetLocalAddr(int fd, ADDR_T& addr);
		static void GetPeerAddr(int fd, ADDR_T& addr);

		/**
		 * ����Server Socket���������˿�
		 * @return ���ؼ���Socket���������������-1
		 */
		static SOCKET_FD_T CreateServerTCPSocket(const char* host, int port);

		/**
		 * ����Server Socket���������˿�
		 * @return ���ؼ���Socket���������������-1
		 */
		static SOCKET_FD_T CreateServerTCPSocket(const char* addr);

		/**
		 * ����UDP Socket
		 * @return ����Socket���������������������-1
		 */
		static SOCKET_FD_T CreateServerUDPSocket(UINT16& port);

		/**
		 * ����UDP Socket
		 * @return ����Socket���������������������-1
		 */
		static SOCKET_FD_T CreateClientUDPSocket();

		/**
		 * �����Ӷ��л�ȡ����
		 * @return ��������Socket���������������-1
		 */
		static SOCKET_FD_T AcceptSocket(SOCKET_FD_T fd);

		/**
		 * ����Զ�˷�����
		 * @return ��������Socket���������������-1
		 */
		static SOCKET_FD_T ConnectRemoteHost(const char* host, int port);

		/**
		 * ����Զ�˷�����
		 * @return ��������Socket���������������-1
		 */
		static SOCKET_FD_T ConnectRemoteHost(const char* addr);

		/**
		 * �ر�socket
		 * ע�⣺ִ����رղ����󣬻���fd=-1
		 */
		static void CloseSocket(SOCKET_FD_T& fd);

		static void CloseSocketWithShutdown(SOCKET_FD_T& fd);

		/**
		 * ����Socket�Ƿ�ɶ�
		 *		timeout = 0 ������
		 *		timeout = INT_MAX һֱ����
		 *		timeout > 0 ����ָ��ʱ��
		 */
		static SOCKET_RET_TYPE_E IsSocketReadable(SOCKET_FD_T fd, int timeout);

		/**
		 * ����Socket�Ƿ�ɶ������Socket���ɶ�����һֱ����
		 */
		static SOCKET_RET_TYPE_E IsSocketReadable(SOCKET_FD_T fd);

		/**
		 * ����Socket�Ƿ��д
		 *		timeout = 0 ������
		 *		timeout = INT_MAX һֱ����
		 *		timeout > 0 ����ָ��ʱ��
		 */
		static SOCKET_RET_TYPE_E IsSocketWritable(SOCKET_FD_T fd, int timeout);

		/**
		 * ����Socket�Ƿ��д�����Socket����д����һֱ����
		 */
		static SOCKET_RET_TYPE_E IsSocketWritable(SOCKET_FD_T fd);

		/**
		 * ��Socket
		 *		timeout = 0 ������
		 *		timeout = INT_MAX һֱ����
		 *		timeout > 0 ����ָ��ʱ��
		 */
		static SOCKET_RET_TYPE_E ReadSocket(SOCKET_FD_T fd, Buffer& buf, int length, int timeout);

		/**
		 * ��Socket������ģʽ
		 */
		static SOCKET_RET_TYPE_E ReadSocket(SOCKET_FD_T fd, Buffer& buf, int length);

		/**
		 * ��Socket
		 *		timeout = 0 ������
		 *		timeout = INT_MAX һֱ����
		 *		timeout > 0 ����ָ��ʱ��
		 */
		static SOCKET_RET_TYPE_E ReadSocket(SOCKET_FD_T fd, char* buf, int length, int timeout);

		/**
		 * ��Socket������ģʽ
		 */
		static SOCKET_RET_TYPE_E ReadSocket(SOCKET_FD_T fd, char* buf, int length);

		/**
		 * дSocket���������Ϊ0���򷵻سɹ�
		 *		timeout = 0 ������
		 *		timeout = INT_MAX һֱ����
		 *		timeout > 0 ����ָ��ʱ��
		 */
		static SOCKET_RET_TYPE_E WriteSocket(SOCKET_FD_T fd, const char* buf, int length, int timeout);

		/**
		 * дSocket������ģʽ
		 */
		static SOCKET_RET_TYPE_E WriteSocket(SOCKET_FD_T fd, const char* buf, int length);

		/**
		 * ��UDP Socketд����
		 */
		static SOCKET_RET_TYPE_E WriteUDPSocket(SOCKET_FD_T fd, const char* addr, const char* buf, int length);
		static SOCKET_RET_TYPE_E WriteUDPSocket(SOCKET_FD_T fd, int port, const char* buf, int length);

		/**
		 * ���UDP Socket����������
		 */
		static void ClearUDPSocket(SOCKET_FD_T fd);

		/**
		 * ���TCP Socket����������
		 */
		static void ClearSocket(SOCKET_FD_T fd);

	public:
		/**
		 * ��ȡ�Ѿ�������socket��IP��ַ�Ͷ˿ں�
		 */
		static bool GetSocketAddress(int fd, std::string& ip, int& port);

		/**
		 * ��ȡ����������IP��ַ
		 */
		static bool GetLocalHostIP(std::string& ip);
		
		/*
		 * ������������²�Ҫ��Ҫָ��host
		 */ 
		static bool IsLocalPortUsed(int port);

		/*
		 * ���������������Ҫ, Ҫָ��host���ж϶˿���û��ʹ��
		 */
		static bool IsLocalPortUsed(const char* host, int port);

	private:
	};
};
