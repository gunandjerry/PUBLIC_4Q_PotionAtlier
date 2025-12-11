#pragma once
#include <list>
#include <string>
#include <thread>
#include <mutex>
#include <sstream>
#include <sqlite_modern_cpp.h>

namespace simple
{
	class Logger
	{
		struct LogData
		{
			std::string TimeStamp;
			std::string LogType;
			std::string Message;
		};
	public:
		Logger(const char* LogFolder = "Logs");
		~Logger();

		void WriteLog(const char* type, const char* message);
		inline size_t GetLogCount() { return logList.size(); }
	private:
		std::string GetLogFileName();
		void WriteLogFunc();
		
	private:
		const std::string logDir;
		std::string LogFileName;
		LogData logData;
		std::stringstream ss_time;
		sqlite::database db;
	private:
		std::list<LogData> logList;
		std::mutex listMutex;
		std::thread workerThreads;
		std::condition_variable cv;
		std::mutex cv_mutex;
		bool isEndLogger = false;
	};
}