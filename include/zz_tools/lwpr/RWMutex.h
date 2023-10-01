#pragma once

#include "LWPRType.h"
#include "Exception.h"

namespace ZZTools
{

	typedef pthread_rwlock_t PTHREAD_RWLOCK_T;

	class RWMutex
	{
	public:

		/**
		 * ���캯��
		 */
		RWMutex();

		/**
		 * ��������
		 */
		~RWMutex();

		/**
		 * ���� Read Lock
		 */
		void RDLock();

		/**
		 * д�� Write Lock
		 */
		void WRLock();

		/**
		 * �ͷ���
		 */
		void Unlock();

	private:

		PTHREAD_RWLOCK_T m_rwlock;
	};
};
