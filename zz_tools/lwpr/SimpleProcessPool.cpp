#include "SimpleProcessPool.h"
#include <sys/types.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>

namespace ZZTools
{
	SimpleProcessPool::SimpleProcessPool()
		: m_nProcessNum(0)
	{
	}

	SimpleProcessPool::~SimpleProcessPool()
	{
		std::list <int > ::iterator it = m_ProcessList.begin();

		for(; it != m_ProcessList.end(); it++)
		{
			kill(SIGKILL, *it);
		}
	}

	void SimpleProcessPool::SetProcessNum(int process_num)
	{
		m_nProcessNum = process_num;
	}

	void SimpleProcessPool::Run(int argc, char** argv)
	{
		// ��������
		for(int i = 0; i < m_nProcessNum; i++)
		{
			pid_t pid = fork();
			// child
			if(pid == 0)
			{
				exit(DoChildWork(argc, argv));
			}
			// father
			else if(pid > 0)
			{
				m_ProcessList.push_back(pid);
			}

		}

		// ����н����˳��� ������
		while(true)
		{
			pid_t pid = wait(NULL);
			if(pid > 0)
			{
				std::list <int > ::iterator it = m_ProcessList.begin();

				for(; it != m_ProcessList.end(); it++)
				{
					if(*it == pid)
					{
						m_ProcessList.erase(it);
						break;
					}
				}

				pid = fork();
				// child
				if(pid == 0)
				{
					exit(DoChildWork(argc, argv));
				}
				// father
				else if(pid > 0)
				{
					m_ProcessList.push_back(pid);
				}
			}
		}
	}
};
