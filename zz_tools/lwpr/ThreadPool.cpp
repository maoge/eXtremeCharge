#include "ThreadPool.h"
#include "Synchronized.h"

namespace ZZTools
{

	//----------------------------------------------------------------------------------
	// class thread_pool_option
	//----------------------------------------------------------------------------------
	thread_pool_option::thread_pool_option()
		: nMinThread(10),
		  nMaxThread(300),
		  nMaxIdlThread(150)
	{
	}

	//----------------------------------------------------------------------------------
	// class ThreadPool
	//----------------------------------------------------------------------------------
	ThreadPool::ThreadPool(THREAD_POOL_OPTION_T& opt)
		: m_Option(opt),
		  m_pThreadManage(NULL),
		  m_nIdleThread(0),
		  m_nTotalThread(0)
	{
		// ���������߳�
		for(int i = 0; i < m_Option.nMinThread; i++)
		{
			ThreadWorker* pWorker = new ThreadWorker(*this);
			m_ThreadWorkerList.push_back(pWorker);
			pWorker->Start();
		}

		// ���������߳�
		m_pThreadManage = new ThreadManage(*this);
		m_pThreadManage->Start();
	}

	ThreadPool::~ThreadPool()
	{
	}

	void ThreadPool::AddRequest(WorkRequest* preq)
	{
		if(preq != NULL)
		{
			Synchronized syn(m_Mutex);
			m_WorkRequestList.push_back(preq);
		}

		Notify();
	}

	void ThreadPool::Destroy()
	{
		m_pThreadManage->StopRunning();
	}

	void ThreadPool::ClearDeadThread()
	{
		ThreadWorkerList::iterator it = m_ThreadWorkerList.begin();
		for(; it != m_ThreadWorkerList.end();)
		{
			if(!(*it)->IsContinue())
			{
				ThreadWorker* p = *it;
				p->DecRef();
				it = m_ThreadWorkerList.erase(it);
			}
			else
			{
				it++;
			}
		}
	}

	bool ThreadPool::IsNeededToCreateThread()
	{
		return (m_nIdleThread < 3 && m_nTotalThread < m_Option.nMaxThread);
	}

	bool ThreadPool::IsNeededToDestroyThread()
	{
		return (m_nIdleThread > m_Option.nMaxIdlThread && m_nTotalThread > m_Option.nMaxIdlThread);
	}

	//----------------------------------------------------------------------------------
	// class ThreadWorker
	//----------------------------------------------------------------------------------
	ThreadPool::ThreadWorker::ThreadWorker(ThreadPool& pool)
		: m_ThreadPool(pool)
	{

	}

	ThreadPool::ThreadWorker::~ThreadWorker()
	{

	}

	void ThreadPool::ThreadWorker::Run()
	{
		// �����߳�����
		AutomaticCount autoTotalThread(m_ThreadPool.m_nTotalThread);

		while(IsContinue())
		{
			WorkRequest* pReq = NULL;

			{
				// �����̼߳���
				AutomaticCount autoIdleThread(m_ThreadPool.m_nIdleThread);
				m_ThreadPool.Wait();

				// ȡ����
				Synchronized syn(m_ThreadPool.m_Mutex);
				ThreadPool::WorkRequestList::iterator it = m_ThreadPool.m_WorkRequestList.begin();
				if(it != m_ThreadPool.m_WorkRequestList.end())
				{
					pReq = *it;
					m_ThreadPool.m_WorkRequestList.pop_front();
				}
			}

			// ִ������
			if(pReq != NULL)
			{
				pReq->DoWork();
			}

			// �����̳߳�
			if(m_ThreadPool.IsNeededToCreateThread() || m_ThreadPool.IsNeededToDestroyThread())
			{
				m_ThreadPool.m_pThreadManage->Notify();
			}
		}
	}

	//----------------------------------------------------------------------------------
	// class ThreadManage
	//----------------------------------------------------------------------------------
	ThreadPool::ThreadManage::ThreadManage(ThreadPool& pool)
		: m_ThreadPool(pool)
	{
	}

	ThreadPool::ThreadManage::~ThreadManage()
	{
	}

	void ThreadPool::ThreadManage::Run()
	{
		while(IsContinue())
		{
			Wait(10);

			// ������ȥ���߳�
			m_ThreadPool.ClearDeadThread();

			// �����߳�
			if(m_ThreadPool.IsNeededToCreateThread())
			{
				ThreadPool::ThreadWorker* pWorker = new ThreadPool::ThreadWorker(m_ThreadPool);
				m_ThreadPool.m_ThreadWorkerList.push_back(pWorker);
				pWorker->Start();
			}

			// �����߳�
			if(m_ThreadPool.IsNeededToDestroyThread())
			{
				ThreadWorkerList::iterator it = m_ThreadPool.m_ThreadWorkerList.begin();
				if(it != m_ThreadPool.m_ThreadWorkerList.end())
				{
					(*it)->StopRunning();
				}
			}
		}

		// ���̳߳����߳��˳�
		ThreadWorkerList::iterator it = m_ThreadPool.m_ThreadWorkerList.begin();
		for(; it != m_ThreadPool.m_ThreadWorkerList.end(); it++)
		{
			(*it)->StopRunning();
		}

		// �����߳���Դ
		for(int i = 0; i < 5 && m_ThreadPool.m_ThreadWorkerList.size() > 0; i++)
		{
			m_ThreadPool.ClearDeadThread();
			Thread::Sleep(1);
		}
	}
};
