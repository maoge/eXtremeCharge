#include "Buffer.h"
#include "StringBuffer.h"
#include <cassert>
#include <cstring>
#include <cstdlib>

namespace ZZTools
{
	Buffer::Buffer(int capacity)
		: m_capacity(capacity), m_size(0), m_pbuf(NULL)
	{
		assert(capacity > 0);

		void *p = malloc(capacity);
		if(p == NULL)
		{
			throw LWPR_MEM_NOT_ENOUGH();
		}

		memset(p, 0, capacity);
		m_pbuf = (char *)p;
	}

	Buffer::Buffer(const Buffer& rhs)
		: m_capacity(0), m_size(0), m_pbuf(NULL)
	{
		*this = rhs;
	}

	Buffer& Buffer::operator=(const Buffer& rhs)
	{
		if(this == &rhs)
		{
			return *this;
		}

		m_size = rhs.m_size;
		Realloc(rhs.m_capacity);
		memcpy(m_pbuf, rhs.m_pbuf, rhs.m_capacity);
		return *this;
	}

	char *Buffer::Inout() const
	{
		return m_pbuf;
	}

	int Buffer::Capacity() const
	{
		return m_capacity;
	}

	int Buffer::Capacity(int capacity)
	{
		this->Realloc(capacity);
		return capacity;
	}

	int Buffer::Size(int sz)
	{
		m_size = sz;

		if(sz > 0)
		{
			Capacity(sz);
		}

		return m_size;
	}

	int Buffer::Size() const
	{
		return m_size;
	}

	Buffer::~Buffer()
	{
		if(m_pbuf != NULL)
		{
			free(m_pbuf);
		}
	}

	void Buffer::Realloc(int size)
	{
		assert(size > 0);

		if(size <= m_capacity) return;

		void *p = NULL;

		if(m_pbuf == NULL)
		{
			p = malloc(size);
		}
		else
		{
			p = realloc(m_pbuf, size);
		}

		if(p == NULL)
		{
			throw LWPR_MEM_NOT_ENOUGH();
		}

		m_pbuf = (char *)p;

		// ��ʼ���·�����ڴ�
		memset(m_pbuf + m_capacity, 0, size - m_capacity);

		m_capacity = size;
	}

	Buffer* Buffer::CopyValueOf(int offset, int count)
	{
		assert(offset >= 0 && offset < m_capacity && count <= (m_capacity - offset));

		Buffer_var vBuf = new Buffer(count + 1);
		memcpy(vBuf->Inout(), m_pbuf + offset, count);
		vBuf->Size(count);
		return vBuf.retn();
	}

	StringBuffer* Buffer::ToString(int offset, int count)
	{
		Buffer_var vBuf = CopyValueOf(offset, count);
		StringBuffer_var vStrBuf = new StringBuffer(*vBuf);
		return vStrBuf.retn();
	}

	void Buffer::Reset()
	{
		memset(m_pbuf, 0, m_capacity);
		m_size = 0;
	}
};
