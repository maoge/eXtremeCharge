#pragma once

#include "LWPRType.h"
#include "Buffer.h"
#include "Thread.h"
#include "IPCMSQ.h"
#include "Synchronized.h"
#include "LWPRType.h"
#include <vector>

namespace ZZTools
{
	/**
	 * ���û�����̳У������������ݴ���
	 */
	class EventMsqHandler
	{
	public:
		/**
		 * ���캯��
		 */
		EventMsqHandler() {};

		/**
		 * ��������
		 */
		virtual ~EventMsqHandler() {};

		/**
		 * �յ��ͻ�����������
		 */
		virtual void DoReceiveMsqData(Buffer& msg) = 0;
	};

	/**
	 * ѡ�����ã����û��������ı�EventMsqDispatch����Ϊ
	 */
	typedef struct event_msq_option
	{
		IPCID_T				nMsqId;			// ��Ϣ����ID��Ĭ��ֵ��-1���û��������ã�
		int					nThreadCount;	// �̳߳����߳�������Ĭ��ֵ��10
		EventMsqHandler*	pHandler;		// �����������Ϣ�����¼� ���û��������ã�

		/**
		 * Ĭ�Ϲ��캯��
		 */
		event_msq_option();
	} EVENT_MSQ_OPTION_T;

	/**
	 * �ڲ������ж������̵߳���Ϣ
	 */
	typedef struct event_msq_magic_msg
	{
		long		nMsgType;
		long		nMsgData;

		/**
		 * Ĭ�Ϲ��캯��
		 */
		event_msq_magic_msg()
			: nMsgType(10225799), nMsgData(10225799)
		{
		}
	} EVENT_MSQ_MAGIC_MSG_T;

	class EventMsqThreadImpl;

	class EventMsqDispatch
	{
		friend class EventMsqThreadImpl;

	public:

		/**
		 * ���캯��
		 */
		EventMsqDispatch(const EVENT_MSQ_OPTION_T& opt);

		/**
		 * ��������
		 */
		~EventMsqDispatch();

		/**
		 * ���������˿ڡ������߳�
		 */
		void Start();

		/**
		 * ֹͣ�̡߳��ر�socket
		 */
		void Halt();

		/**
		 * �����̡߳��ر�SOCKET
		 */
		void Reap();

	private:
		Mutex m_Mutex;

		EVENT_MSQ_OPTION_T m_EventMsqOption;
		volatile bool m_bContinue;
		std::vector< EventMsqThreadImpl* > m_Threads;
	};

	class EventMsqThreadImpl : public Thread
	{
	public:
		/**
		 * ���캯��
		 */
		EventMsqThreadImpl(EventMsqDispatch& dispatch);
		/**
		 * ��������
		 */
		~EventMsqThreadImpl();

		/**
		 * �߳�Run����
		 */
		void Run();

	private:
		EventMsqDispatch& m_EventMsqDispatch;
	};
};
