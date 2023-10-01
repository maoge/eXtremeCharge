#pragma once

#include "LWPRType.h"
#include "Exception.h"

namespace ZZTools
{

	typedef pthread_mutex_t  PTHREAD_MUTEX_T;

	class Mutex
	{
	public:

		/**
		 * ���캯��
		 */
		Mutex();

		/**
		 * ��������
		 */
		~Mutex();

		/**
		 * ����
		 */
		void Lock();

		/**
		 * ����������
		 * ��������ɹ�����true�����򷵻�false
		 */
		bool TryLock();

		/**
		 * �ͷ���
		 */
		void Unlock();

		/**
		 * ��ȡ����Ϊ��������׼��
		 */
		PTHREAD_MUTEX_T* GetMutexRef();

	private:

		PTHREAD_MUTEX_T m_mutex;
	};
};
