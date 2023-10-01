#pragma once

#include "LWPRType.h"
#include <list>

namespace ZZTools
{
	/**
	 * �򵥽��̳ع��ܣ��ܴ����̶���Ŀ�Ľ��̣���ά�������������н��������˳���������
	 */
	class SimpleProcessPool
	{
	public:
		/**
		 * ���캯��
		 */
		SimpleProcessPool();

		/**
		 * ��������
		 */
		virtual ~SimpleProcessPool();

		/**
		 * ����������������
		 */
		void SetProcessNum(int process_num);

		/**
		 * ��ʼ����
		 */
		void Run(int argc, char** argv);

		/**
		 * �ӽ���ִ����
		 */
		virtual int DoChildWork(int argc, char** argv) = 0;

	private:
		int m_nProcessNum;
		std::list < int > m_ProcessList;
	};
};
