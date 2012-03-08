#pragma once
#ifndef CRAZE__SETTINGS__MANAGER__H__
#define CRAZE__SETTINGS__MANAGER__H__

#include <map>
#include "Setting.h"

namespace Craze
{
	namespace Settings
	{
		typedef std::map<std::string, Setting> SettingMap;
		class SettingManager
		{
		public:
			void AddFloat(std::string name, float value);
			void AddString(std::string name, std::string value);
			void AddInt(std::string name, int value);

			void AddSetting(std::string name, const Setting& setting);

			Setting GetSetting(std::string name);

			bool ImportSettingsFromFile();

			void Clear();

			~SettingManager() { Clear(); }
		};
	}
}

#endif /*CRAZE__SETTINGS__MANAGER__H__*/