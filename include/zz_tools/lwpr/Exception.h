#pragma once

#include "Utility.h"
#include <cerrno>

#if defined(WIN32) || defined(LINUX)
#define EXCEPTION_TRACE   \
	__FILE__,   \
	__LINE__,   \
	__FUNCTION__, \
	errno

#define EXCEPTION_THREAD_TRACE   \
	__FILE__,   \
	__LINE__,   \
	__FUNCTION__
#endif

namespace ZZTools
{
	class Exception : public std::exception
	{
	public:
		Exception(const char *msg = "LWPRException") throw()
			: m_strExcep(msg), m_errno(errno)
		{
		}

		Exception(const char *file,
		          int line,
		          const char *fun,
		          int err_no,
		          const char *dsc = NULL) throw()
			: m_strExcep(Utility::ExceptionInfo(file, line, fun, err_no, "LWPRException", dsc).c_str()),
			  m_errno(errno)
		{
		}

		virtual const char * what() const throw()
		{
			return m_strExcep.c_str();
		}

		int GetErrno()
		{
			return (m_errno);
		}

		virtual ~Exception() throw()
		{
		}
	protected:
		std::string         m_strExcep;
		int                 m_errno;
	};

};

#define DEFINE_EXCEPTION(name) \
class name : public Exception \
{   \
public: \
	name(const char *msg = #name) throw () \
	: Exception(msg) {} \
	name(const char *file, \
	int line,   \
	const char *fun,    \
	int errnum, \
	const char *dsc = NULL) throw ()    \
	: Exception(Utility::ExceptionInfo(file, line, fun, errnum, #name, dsc).c_str()) {} \
};

namespace ZZTools
{
	DEFINE_EXCEPTION(LWPR_BAD_PARM);
	DEFINE_EXCEPTION(LWPR_NOT_IMPLEMENT);
};
