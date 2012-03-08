#include "EventLogger.h"

#include <ctime>
#include <iostream>
#include "windows.h"
#include <string>
#include <sstream>

#include "Profiling/Profiler.h"

Craze::LOG_TYPE Craze::EventLogger::m_logFilter = Craze::LOGTYPE_NONE;
bool Craze::EventLogger::m_initialized = false;
std::ofstream Craze::EventLogger::m_outputFile;

bool Craze::EventLogger::initialize(const wchar_t* fileName, Craze::LOG_TYPE logFilter)
{
	m_outputFile.open(fileName, std::ios_base::out | std::ios_base::app);

	if (!m_outputFile.is_open())
	{
		return false;
	}
	
	m_logFilter = logFilter;

	m_initialized = true;

	return true;
}

void Craze::EventLogger::log(std::string message, Craze::LOG_TYPE logType)
{
	PROFILE("EventLogger::Log");
	if (m_initialized)
	{
		char time[30];
		_strtime_s(time);

		std::stringstream output;

		if (logType >= m_logFilter)
		{

			switch (logType)
			{
				case Craze::LOGTYPE_CRITICAL:
					output << "C- ";
					break;
				case Craze::LOGTYPE_ERROR:
					output << "E- ";
					break;
				case Craze::LOGTYPE_WARNING:
					output << "W- ";
					break;
				case Craze::LOGTYPE_INFORMATION:
					output << "I- ";
					break;
			};

			if (logType == LOGTYPE_CRITICAL)
			{
				MessageBoxA(NULL, message.c_str(), "Craze Engine Error", MB_OK);
			}

			output << message << '\n';
			
			std::cout << output.str();
			m_outputFile << output.str();
		}
	}
}


void Craze::EventLogger::shutdown()
{
	m_outputFile.close();
	m_logFilter = Craze::LOGTYPE_NONE;
	m_initialized = false;
}


void Craze::EventLogger::print(std::string message)
{
	PROFILE("EventLogger::Print");
	if (m_initialized)
	{
		m_outputFile << message << std::endl;
	}
	
}