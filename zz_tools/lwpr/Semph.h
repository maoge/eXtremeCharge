#pragma once

#include "LWPRType.h"
#include "Exception.h"
#include <semaphore.h>

namespace ZZTools
{
	typedef sem_t SEM_T;

	class Semph
	{
	public:

		/**
		 * ���캯��
		 */
		Semph(int value = 1);

		/**
		 * ��������
		 */
		~Semph();

		/**
		 * ����
		 */
		void Lock();

		/**
		 * �ͷ���
		 */
		void Unlock();

	private:
		SEM_T m_sem;
	};

	DEFINE_EXCEPTION(LWPR_THREAD_SEM_INIT_ERR);
	DEFINE_EXCEPTION(LWPR_THREAD_SEM_POST_ERR);
	DEFINE_EXCEPTION(LWPR_THREAD_SEM_WAIT_ERR);
};
