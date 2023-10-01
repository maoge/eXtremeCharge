#pragma once

#include "LWPRType.h"
#include "Thread.h"
#include "Resource.h"
#include "ThreadPool.h"
#include "Mutex.h"
#include <list>
#include <time.h>

namespace ZZTools
{
	class TimerThread :  public Thread, public Resource
	{
		typedef struct timer_request
		{
			WorkRequest* pRequest;
			time_t nTimestamp;

			bool operator < (timer_request& right);
		} TIMER_REQUEST_T;

		typedef std::list< TIMER_REQUEST_T > TimerRequestList;

	public:
		TimerThread();
		virtual ~TimerThread();

		void AddTimerRequest(WorkRequest* preq, int seconds);

	protected:

		virtual void DoTimerRequest(WorkRequest* preq);

	private:

		void Run();

	private:
		Mutex				m_Mutex;
		TimerRequestList	m_TimerRequestList;
	};
};
