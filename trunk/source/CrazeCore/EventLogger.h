#pragma once
#include <fstream>

/*Macros for printing out messages more conveniently*/
#define LOGMSG(message, type) Craze::EventLogger::log( message , type )
#define LOG_INFO(message) LOGMSG( message , Craze::LOGTYPE_INFORMATION)
#define LOG_WARNING(message) LOGMSG( message , Craze::LOGTYPE_WARNING)
#define LOG_ERROR(message) LOGMSG( message , Craze::LOGTYPE_ERROR)
#define LOG_CRITICAL(message) LOGMSG( message , Craze::LOGTYPE_CRITICAL)


namespace Craze
{
	enum LOG_TYPE
	{
		//General information, ie. graphics engine initialized
		LOGTYPE_INFORMATION,
		//Warning, ie. device was lost
		LOGTYPE_WARNING,
		//Errors, ie. an invalid game state was attempt to be set 
		LOGTYPE_ERROR,
		//Critical, ie. the rendering device could not be initialized.
		LOGTYPE_CRITICAL,
		//None, logs no events at all.
		LOGTYPE_NONE,
	};

	/**
	EventLogger
	A class that is used to print messages to a file specified on initialization.
	Messages can have some different levels in order to be filtered.
	*/
	class EventLogger
	{
	public:

		/**
		Initialize
		Initializes the event logger and opens the file.
		@param fileName the file to print out the log to
		@param filter the minimum log level that should be printed to the file
		@return indicates success of the initialization.
		*/
		static bool initialize(const wchar_t* fileName, LOG_TYPE filter = LOGTYPE_WARNING);

		/**
		Log
		Prints the message to the file if the type is higher or equal than the
		current filter level. The message is preceeded with the log type and 
		a time stamp.
		@param message the message to print out
		@param logType the log level
		*/
		static void log(std::string message, LOG_TYPE logType);

		/**
		Print
		Prints the message to the file without any time stamp or log type
		@param the message to be printed
		*/
		static void print(std::string message);

		/**
		Shutdown
		Closes the event logger
		*/
		static void shutdown();

	protected:
		static bool m_initialized;
		static std::ofstream m_outputFile;
		static LOG_TYPE m_logFilter;
	};
}