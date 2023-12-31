#include "OutputStream.h"
#include <arpa/inet.h>
#include <string.h>


namespace ZZTools
{
	OutputStream::OutputStream(int capacity)
		: m_vBuffer(new Buffer(capacity)),
		  m_nOffSet(0)
	{
	}

	void OutputStream::Reset()
	{
		m_nOffSet = 0;
	}

	void OutputStream::WriteBOOL(BOOL value)
	{
		m_vBuffer->Size(m_nOffSet + sizeof(value));
		memcpy(m_vBuffer->Inout() + m_nOffSet, &value, sizeof(value));
		m_nOffSet += sizeof(value);
	}

	void OutputStream::WriteFloat(FP32 value)
	{
		m_vBuffer->Size(m_nOffSet + sizeof(value));
		memcpy(m_vBuffer->Inout() + m_nOffSet, &value, sizeof(value));
		m_nOffSet += sizeof(value);
	}

	void OutputStream::WriteDouble(FP64 value)
	{
		m_vBuffer->Size(m_nOffSet + sizeof(value));
		memcpy(m_vBuffer->Inout() + m_nOffSet, &value, sizeof(value));
		m_nOffSet += sizeof(value);
	}

	void OutputStream::WriteString(const char* value)
	{
		assert(value != NULL);

		INT32 nLen = strlen(value);
		WriteINT32(nLen);

		m_vBuffer->Size(m_nOffSet + nLen);
		memcpy(m_vBuffer->Inout() + m_nOffSet, value, nLen);
		m_nOffSet += nLen;
	}

	void OutputStream::WriteString(const std::string& value)
	{
		WriteString(value.c_str());
	}

	void OutputStream::WriteAny(const char* buf, int size)
	{
		assert(buf != NULL);

		WriteINT32(size);

		m_vBuffer->Size(m_nOffSet + size);
		memcpy(m_vBuffer->Inout() + m_nOffSet, buf, size);
		m_nOffSet += size;
	}

	void OutputStream::WriteArray(const void* buf, int size)
	{
		WriteAny((const char*)buf, size);
	}

	void OutputStream::WriteINT8(INT8 value)
	{
		m_vBuffer->Size(m_nOffSet + sizeof(value));
		memcpy(m_vBuffer->Inout() + m_nOffSet, &value, sizeof(value));
		m_nOffSet += sizeof(value);
	}

	void OutputStream::WriteUINT8(UINT8 value)
	{
		m_vBuffer->Size(m_nOffSet + sizeof(value));
		memcpy(m_vBuffer->Inout() + m_nOffSet, &value, sizeof(value));
		m_nOffSet += sizeof(value);
	}

	void OutputStream::WriteINT16(INT16 value)
	{
		value = htons(value);
		m_vBuffer->Size(m_nOffSet + sizeof(value));
		memcpy(m_vBuffer->Inout() + m_nOffSet, &value, sizeof(value));
		m_nOffSet += sizeof(value);
	}

	void OutputStream::WriteUINT16(UINT16 value)
	{
		value = htons(value);
		m_vBuffer->Size(m_nOffSet + sizeof(value));
		memcpy(m_vBuffer->Inout() + m_nOffSet, &value, sizeof(value));
		m_nOffSet += sizeof(value);
	}

	void OutputStream::WriteINT32(INT32 value)
	{
		value = htonl(value);
		m_vBuffer->Size(m_nOffSet + sizeof(value));
		memcpy(m_vBuffer->Inout() + m_nOffSet, &value, sizeof(value));
		m_nOffSet += sizeof(value);
	}

	void OutputStream::WriteUINT32(UINT32 value)
	{
		value = htonl(value);
		m_vBuffer->Size(m_nOffSet + sizeof(value));
		memcpy(m_vBuffer->Inout() + m_nOffSet, &value, sizeof(value));
		m_nOffSet += sizeof(value);
	}

	void OutputStream::WriteINT64(INT64 value)
	{
		value = Utility::htonl64(value);
		m_vBuffer->Size(m_nOffSet + sizeof(value));
		memcpy(m_vBuffer->Inout() + m_nOffSet, &value, sizeof(value));
		m_nOffSet += sizeof(value);
	}

	void OutputStream::WriteUINT64(UINT64 value)
	{
		value = Utility::htonl64(value);
		m_vBuffer->Size(m_nOffSet + sizeof(value));
		memcpy(m_vBuffer->Inout() + m_nOffSet, &value, sizeof(value));
		m_nOffSet += sizeof(value);
	}

	Buffer_ptr OutputStream::GetBuffer()
	{
		m_vBuffer->Size(m_nOffSet);
		return Buffer_var::Duplicate(m_vBuffer);
	}

	char * OutputStream::GetStream()
	{
		return m_vBuffer->Inout();
	}

	int OutputStream::GetStreamLength()
	{
		return m_nOffSet;
	}
};
