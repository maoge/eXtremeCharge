#pragma once

namespace ZZTools
{

/**
 * AutoHandle��Ҫ����������̬����ľֲ���������ڴ�
 */
template <class T>
class AutoHandle
{
public:
	T *operator->() const
	{
		return m_p;
	}

	operator T*() const
	{
		return m_p;
	}

	AutoHandle() : m_p(0) {}
	AutoHandle(T *p) : m_p(p) {}
	~AutoHandle()
	{
		delete m_p;
	}

	void operator=(T *p)
	{
		m_p = p;
	}

	T *ptr()
	{
		return m_p;
	}

private:
	T   *m_p;
};


}
