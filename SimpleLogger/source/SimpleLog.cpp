#include "../inc/SimpleLog.h"
#include <iostream>
#include <fstream>
#include <chrono>
#include <ctime>
#include <filesystem>

simple::Logger::Logger(const char* LogFolder) :
	workerThreads(&Logger::WriteLogFunc, this),
	logDir(LogFolder),
	db(GetLogFileName())
{
	ss_time.str().reserve(30);

	// 로그 테이블 생성
	db << "CREATE TABLE IF NOT EXISTS Logs ("
		"ID INTEGER PRIMARY KEY AUTOINCREMENT, "
		"Timestamp TEXT NOT NULL, "
		"LogType TEXT NOT NULL, "
		"Message TEXT NOT NULL"
		");";
}

simple::Logger::~Logger()
{
	isEndLogger = true;   //로거 종료 활성화
	cv.notify_one();	  //스레드 정리.
	workerThreads.join(); //스레드 종료 대기
}

void simple::Logger::WriteLog(const char* type, const char* message)
{
	//stringstream 초기화
	ss_time.str("");
	ss_time.clear();

	// 현재 시간 얻기
	auto now = std::chrono::system_clock::now();
	std::time_t now_c = std::chrono::system_clock::to_time_t(now);
	std::tm local_time;
	localtime_s(&local_time, &now_c);
	ss_time << std::put_time(&local_time, "%Y-%m-%d %H:%M:%S");

	//로그 작성
	logData.TimeStamp = ss_time.str();
	logData.LogType = type;
	logData.Message = message;

	//로그 목록 생성. 생성시에만 락
	listMutex.lock();
	logList.emplace_back(logData);
	listMutex.unlock();

	cv.notify_one(); //작성 시작
}

std::string simple::Logger::GetLogFileName()
{
	LogFileName = logDir;
	if (LogFileName.back() != '/' && LogFileName.back() != '\\')
	{
		LogFileName += '/';
	}
	LogFileName += "logs.db";

	//폴더 존재 여부 확인 후 생성.
	if (!std::filesystem::exists(logDir.c_str()))
	{
		std::filesystem::create_directories(logDir.c_str());
	}

	return LogFileName;
}

void simple::Logger::WriteLogFunc()
{
	while (!isEndLogger)
	{
		{
			std::unique_lock lock(cv_mutex);
			cv.wait(lock, [this] {return !logList.empty() || isEndLogger; });
		}
		std::ofstream outLog(LogFileName.c_str(), std::ios::app);

		while (!logList.empty())
		{
			if (!outLog)
			{
				break; //재시도
			}

			LogData& front = logList.front();
			db << "INSERT INTO Logs (Timestamp, LogType, Message) VALUES (?, ?, ?);"
				<< front.TimeStamp
				<< front.LogType
				<< front.Message;

			//삭제시에만 락
			listMutex.lock();
			logList.pop_front();
			listMutex.unlock();
		}		
		outLog.close();
	}

	//최종 로그 작성
	std::ofstream outLog(LogFileName.c_str(), std::ios::app);
	while(!logList.empty())
	{		
		LogData& front = logList.front();
		db << "INSERT INTO Logs (Timestamp, LogType, Message) VALUES (?, ?, ?);"
			<< front.TimeStamp
			<< front.LogType
			<< front.Message;
		logList.pop_front();		
	}
	outLog.close();
	return;
}
