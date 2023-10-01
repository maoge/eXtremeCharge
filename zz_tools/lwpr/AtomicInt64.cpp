#include "AtomicInt64.h"

namespace ZZTools
{
	AtomicInt64::AtomicInt64(INT64 initialValue)
		: m_nValue(initialValue)
	{
	}

	AtomicInt64::~AtomicInt64()
	{
	}

	INT64 AtomicInt64::Get() const
	{
		return m_nValue;
	}

	void AtomicInt64::Set(INT64 newValue)
	{
		m_nValue = newValue;
	}

	bool AtomicInt64::CompareAndSet(INT64 expect, INT64 update)
	{
		return CompareAndSet(&m_nValue, expect, update);
	}

	INT64 AtomicInt64::GetAndIncrement()
	{
		return GetAndIncrement(&m_nValue);
	}

	INT64 AtomicInt64::GetAndDecrement()
	{
		return GetAndDecrement(&m_nValue);
	}

	INT64 AtomicInt64::GetAndAdd(int delta)
	{
		return GetAndAdd(&m_nValue, delta);
	}

	INT64 AtomicInt64::AddAndGet(int delta)
	{
		return AddAndGet(&m_nValue, delta);
	}

	INT64 AtomicInt64::GetAndSub(int delta)
	{
		return GetAndSub(&m_nValue, delta);
	}

	INT64 AtomicInt64::SubAndGet(int delta)
	{
		return SubAndGet(&m_nValue, delta);
	}

	INT64 AtomicInt64::IncrementAndGet()
	{
		return IncrementAndGet(&m_nValue);
	}
	
	INT64 AtomicInt64::DecrementAndGet()
	{
		return DecrementAndGet(&m_nValue);
	}
	
	INT64 AtomicInt64::operator = (INT64 newValue)
	{
		this->Set(newValue);
		return newValue;
	}

	INT64 AtomicInt64::operator++()
	{
		return this->IncrementAndGet();
	}
	
	INT64 AtomicInt64::operator++(int)
	{
		return this->GetAndIncrement();
	}
	
	INT64 AtomicInt64::operator--()
	{
		return this->DecrementAndGet();
	}

	INT64 AtomicInt64::operator--(int)
	{
		return this->GetAndDecrement();
	}
	
	AtomicInt64::operator INT64 () const
	{
		return this->Get();
	}
	
	bool AtomicInt64::CompareAndSet(volatile INT64 *ptr, INT64 expect, INT64 update)
	{
		return __sync_bool_compare_and_swap(ptr, expect, update);
	}
	
	INT64 AtomicInt64::GetAndIncrement(volatile INT64 *ptr)
	{
		return __sync_fetch_and_add(ptr, 1);
	}
	
	INT64 AtomicInt64::GetAndDecrement(volatile INT64 *ptr)
	{
		return __sync_fetch_and_sub(ptr, 1);
	}
	
	INT64 AtomicInt64::GetAndAdd(volatile INT64 *ptr, int delta)
	{
		return __sync_fetch_and_add(ptr, delta);
	}
	
	INT64 AtomicInt64::AddAndGet(volatile INT64 *ptr, int delta)
	{
		return __sync_add_and_fetch(ptr, delta);
	}
	
	INT64 AtomicInt64::GetAndSub(volatile INT64 *ptr, int delta)
	{
		return __sync_fetch_and_sub(ptr, delta);
	}
	
	INT64 AtomicInt64::SubAndGet(volatile INT64 *ptr, int delta)
	{
		return __sync_sub_and_fetch(ptr, delta);
	}
	
	INT64 AtomicInt64::IncrementAndGet(volatile INT64 *ptr)
	{
		return __sync_add_and_fetch(ptr, 1);
	}
	
	INT64 AtomicInt64::DecrementAndGet(volatile INT64 *ptr)
	{
		return __sync_sub_and_fetch(ptr, 1);
	}

};
