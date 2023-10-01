#pragma once

#include "LWPRType.h"
#include "Buffer.h"

namespace ZZTools
{
	class StringBuffer : public Buffer
	{
	public:
		/**
		 * ���캯��
		 */
		StringBuffer();
		StringBuffer(int capacity = BUFFER_DEFAULT_CAPACITY);
		StringBuffer(char* str);
		StringBuffer(const char* str);
		StringBuffer(const Buffer& buf);
		StringBuffer(const StringBuffer& strbuf);

		/**
		 * ��ֵ����������
		 */
		void operator= (char* str);
		void operator= (const char* str);
		void operator= (const Buffer& buf);
		void operator= (const StringBuffer& strbuf);

		/**
		 * ���ַ���ĩ��׷��
		 */
		char* Append(const char* str);

		/**
		 * �����ַ���ָ��
		 */
		char* str() const;

		/**
		 * �����ַ����ĳ���
		 */
		int Length() const;

		/**
		 * ȥ��ǰ��ո��Ʊ��������з�
		 */
		char* Trim();

		/**
		 * ����ch���ַ����е�һ�γ��ֵ�λ��
		 */
		int FindFirstOf(char ch, int index = 0) const;

	private:
	};

	DECLAREVAR(StringBuffer);
};
