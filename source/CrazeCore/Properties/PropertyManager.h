#pragma once
#include "CrazeEngine.h"

#include "Property.h"


namespace Craze
{
	namespace Properties
	{
		typedef std::map<std::string, Setting> PropertyMap;
		class PropertiesManager
		{
		public:
			void AddFloat(std::string name, float value);
			void AddString(std::string name, std::string value);
			void AddInt(std::string name, int value);

			void AddProperty(std::string name, const Setting& setting);

			Setting GetProperty(std::string name);

			bool ImportSettingsFromFile();

			void Clear();

			~PropertiesManager() { Clear(); }
		};
	}
}