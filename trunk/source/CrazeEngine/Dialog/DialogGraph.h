#pragma once
#include "CrazeEngine.h"

namespace Craze
{
	struct Node
	{
		std::string m_Text;
		std::string m_ScriptAction;
	};

	namespace DialogGraph
	{
	public:
		typedef std::vector<std::string> OptionList;

		OptionList GetAvailableOptions() const;
		void ChooseOption(unsigned int optionNumber);
	private:
		OptionList m_AvailableOptions;
	}
}