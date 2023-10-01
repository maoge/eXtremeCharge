#pragma once

#include "LWPRType.h"
#include "Exception.h"
#include <map>
#include <string>

namespace ZZTools
{
	typedef std::map< std::string, std::string > OptionProperty;

	/**
	 * �������ȼ��� ���� < �������� < �����ļ� < �����в���
	 */
	class ConfigProperty
	{
	public:
		ConfigProperty();
		virtual ~ConfigProperty();

		void ConfigInit(int argc, char* argv[]);

		void ConfigInit();

		void SetProperty(const OptionProperty& prop);

		virtual void DoPropConstruct() = 0;

		virtual std::string GetPreConfigFile();

		static std::string GetPropertyLabel(const char* name);

		std::vector<std::string>& GetOtherArgs();

		std::string GetProgramName();

	protected:
		int GetPropertyValue(const char* name, int default_value);
		bool GetPropertyValue(const char* name, bool default_value);
		std::string GetPropertyValue(const char* name, const std::string& default_value);
		std::string GetPropertyValue(const char* name, const char* default_value);

	protected:
		OptionProperty				m_OptionProperty;
		std::vector<std::string>	m_vtOtherArgs;		// ����������argv[0]
	};

	DEFINE_EXCEPTION(LWPR_FILE_NOT_EXIST);
};
