#pragma once

#include "LWPRType.h"
#include "AtomicInteger.h"

namespace ZZTools
{
	class Object
	{
	public:
		/**
		 * �������ø�����1
		 */
		static Object* Duplicate(Object *ref);
		/**
		 * �������ø�����1������Ϊ0ʱ���ͷŶ���
		 */
		static void Release(Object *ref);

		void IncRef();
		void DecRef();

	protected:
		Object() : m_nRefCount(1) {}
		virtual ~Object();

	private:
		AtomicInteger m_nRefCount;
	};
};
