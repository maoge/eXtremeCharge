#pragma once

#include "LWPRType.h"
#include "Exception.h"
#include "Buffer.h"
#include "StringBuffer.h"
#include <cstdio>

namespace ZZTools
{
	class CFile
	{
	public:

		/**
		 * ���캯��
		 */
		CFile(const char* fname, const char* mode);

		/**
		 * ��������
		 */
		virtual ~CFile();

		/**
		 * ��ȡ�ļ��Ĵ�С
		 */
		int FSIZE();

		/**
		 * ���ļ�����ȫ�����ص��ڴ�
		 */
		Buffer_ptr LoadFileContent();

		/**
		 * д���ݵ��ļ�
		 */
		void FWRITE(const Buffer& buf);

		/**
		 * д�ı����ļ�
		 */
		void FWRITE(const StringBuffer& strbuf);

	private:
		FILE *m_fp;
	};

	class AutoCloseFile
	{
	public:
		AutoCloseFile(FILE* fp);
		~AutoCloseFile();
	private:
		FILE* m_fp;
	};

#define AUTO_CLOSE_FILE(fp)	AutoCloseFile __AutoCloseFile(fp)

	DEFINE_EXCEPTION(FileNotFoundException);
	DEFINE_EXCEPTION(IOException);
};
