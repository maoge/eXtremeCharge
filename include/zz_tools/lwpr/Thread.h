#pragma once

#include "LWPRType.h"
#include "Thread.h"
#include "Exception.h"
#include "Var.h"
#include "Object.h"
#include <pthread.h>

namespace ZZTools
{
	typedef pthread_attr_t  THREAD_ATTR_T;

	/**
	 * Ĭ��Ϊ�����̣߳������Ե���join����
	 */
	class Thread : virtual public Object
	{
		Thread(const Thread& rhs) {}
		void operator= (const Thread& rhs) {}

	public:
		/**
		 * ���캯��
		 */
		Thread();

		/**
		 * ��������
		 */
		virtual ~Thread();

		/**
		 * �����߳�Ϊ�Ƿ����̣߳��û����Ե���Join�������߳�
		 * ע�⣺��Ҫ��Start֮ǰ����
		 */
		void EnableJoinable();

		/**
		 * �����߳�Ϊ�����̣߳��û������Ե���Join�������߳�
		 * ע�⣺��Ҫ��Start֮ǰ����
		 */
		void DisableJoinable();

		/**
		 * �����̣߳���ʹ�����߳�����
		 */
		void Join();

		/**
		 * ʹ���߳̿�ʼִ��
		 */
		void Start();

		/**
		 * ִ���̵߳ľ��幤������������д
		 */
		virtual void Run() = 0;

		/**
		 * �����߳�ID
		 */
		THREAD_ID_T GetId();

		/**
		 * �߳��Ƿ��Ѿ��˳���ֹͣ����
		 */
		bool IsExited();

		/**
		 * �Ƿ���Լ�������
		 */
		bool IsContinue();

		bool IsStart();

		/**
		 * ֹͣ�߳����У����߳��˳�
		 */
		void StopRunning();

	public:

		/**
		 * �߳�Sleep
		 */
		static void Sleep(double seconds);

		/**
		 * ������ǰ�߳�
		 */
		static void ExitThread(void *status);

		/**
		 * �����߳�ID
		 */
		static THREAD_ID_T GetCurrentThreadId();

	private:
		static void* DoThreadWork(void* arg);

	private:
		THREAD_ID_T		m_threadId;			// �߳�ID
		THREAD_ATTR_T	m_threadAttr;		// �߳����Զ���
		bool			m_bStarted;			// �߳��Ƿ��Ѿ�����
		bool			m_bAttrDestroyed;	// �߳����Զ����Ƿ��Ѿ�����
		volatile bool	m_bContinue;		// �߳��Ƿ���Լ�������
	};

	DEFINE_EXCEPTION(LWPR_THREAD_API_ERR);

	DECLAREVAR(Thread);
};
