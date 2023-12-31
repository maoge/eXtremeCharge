#include "ConfigProperty.h"
#include "Buffer.h"
#include "StringUtil.h"
#include "ProcessUtil.h"
#include <unistd.h>
#include <string>
#include <vector>
#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <algorithm>
#include <string.h>

using namespace std;

namespace ZZTools
{
	ConfigProperty::ConfigProperty()
	{
	}

	ConfigProperty::~ConfigProperty()
	{
	}

	void ConfigProperty::ConfigInit(int argc, char* argv[])
	{
		std::string strCfgFile;
		std::vector< std::string > cmdDefs;

		if(argc > 0)
		{
			m_vtOtherArgs.push_back(argv[0]);

			char ch;
			while((ch = getopt(argc, argv, "c:D:")) != -1)
			{
				switch(ch)
				{
				case 'c':
					strCfgFile = optarg;
					break;
				case 'D':
					cmdDefs.push_back(optarg);
					break;
					// ������Чѡ��
				case '?':
					break;
				default:
					if(optarg)
					{
						m_vtOtherArgs.push_back(optarg);
					}
					break;
				}
			}
		}

		// ���建����
		Buffer bufTmp(1024);
		Buffer bufLine(1024);
		Buffer bufName(256);
		Buffer bufValue(1024);

		// ����Ҫ�������ļ�
		std::vector< std::string > cfgFiles;
		std::string str = GetPreConfigFile();
		if(str.length() > 0)
		{
			const char* p = getenv(str.c_str());
			if(p != NULL && strlen(p) > 0)
			{
				cfgFiles.push_back(p);
			}
		}

		if(strCfgFile.length() > 0)
		{
			cfgFiles.push_back(strCfgFile);
		}

		// �Ƚ����ļ�
		for(size_t k = 0; k < cfgFiles.size(); k++)
		{
			FILE* fp = fopen(cfgFiles[k].c_str(), "r");
			if(fp != NULL)
			{
				while(!feof(fp))
				{
					bufTmp.Reset();
					bufLine.Reset();
					bufName.Reset();
					bufValue.Reset();
					if(fgets(bufTmp.Inout(), bufTmp.Capacity() - 1, fp) != NULL)
					{
						// ȥ��ע��
						int code = sscanf(bufTmp.Inout(), "%[^#\n]", bufLine.Inout());
						if(code != 1)
						{
							continue;
						}

						// �ָ��name/value
						code = sscanf(bufLine.Inout(), "%[^ =]%*[ =]%[^$]", bufName.Inout(), bufValue.Inout());
						if(code != 2)
						{
							continue;
						}

						// ��������õ�name/value
						m_OptionProperty[StringUtil::TrimAll(bufName.Inout())] = StringUtil::TrimAll(bufValue.Inout());
					}
				}

				fclose(fp);
			}
			else
			{
				throw LWPR_FILE_NOT_EXIST(EXCEPTION_THREAD_TRACE, 0, "fopen error");
			}
		}

		// �ٽ���������
		for(size_t i = 0; i < cmdDefs.size(); i++)
		{
			bufName.Reset();
			bufValue.Reset();

			// �ָ��name/value
			int code = sscanf(cmdDefs[i].c_str(), "%[^ =]%*[ =]%[^$]", bufName.Inout(), bufValue.Inout());
			if(code != 2)
			{
				continue;
			}

			// ��������õ�name/value

			m_OptionProperty[StringUtil::TrimAll(bufName.Inout())] = StringUtil::TrimAll(bufValue.Inout());
		}

		DoPropConstruct();
	}

	void ConfigProperty::ConfigInit()
	{
		std::vector<std::string> cmd;
		ProcessUtil::GetProcessCmdline(cmd);

		char **argv = new char*[cmd.size() + 1];

		argv[cmd.size()] = NULL;

		for (size_t i = 0; i < cmd.size(); i++)
		{
			argv[i] = (char *)cmd[i].c_str();
		}

		ConfigInit(cmd.size(), argv);

		delete[] argv;
	}

	void ConfigProperty::SetProperty(const OptionProperty& prop)
	{
		OptionProperty::const_iterator it = prop.begin();
		for(; it != prop.end(); it++)
		{
			if(m_OptionProperty.find(it->first) == m_OptionProperty.end())
			{
				m_OptionProperty[it->first] = it->second;
			}
		}

		DoPropConstruct();
	}

	std::string ConfigProperty::GetPropertyLabel(const char* name)
	{
		assert(NULL != name);

		std::string tmp = name;
		std::replace(tmp.begin(), tmp.end(), '.', '_');
		return StringUtil::ToUpper(tmp);
	}

	bool ConfigProperty::GetPropertyValue(const char* name, bool default_value)
	{
		assert(name != NULL);

		std::string tmp = GetPropertyValue(name, default_value ? "true" : "false");

		return (tmp == "true");
	}

	int ConfigProperty::GetPropertyValue(const char* name, int default_value)
	{
		assert(name != NULL);

		char buf[64] = {0};
		sprintf(buf, "%d", default_value);

		std::string tmp = GetPropertyValue(name, buf);

		int result = atoi(tmp.c_str());

		return result;
	}

	std::string ConfigProperty::GetPropertyValue(const char* name, const std::string& default_value)
	{
		return GetPropertyValue(name, default_value.c_str());
	}

	std::string ConfigProperty::GetPropertyValue(const char* name, const char* default_value)
	{
		assert(name != NULL);
		assert(default_value != NULL);

		std::string result = default_value;

		// �ȴӻ���������ȡ
		const char* p = getenv(GetPropertyLabel(name).c_str());
		if(p != NULL)
		{
			result = p;
		}

		// �ٴ������ļ�/��������ȥȡ
		OptionProperty::iterator it = m_OptionProperty.find(name);
		if(it != m_OptionProperty.end())
		{
			result = it->second;
		}

		return result;
	}

	std::string ConfigProperty::GetPreConfigFile()
	{
		return "";
	}

	std::vector<std::string>& ConfigProperty::GetOtherArgs()
	{
		return m_vtOtherArgs;
	}

	std::string ConfigProperty::GetProgramName()
	{
		assert(m_vtOtherArgs.size() > 0);
		return m_vtOtherArgs[0];
	}
};
