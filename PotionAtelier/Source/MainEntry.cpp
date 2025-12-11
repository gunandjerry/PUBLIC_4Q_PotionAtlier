#include "StartApp.h"
#include <EditorScene.h>
#include "Scene/MainGameScene.h"
#include <Utility/SQLiteLogger.h>

int APIENTRY wWinMain(
    _In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR lpCmdLine,
    _In_ int nCmdShow
)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);


    StartApp app;
    app.Initialize(hInstance);
#ifdef _EDITOR
    sceneManager.LoadScene<EditorScene>();
#else
    sceneManager.LoadScene<MainGameScene>();
#endif // _EDITOR
    app.Run();
    app.Uninitialize();

    return 0;
}