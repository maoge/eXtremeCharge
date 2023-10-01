#ifndef LWPR_PROGRAM_OPTION_H__
#define LWPR_PROGRAM_OPTION_H__
#include "LWPRType.h"
#include <string>
#include <map>
#include <vector>

namespace ZZTools
{
	class ProgramOption
	{
	public:
		/**
		 * ���캯��
		 */
		ProgramOption();

		/**
		 * ��������
		 */
		virtual ~ProgramOption();

		/**
		 * ����������ѡ��
		 */
		bool ParseCmdlineOption(int argc, char **argv, const char *optstring);

		/**
		 * ѡ���Ƿ���Ч
		 */
		bool IsOptionValid(char opt);

		/**
		 * ��ȡѡ����Ӧ�Ĳ����б�
		 */
		bool GetArgumentsByOption(char opt, std::vector< std::string >& args);

		/**
		 * ��ȡ������������һ������
		 */
		const std::string& GetFirstArgument() const;

	protected:
		/**
		 * ��������ѡ�������֮��У���������Ƿ�����Ӧ������
		 */
		virtual bool CheckCmdline();

	protected:
		// ����ȷʶ���ѡ��
		std::map< char, std::vector<std::string> > m_Options;
		// ��ѡ���⣬�������õĲ���
		std::vector<std::string> m_OtherArguments;
		// ��ʶ�����Чѡ��
		std::vector<char> m_InvalidOptions;
		// �����һ����������������
		std::string m_strFirstArgument;
	};
};

#endif // end of LWPR_PROGRAM_OPTION_H__
