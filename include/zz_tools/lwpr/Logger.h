#pragma once

#include "LWPRType.h"
#include "Buffer.h"
#include <string>
#include <stdio.h>

#define logger ZZTools::Logger::GetInstance()

#define LTRACE __FILE__, __LINE__

#define DEBUG_FUNCTION() ZZTools::LogFunction __LogFunction(LTRACE, __FUNCTION__)

namespace ZZTools
{
	typedef int LOGGER_LEVEL_T;

	const int LOGGER_LEVEL_NONE = 0;		// ����ӡ��־
	const int LOGGER_LEVEL_FATAL = 1;
	const int LOGGER_LEVEL_ERROR = 2;
	const int LOGGER_LEVEL_WARN = 3;
	const int LOGGER_LEVEL_INFO = 4;
	const int LOGGER_LEVEL_DEBUG = 5;
	const int LOGGER_LEVEL_TRACE = 6;
	const int LOGGER_LEVEL_MAX = 7;

	const int LOG_DEFAULT_BUF_SIZE = 1024;

	/**
	 *
	 */
	typedef struct logger_option
	{
		std::string		strLogFileName;
		LOGGER_LEVEL_T	nLogLevel;
		bool			bPrintFile;
		bool			bPrintConsole;
		bool			bPrintWhere;

		logger_option();
	} LOGGER_OPTION_T;

	/**
	 * ��־ģ����������Ƿ����������֤��ӡ����־������
	 */
	class Logger
	{
		Logger();
		~Logger();

	public:
		static Logger* GetInstance();

		/**
		 * ��ʼ����־ģ��
		 */
		bool Initialize(const LOGGER_OPTION_T& opt);

		void trace(const char* file, int line, const std::string& message);
		void trace(const char* file, int line, const char* stringFormat, ...);

		void debug(const char* file, int line, const std::string& message);
		void debug(const char* file, int line, const char* stringFormat, ...);

		void info(const char* file, int line, const std::string& message);
		void info(const char* file, int line, const char* stringFormat, ...);

		void warn(const char* file, int line, const std::string& message);
		void warn(const char* file, int line, const char* stringFormat, ...);

		void error(const char* file, int line, const std::string& message);
		void error(const char* file, int line, const char* stringFormat, ...);

		void fatal(const char* file, int line, const std::string& message);
		void fatal(const char* file, int line, const char* stringFormat, ...);

	private:

		inline bool IsPriorityEnabled(LOGGER_LEVEL_T level);

		inline void FillFrontLogLine(const char* file, int line, LOGGER_LEVEL_T level, Buffer& buf);

		inline void DoLog(const char* content, int size);

		inline std::string FixFileString(const char* file);

	private:
		int m_nfd;
		LOGGER_OPTION_T m_LoggerOption;
	};

	class LogFunction
	{
	public:
		LogFunction(const char* file, int line, const char* pfun);
		~LogFunction();

	private:
		std::string m_strFileName;
		std::string m_strFunctionName;
		int m_nLine;
	};
};
