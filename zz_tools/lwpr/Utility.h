#pragma once

#include "LWPRType.h"
#include "AtomicInteger.h"
#include <string>

namespace ZZTools
{
#define SWAP_BYTE_ORDER16(A) \
	((((UINT16)(A) & 0xff00) >> 8) | \
	(((UINT16)(A) & 0x00ff) << 8))

#define SWAP_BYTE_ORDER32(A) \
	((((UINT32)(A) & 0xff000000) >> 24) | \
	(((UINT32)(A) & 0x00ff0000) >> 8 ) | \
	(((UINT32)(A) & 0x0000ff00) << 8 ) | \
	(((UINT32)(A) & 0x000000ff) << 24))

	UINT64 SWAP_BYTE_ORDER64(UINT64 value);

	typedef struct time_precision_t
	{
		UINT32 nSeconds;	// ��
		UINT32 nUSeconds;	// ΢��
	} TIME_PRECISION_T;

	class Utility
	{
		static PID_T _nPID;
		static TIME_PRECISION_T _nBootTime;

	public:
		static const char* ErrnoName(int num);
		static std::string ExceptionInfo(const char *file,
		                                 int line,
		                                 const char *fun,
		                                 int errnum,
		                                 const char *excptname,
		                                 const char *dsc);
		static PID_T GetPid();

		/**
		 * ��ȡ��ǰʱ�̵ľ�ȷʱ��
		 */
		static TIME_PRECISION_T GetTimePrecision();

		/**
		 * ��ȡ��ǰ���̵�����ʱ��
		 */
		static TIME_PRECISION_T GetBootTime();

		/**
		 * ��ӡ��ֵ�ԣ�0-��=�� 1-����=
		 */
		static std::string PrintCfgNameValue(const char* name, int value, int type = 0);

		/**
		 * ��ӡ��ֵ�ԣ�0-��=�� 1-����=
		 */
		static std::string PrintCfgNameValue(const char* name, const std::string& value, int type = 0);

		/**
		 * ��num����Size�ı������ҽ��>=num
		 * �˺���һ�������ֽڶ��룬�������ڴ�ҳ����ȷ���
		 */
		static long RoundUp(long num, long size);

		/**
		 * ���ص�ǰϵͳ���� YYYYMMDD
		 */
		static std::string DateNow();

		/**
		 * ���ص�ǰϵͳ���� YYYY/MM/DD HH:MM:SS
		 */
		static std::string DateTimeNow();
		
		/**
		 * ��ȡ��ǰϵͳʱ���
		 */
		 static long GetMillisecondsFromLocal();

		/**
		 * ��ȡһ��Ψһ�ַ�������ʱ��Ϊ��׼
		 */
		static std::string GetRandomString();

		/**
		 * ��ȡ�Ե�ǰʱ��ΪΨһ�ַ�����ID
		 * ����ֵ�� 16λ�����ַ���
		 */
		static std::string GetTimeID();

		/**
		 * ע���źŴ���
		 */
		typedef void (*PF_SIGHANDLER_T)(int);
		static void RegisterSignal(int signum, PF_SIGHANDLER_T handler);

		/**
		 * �źŴ�������
		 */
		static void DefaultSignalHandler(int signum);

		/**
		 * host long 64 to network
		 */
		static UINT64 htonl64(UINT64 host);

		/**
		* network to host long 64
		 */
		static UINT64 ntohl64(UINT64 net);

		/**
		 * License�Ƿ���Ч
		 */
		static bool HasLicenseEffect(int days);

		static UINT16 IntegerToLocalByte16(UINT16 value, bool bigendian);

		static UINT32 IntegerToLocalByte32(UINT32 value, bool bigendian);

		static UINT64 IntegerToLocalByte64(UINT64 value, bool bigendian);

		/**
		 * ���ֽ���ת�����׶��ĸ�ʽ
		 */
		static std::string ByteToHumanFmt(long size);

		/**
		 * ���û�������
		 */
		bool PutEnv(char* str);

		/**
		 * ���û�������
		 */
		bool PutEnv(char* name, char* value);

		/**
		 * ���ػ�������
		 */
		const char* GetEnv(char* str);
	};

	class AlarmTimer
	{
	public:
		/**
		 * ���캯��
		 */
		AlarmTimer(int timeout);

		/**
		 * ��������
		 */
		~AlarmTimer();

	private:
		int m_nLastRemainTime;
		int m_nTimeout;
	};

	class AutomaticCount
	{
	public:
		/**
		 * ���캯��
		 */
		AutomaticCount(AtomicInteger& atom);

		/**
		 * ��������
		 */
		~AutomaticCount();

	private:
		AtomicInteger& m_AtomicInteger;
	};
};
