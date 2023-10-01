#pragma once

#include "LWPRType.h"
#include "Exception.h"
#include "Mutex.h"

namespace ZZTools
{
	typedef pthread_cond_t PTHREAD_COND_T;

	class Resource
	{
	public:
		Resource();

		~Resource();

		void Wait();

		void Wait(int seconds);

		void Notify();

		void Notifyall();

	private:
		Mutex m_mutex;
		PTHREAD_COND_T m_cond;
		volatile int m_nWaitThreads;
		volatile int m_nResource;
	};
};
