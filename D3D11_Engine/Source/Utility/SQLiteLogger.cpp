#include "SQLiteLogger.h"
#include <memory>
#include <Scene/Base/Scene.h>

using namespace simple;
static std::unique_ptr<Logger> editorLogger;
static std::unique_ptr<Logger> gameLogger;


void SQLiteLogger::EditorLog(const char* logType, const char* message)
{
#ifdef _EDITOR
	if (!editorLogger)
		editorLogger.reset(new Logger("logs/Editor/"));
	
	editorLogger->WriteLog(logType, message);
	Scene::EditorSetting.AddLogMessage(message);
#endif
}

void SQLiteLogger::GameLog(const char* logType, const char* message)
{
	if (!gameLogger)
		gameLogger.reset(new Logger("logs/Game/"));

	gameLogger->WriteLog(logType, message);
#ifdef _EDITOR
	Scene::EditorSetting.AddLogMessage(message);
#endif
}


void SQLiteLogger::DestroyLogger()
{
	editorLogger.reset();
	gameLogger.reset();
}
