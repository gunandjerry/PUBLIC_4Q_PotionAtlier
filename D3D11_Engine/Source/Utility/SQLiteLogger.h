#pragma once
#include <SimpleLog.h>

namespace simple
{
	class Logger;
}

class SQLiteLogger
{
public:
	static void EditorLog(const char* logType, const char* message);
	static void GameLog(const char* logType, const char* message);

	static void DestroyLogger();
};

