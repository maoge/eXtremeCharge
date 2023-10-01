#pragma once

#include "LWPRType.h"
#include "Thread.h"

namespace ZZTools
{
	class TLSValue
	{
	public:
		virtual ~TLSValue() {};
	};

	class ThreadLocal
	{
	public:
		ThreadLocal();
		virtual ~ThreadLocal();

		TLSValue* GetValue();
		void SetValue(TLSValue* value);

	private:
		static void KeyDestructor(void* value);
		THREAD_KEY_T	m_Key;
	};
};
