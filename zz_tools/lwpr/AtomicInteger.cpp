#include "AtomicInteger.h"

namespace ZZTools
{

	AtomicInteger::AtomicInteger(int initialValue)
		: m_nValue(initialValue)
	{
	}

	AtomicInteger::~AtomicInteger()
	{
	}

	int AtomicInteger::Get() const
	{
		return m_nValue;
	}

	void AtomicInteger::Set(int newValue)
	{
		m_nValue = newValue;
	}

	int AtomicGet()
	{
		return AtomicGet();
	}

	bool AtomicInteger::CompareAndSet(int expect, int update)
	{
		return CompareAndSet(&m_nValue, expect, update);
	}

	int AtomicInteger::GetAndIncrement()
	{
		return GetAndIncrement(&m_nValue);
	}

	int AtomicInteger::GetAndDecrement()
	{
		return GetAndDecrement(&m_nValue);
	}

	int AtomicInteger::GetAndAdd(int delta)
	{
		return GetAndAdd(&m_nValue, delta);
	}

	int AtomicInteger::AddAndGet(int delta)
	{
		return AddAndGet(&m_nValue, delta);
	}
	
	int AtomicInteger::GetAndSub(int delta)
	{
		return GetAndSub(&m_nValue, delta);
	}
	
	int AtomicInteger::SubAndGet(int delta)
	{
		return SubAndGet(&m_nValue, delta);
	}

	int AtomicInteger::IncrementAndGet()
	{
		return IncrementAndGet(&m_nValue);
	}

	int AtomicInteger::DecrementAndGet()
	{
		return DecrementAndGet(&m_nValue);
	}

	int AtomicInteger::operator = (int newValue)
	{
		this->Set(newValue);
		return newValue;
	}

	int AtomicInteger::operator++()
	{
		return this->IncrementAndGet();
	}

	int AtomicInteger::operator++(int)
	{
		return this->GetAndIncrement();
	}

	int AtomicInteger::operator--()
	{
		return this->DecrementAndGet();
	}

	int AtomicInteger::operator--(int)
	{
		return this->GetAndDecrement();
	}

	AtomicInteger::operator int () const
	{
		return this->Get();
	}

	int AtomicInteger::AtomicGet(volatile int *ptr)
	{
		__sync_synchronize ();
		return *ptr;
	}

	bool AtomicInteger::CompareAndSet(volatile int *ptr, int expect, int update)
	{
		return __sync_bool_compare_and_swap(ptr, expect, update);
	}

	int AtomicInteger::GetAndIncrement(volatile int *ptr)
	{
		return __sync_fetch_and_add(ptr, 1);
	}

	int AtomicInteger::GetAndDecrement(volatile int *ptr)
	{
		return __sync_fetch_and_sub(ptr, 1);
	}

	int AtomicInteger::GetAndAdd(volatile int *ptr, int delta)
	{
		return __sync_fetch_and_add(ptr, delta);
	}

	int AtomicInteger::AddAndGet(volatile int *ptr, int delta)
	{
		return __sync_add_and_fetch(ptr, delta);
	}
	
	int AtomicInteger::GetAndSub(volatile int *ptr, int delta)
	{
		return __sync_fetch_and_sub(ptr, delta);
	}

	int AtomicInteger::SubAndGet(volatile int *ptr, int delta)
	{
		return __sync_sub_and_fetch(ptr, delta);
	}

	int AtomicInteger::IncrementAndGet(volatile int *ptr)
	{
		return __sync_add_and_fetch(ptr, 1);
	}

	int AtomicInteger::DecrementAndGet(volatile int *ptr)
	{
		return __sync_sub_and_fetch(ptr, 1);
	}
};
