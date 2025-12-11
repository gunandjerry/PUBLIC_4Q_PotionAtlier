#pragma once
#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include <map>
#include <Utility/AssimpUtility.h>
#include <functional>
#include <queue>
#include <Manager/InputManager.h>

class GameObject;
class ShaderNodeEditor;
class Scene
{
	friend class SceneManager;
	friend class GameObjectFactory;
	using InstanceID = unsigned int;
public:
	Scene();
	virtual void Awake() {}
	virtual ~Scene();
protected:
	virtual void ImGUIRender() {}

private:
	static void ImGUIBegineDraw();
	std::queue<std::function<void()>> ImGUIPopupQue;
	static void ImGUIEndDraw();

public:
	bool UseImGUI = false;
	void SetDragEvent(bool value);
public:
	const wchar_t* GetSceneName() { return sceneName.c_str(); }

private:
	std::wstring sceneName;

private:
	std::vector<std::shared_ptr<GameObject>> objectList;
	std::vector<std::weak_ptr<GameObject>>   dontdestroyonloadList;

	/*로드된 씬 관리용 맵*/
	std::unordered_map<std::wstring, std::vector<std::weak_ptr<GameObject>>> loadScenesMap;

	/*노드 에디터 관리용*/
	std::unordered_map<std::string, std::unique_ptr<ShaderNodeEditor>> nodeEditorMap;
	std::vector<std::string> nodeEditorEraseVec;
private:
	//Update
	void FixedUpdate();
	void PhysicsUpdate(float fixed_delta_time);
	void Update();
	void LateUpdate();
public:
	bool update_physics_scene{ true };
private:

	//Render
	void Render();

public:
	//기즈모용
	inline static struct ImGuizmoSetting
	{
#ifdef _EDITOR
		bool UseImGuizmo = true;
#else
		bool UseImGuizmo = false;
#endif
		GameObject* SelectObjectHelp = nullptr;
		GameObject* SelectObject = nullptr;
		int operation = 7;		//ImGuizmo::OPERATION::TRANSLATE
		int mode = 1;			//ImGuizmo::WORLD
		struct 
		{
			KeyboardKeys TRANSLATE	= KeyboardKeys::W;
			KeyboardKeys ROTATE		= KeyboardKeys::E;
			KeyboardKeys SCALE		= KeyboardKeys::R;
			KeyboardKeys UNIVERSAL	= KeyboardKeys::T;
			KeyboardKeys MODE		= KeyboardKeys::X;
		}KeySetting;	
	}
	GuizmoSetting;
	void ImGuizmoDraw();
	//에디터 관련
#ifdef _EDITOR
	inline static struct EngineEditorSetting
	{
		friend class Scene;
		friend class D3D11_GameApp;
		EngineEditorSetting();
		~EngineEditorSetting();

		void UpdateEditorCamera(DefferdRenderer& renderer);

		void PlayScene();
		void PauseScene();
		void StopScene();
		bool IsPlay() const { return isPlay; }
		bool IsPause() const { return isPause; }

		void AddLogMessage(const char* message);

		bool DrawSelectCameraFrustum = true;
	private:
		bool isPlay = false;
		class Camera* mainCam = nullptr;
		std::unique_ptr<class CameraObject> editorCamera;
		class CameraMoveHelper* editorMoveHelper = nullptr;
		Camera* editorCam = nullptr;

		bool isPause = false;
		bool drawObjectBounds = false;
		bool showLogs = false;
		std::vector<std::string> logMessages;
	}EditorSetting;
#endif

	ShaderNodeEditor* MakeShaderNodeEditor(const char* path);
	ShaderNodeEditor* MakeShaderNodeEditor(const wchar_t* path);
	void EraseShaderNodeEditor(const char* path);
	void EraseShaderNodeEditor(const wchar_t* path);
};
