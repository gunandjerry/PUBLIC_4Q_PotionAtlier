#include "Scene.h"
#include <typeinfo>
#include <algorithm>
#include <framework.h>
#include <ImGuizmo/ImGuizmo.h>
#include <imgui_internal.h>
#include <NodeEditor/NodeEditor.h>
#include <Utility/IconsFontAwesome4.h>
#include <DrawCommand.h>
#include <Core/EventManager.h>
#include <Manager/InputManager.h>

Scene::Scene()
{
	constexpr unsigned int ReserveSize = 100000;
	objectList.reserve(ReserveSize);
	dontdestroyonloadList.reserve(ReserveSize);
	Transform::reserveUpdateList(ReserveSize);
	SetDragEvent(false);

#ifdef _EDITOR
	EditorSetting.editorCamera.reset(new CameraObject);
	EditorSetting.editorCamera->name = L"Editor Camera";
	EditorSetting.editorCamera->instanceID = instanceIDManager.getUniqueID();
	EditorSetting.editorMoveHelper = &EditorSetting.editorCamera->AddComponent<CameraMoveHelper>();
	EditorSetting.editorCam = EditorSetting.editorCamera->GetCamera();
	EditorSetting.editorCam->Far = 10000.f;
#endif
}

Scene::~Scene()
{
	nodeEditorMap.clear();
	hlslManager.ClearSharingShader();
	Resource::ClearResourceManagers();
	SamplerState::CleanupSamplers();
}

void Scene::FixedUpdate()
{
#ifdef _EDITOR
	if (EditorSetting.IsPlay())
#endif // _EDITOR
	for (auto& obj : objectList)
	{
		if (obj && obj->GetWorldActive())
		{		
			obj->FixedUpdate();  
		}			
	}
}

void Scene::PhysicsUpdate(float fixed_delta_time)
{
#ifdef _EDITOR
	if (!Scene::EditorSetting.IsPlay())
		return;
#endif // DEBUG

	if (update_physics_scene == false) return;


	for (auto& obj : objectList)
	{
		if (obj && obj->GetWorldActive())
		{
			if (PhysicsActor* actor = obj->GetPhysicsActor(); actor != nullptr)
			{
				actor->SetPxActorTransformByObjectTransform(&obj->transform);
			}
		}
	}

	PhysicsManager::GetInstance().GetPhysicsScene()->Update(fixed_delta_time);

	for (auto& obj : objectList)
	{
		if (obj && obj->GetWorldActive())
		{
			if (PhysicsActor* actor = obj->GetPhysicsActor(); actor != nullptr)
			{
				actor->SetObjectTransformByPxActorTransform(&obj->transform);
			}
		}
	}
}

void Scene::Update()
{
	for (auto& obj : objectList)
	{
#ifdef _EDITOR
		if (!Scene::EditorSetting.IsPlay())
			break;
#endif // DEBUG
		if (obj)
		{
			obj->Start();
		}
	}

	for (auto& obj : objectList)
	{
		if (obj && obj->GetWorldActive())
			obj->Update();
	}
	SoundSystem::GetInstance().UpdateFMODSystem();
}

void Scene::LateUpdate()
{
	for (auto& obj : objectList)
	{
		if (obj && obj->GetWorldActive())
			obj->LateUpdate();
	}
}

void Scene::Render()
{
	DefferdRenderer& renderer = D3D11_GameApp::GetRenderer();
#ifdef _EDITOR
	if (inputManager.input.IsKeyDown(KeyboardKeys::F10))
		UseImGUI = !UseImGUI;

	if(!EditorSetting.IsPlay())
		EditorSetting.UpdateEditorCamera(renderer);
#endif
	Transform::UpdateMatrix();
	Transform::ClearUpdateList();
	if (Camera::GetMainCamera())
	{
		for (auto& obj : objectList)
		{
			if (obj && obj->GetWorldActive())
			{
				obj->Render();
#ifdef _EDITOR
				if (
					EditorSetting.drawObjectBounds
					//&& obj->transform.Parent == nullptr
					)
				{
					DebugMeshDrawCommand dc;
					dc.boundingBox = obj->GetOBBToWorld();
					if (obj->transform.Parent == nullptr)
						dc.color = Vector4(0, 0.8, 0, 1);
					else
						dc.color = Vector4(0, 0, 0.8, 1);
					dc.type = EDebugMeshDraw::Box;
					renderer.AddDrawCommand(dc);
				}
#endif
			}
		}
		renderer.Render();
	}
	else
	{
		constexpr float color[4] = { 0.3, 0.3, 0.3, 1.f };
		D3D11_GameApp::GetIDXGI().ClearBackBuffer(color);
	}
	if (UseImGUI)
	{
		ImGUIBegineDraw();
		if constexpr (IS_EDITOR)
		{
			ImGuizmoDraw();
		}
		ImGUIRender();
		if (!ImGUIPopupQue.empty())
			ImGUIPopupQue.front()();
		ImGUIEndDraw();
	}
	D3D11_GameApp::Present();
}

ShaderNodeEditor* Scene::MakeShaderNodeEditor(const char* path)
{
	auto find = nodeEditorMap.find(path);
	if (find != nodeEditorMap.end())
	{
		return find->second.get();
	}
	nodeEditorMap[path] = std::make_unique<ShaderNodeEditor>(path);
	return nodeEditorMap[path].get();
}

ShaderNodeEditor* Scene::MakeShaderNodeEditor(const wchar_t* path)
{
	return MakeShaderNodeEditor(utfConvert::wstring_to_utf8(path).c_str());
}

void Scene::EraseShaderNodeEditor(const char* path)
{
	auto find = nodeEditorMap.find(path);
	if (find != nodeEditorMap.end())
	{
		nodeEditorEraseVec.emplace_back(path);
	}
}

void Scene::EraseShaderNodeEditor(const wchar_t* path)
{
	EraseShaderNodeEditor(utfConvert::wstring_to_utf8(path).c_str());
}

void Scene::ImGUIBegineDraw()
{
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
}

void Scene::ImGuizmoDraw()
{
	using namespace DirectX::SimpleMath;
	using namespace DirectX;

	if (!GuizmoSetting.UseImGuizmo)
		return;

#ifdef _EDITOR
	for (auto& [key, editor] : nodeEditorMap)
	{
		editor->Update();
	}

	if (!nodeEditorEraseVec.empty())
	{
		for (auto& erasePath : nodeEditorEraseVec)
		{
			nodeEditorMap.erase(erasePath);
		}
		nodeEditorEraseVec.clear();
	}

	InputManager::Input& Input = inputManager.input;
	const bool isPlayMode = Scene::EditorSetting.IsPlay();
	const bool isLeftCtrl = Input.IsKey(KeyboardKeys::LeftCtrl);
	if (GuizmoSetting.UseImGuizmo)
	{
		ImGuiIO& io = ImGui::GetIO();
		ImGuiContext& imGuiContext = *ImGui::GetCurrentContext();
		if (Camera* mainCamera = Scene::EditorSetting.editorCam)
		{
			ImGuizmo::BeginFrame();
			const SIZE& clientSize = D3D11_GameApp::GetClientSize();
			float width = (float)clientSize.cx;
			float height = (float)clientSize.cy;
			ImGuizmo::SetRect(0, 0, width, height);
			const Matrix& cameraVM = mainCamera->GetVM();
			const Matrix& cameraPM = mainCamera->GetPM();

			bool isNotRightClickHELD = !Input.IsKey(MouseKeys::rightButton);
			bool isHoveredWindow = imGuiContext.HoveredWindow != nullptr;
			if (Input.IsKeyDown(MouseKeys::leftButton))
				if (!ImGuizmo::IsOver() && !ImGuizmo::IsUsing() && isNotRightClickHELD && !isHoveredWindow)
				{				
					const Mouse::State& state = Input.GetMouseState();
					if (state.positionMode == Mouse::Mode::MODE_ABSOLUTE)
					{
						Ray ray = mainCamera->ScreenPointToRay(state.x, state.y);
						ObjectList list = sceneManager.GetObjectList();			
						if (isLeftCtrl)
						{
							std::erase_if(list, [](GameObject* object)
								{
									return object->Bounds.Extents.x <= Mathf::Epsilon || object->Bounds.Extents.y <= Mathf::Epsilon;
								});
						}
						else
						{
							std::erase_if(list, [](GameObject* object)
								{
									return !!object->transform.Parent || object->Bounds.Extents.x <= Mathf::Epsilon || object->Bounds.Extents.y <= Mathf::Epsilon;
								});
						}
						std::sort(list.begin(), list.end(), [&mainCamera](GameObject* a, GameObject* b)
							{
								auto fastDistance = [](const Vector3& p1, const Vector3& p2) {
									Vector3 diff = p1 - p2;
									return diff.x * diff.x + diff.y * diff.y + diff.z * diff.z;
									};
								float disA = fastDistance(mainCamera->transform.position, a->transform.position);
								float disB = fastDistance(mainCamera->transform.position, b->transform.position);
								return disA < disB;
							});
						float Dist = 0;
						for (auto& obj : list)
						{
							if (obj->GetWorldActive() && obj->GetOBBToWorld().Intersects(ray.position, ray.direction, Dist))
							{
								if (obj->transform.RootParent)
									GuizmoSetting.SelectObject = isLeftCtrl ? obj : &obj->transform.RootParent->gameObject;
								else
									GuizmoSetting.SelectObject = obj;
								break;
							}
						}
					}
				}

			if (ImGUIPopupQue.empty() && isNotRightClickHELD)
			{
				//Button Setting				
				if (Input.IsKeyDown(GuizmoSetting.KeySetting.TRANSLATE))
				{
					GuizmoSetting.operation = ImGuizmo::OPERATION::TRANSLATE;
				}
				else if (Input.IsKeyDown(GuizmoSetting.KeySetting.ROTATE))
				{
					GuizmoSetting.operation = ImGuizmo::OPERATION::ROTATE;
				}
				else if (Input.IsKeyDown(GuizmoSetting.KeySetting.SCALE))
				{
					GuizmoSetting.operation = ImGuizmo::OPERATION::SCALE;
				}
				else if (Input.IsKeyDown(GuizmoSetting.KeySetting.UNIVERSAL))
				{
					GuizmoSetting.operation = ImGuizmo::OPERATION::UNIVERSAL;
				}
				else if (Input.IsKeyDown(GuizmoSetting.KeySetting.MODE))
				{
					GuizmoSetting.mode = (GuizmoSetting.mode != ImGuizmo::MODE::WORLD) ? ImGuizmo::MODE::WORLD : ImGuizmo::MODE::LOCAL;
				}

				static float dummyMatrix[16]{};
				if (Input.IsKeyDown(KeyboardKeys::Escape))
				{
					ImGuizmo::Enable(false);
					ImGuizmo::Enable(true);
					ImGuizmo::Manipulate(dummyMatrix, dummyMatrix, ImGuizmo::OPERATION(0), ImGuizmo::WORLD, dummyMatrix);
					GuizmoSetting.SelectObject = nullptr;
				}
				else if (GuizmoSetting.SelectObject && Input.IsKeyDown(KeyboardKeys::Delete) && nodeEditorMap.empty())
				{
					if (GuizmoSetting.SelectObject != static_cast<GameObject*>(&mainCamera->gameObject))
					{
						ImGuizmo::Enable(false);
						ImGuizmo::Enable(true);
						ImGuizmo::Manipulate(dummyMatrix, dummyMatrix, ImGuizmo::OPERATION(0), ImGuizmo::WORLD, dummyMatrix);
						GameObject::Destroy(GuizmoSetting.SelectObject);
						GuizmoSetting.SelectObject = nullptr;
					}
				}
			}

			if (GuizmoSetting.SelectObject)
			{
				ImGuizmo::OPERATION operation = (ImGuizmo::OPERATION)GuizmoSetting.operation;
				ImGuizmo::MODE mode = (ImGuizmo::MODE)GuizmoSetting.mode;
				//Draw Guizmo

				if (!isPlayMode)
				{
					const float* cameraView = reinterpret_cast<const float*>(&cameraVM);
					const float* cameraProjection = reinterpret_cast<const float*>(&cameraPM);

					bool isParent = GuizmoSetting.SelectObject->transform.RootParent != nullptr;

					Matrix objMatrix = GuizmoSetting.SelectObject->transform.GetWM();

					float* pMatrix = reinterpret_cast<float*>(&objMatrix);
					ImGuizmo::Manipulate(cameraView, cameraProjection, operation, mode, pMatrix);

					Transform* parent = GuizmoSetting.SelectObject->transform.Parent;
					if (parent)
					{
						objMatrix *= parent->GetIWM();
					}

					Vector3 postion, scale;
					Quaternion rotation;
					
					objMatrix.Decompose(scale, rotation, postion);
					if (parent)
					{
						if ((GuizmoSetting.SelectObject->transform.localPosition - postion).Length() > Mathf::AngleEpsilon)
							GuizmoSetting.SelectObject->transform.localPosition = postion;
						if ((GuizmoSetting.SelectObject->transform.localScale - scale).Length() > Mathf::AngleEpsilon)
							GuizmoSetting.SelectObject->transform.localScale = scale;
						if (Mathf::GetAngleDifference(rotation, GuizmoSetting.SelectObject->transform.localRotation) > Mathf::AngleEpsilon)
							GuizmoSetting.SelectObject->transform.localRotation = rotation;
					}
					else
					{
						if ((GuizmoSetting.SelectObject->transform.position - postion).Length() > Mathf::AngleEpsilon)
							GuizmoSetting.SelectObject->transform.position = postion;
						if ((GuizmoSetting.SelectObject->transform.scale - scale).Length() > Mathf::AngleEpsilon)
							GuizmoSetting.SelectObject->transform.scale = scale;
						if (Mathf::GetAngleDifference(rotation, GuizmoSetting.SelectObject->transform.rotation) > Mathf::AngleEpsilon)
							GuizmoSetting.SelectObject->transform.rotation = rotation;
					}
					
				}
			}
		}

		//Editer draw
		const bool pause = EditorSetting.IsPause();
		static bool editorDirectionalLights = false;
		if (editorDirectionalLights)
		{
			ImGui::Begin("Directional Lights", &editorDirectionalLights, ImGuiWindowFlags_AlwaysAutoResize);
			ImGui::EditDirectionalLights(&D3D11_GameApp::GetRenderer());
			ImGui::End();
		}
		if (ImGui::BeginMainMenuBar())
		{
			if (ImGui::Button(ICON_FA_PLAY " Play"))
			{
				Scene::EditorSetting.PlayScene();
			}
			if (pause)
			{
				constexpr ImVec4 darkBlue = ImVec4(20 / 255.0f, 30 / 255.0f, 100 / 255.0f, 1.0f);        // 버튼 기본 색
				constexpr ImVec4 darkBlueHovered = ImVec4(30 / 255.0f, 50 / 255.0f, 130 / 255.0f, 1.0f); // 호버 시 
				constexpr ImVec4 darkBlueActive = ImVec4(10 / 255.0f, 20 / 255.0f, 80 / 255.0f, 1.0f);   // 클릭 시 
				ImGui::PushStyleColor(ImGuiCol_Button, darkBlue);
				ImGui::PushStyleColor(ImGuiCol_ButtonHovered, darkBlueHovered);
				ImGui::PushStyleColor(ImGuiCol_ButtonActive, darkBlueActive);
			}
			if (ImGui::Button(ICON_FA_PAUSE " Pause"))
			{
				Scene::EditorSetting.PauseScene();
			}
			if (pause)
			{
				ImGui::PopStyleColor();
				ImGui::PopStyleColor();
				ImGui::PopStyleColor();
			}
			if (ImGui::Button(ICON_FA_STOP " Stop"))
			{
				Scene::EditorSetting.StopScene();
			}
			if (!isPlayMode)
			{
				if (ImGui::BeginMenu(ICON_FA_CUBES " Game Object"))
				{
					if (ImGui::MenuItem(" " ICON_FA_FOLDER_OPEN " Open GameObject"))
					{
						ImGui::ShowOpenGameObjectPopup();
					}
					if (ImGui::MenuItem(" " ICON_FA_FLOPPY_O "  Save As GameObject"))
					{
						if (GuizmoSetting.SelectObject)
						{
							ImGui::ShowSaveAsGameObjectPopup(GuizmoSetting.SelectObject);
						}
					}
					if (ImGui::BeginMenu(" " ICON_FA_FILE "  New GameObject"))
					{
						for (const auto& [name, folder] : gameObjectFactory.GetGameObjectParentFolderMap())
						{
							if (ImGui::BeginMenu(folder.c_str()))
							{
								if (ImGui::MenuItem((name.c_str() + 6)))
								{
									std::wstring typeName = utfConvert::utf8_to_wstring(name);
									const wchar_t* cwstr = typeName.c_str();
									cwstr += 6; //class 제외
									gameObjectFactory.NewGameObjectToKey(name.c_str())(cwstr);
								}
								ImGui::EndMenu();
							}
						}
						ImGui::EndMenu();
					}
					ImGui::EndMenu();
				}
				if (ImGui::BeginMenu(ICON_FA_FILM " Scene"))
				{
					if (ImGui::MenuItem(" " ICON_FA_FOLDER_OPEN " Load Scene"))
					{
						ImGui::ShowLoadScenePopup();
					}
					if (ImGui::MenuItem(" " ICON_FA_FILE "  Save Scene"))
					{
						std::wstring savePath = sceneManager.GetLastLoadScenePath();
						if (savePath.empty())
						{
							savePath = WinUtility::GetSaveAsFilePath(L"Scene", sceneManager.GetActiveScene()->GetSceneName());
						}
						if (!savePath.empty())
						{
							sceneManager.SaveScene(savePath.c_str(), true);
						}
					}
					if (ImGui::MenuItem(" " ICON_FA_FLOPPY_O "  Save As Scene"))
					{
						ImGui::ShowSaveAsScenePopup(this);
					}
					if (ImGui::MenuItem(" " ICON_FA_PLUS_SQUARE "  Add Scene"))
					{
						ImGui::ShowAddScenePopup();
					}
					if (ImGui::MenuItem(" " ICON_FA_MINUS_SQUARE "  Sub Scene"))
					{
						ImGui::ShowSubScenePopup();
					}
					ImGui::EndMenu();
				}
				if (ImGui::BeginMenu(ICON_FA_LIGHTBULB_O " Light"))
				{
					if (ImGui::MenuItem(ICON_FA_SUN_O " Directional"))
					{
						editorDirectionalLights = !editorDirectionalLights;
					}
					ImGui::EndMenu();
				}
				if (ImGui::BeginMenu(ICON_FA_CAMERA " Editor Camera"))
				{
					ImGui::Checkbox("Draw Camera Frustum", &Scene::EditorSetting.DrawSelectCameraFrustum);
					ImGui::Checkbox("Draw Object Bounding Box", &EditorSetting.drawObjectBounds);
					ImGui::EditCamera("Editor Camera", Scene::EditorSetting.editorCam, Scene::EditorSetting.editorMoveHelper);
					if (ImGui::Button("Sync with Editor Camera"))
					{
						if (Camera* mainCam = Camera::GetMainCamera())
						{
							mainCam->SyncCamera(Scene::EditorSetting.editorCam);
						}
					}
					ImGui::SameLine();
					if (ImGui::Button("Sync to Main Camera"))
					{
						if (Camera* mainCam = Camera::GetMainCamera())
						{
							Scene::EditorSetting.editorCam->SyncCamera(mainCam);
						}
					}
					ImGui::EndMenu();
				}
				if (ImGui::BeginMenu(ICON_FA_DOWNLOAD " Import"))
				{
					if (ImGui::MenuItem(ICON_FA_FEMALE "  Import FBX"))
					{
						std::wstring path = WinUtility::GetOpenFilePath(L"fbx");
						if (!path.empty())
						{
							D3D11_GameApp::DefaultFBXDragDropHandler(path.c_str());
						}
					}
					ImGui::EndMenu();
				}
				if (ImGui::BeginMenu(ICON_FA_UPLOAD " Export"))
				{
					if (ImGui::MenuItem(ICON_FA_CIRCLE_THIN"  Export Material Nodes"))
					{
						MeshRender::ExportMaterialAll();
					}
					if (ImGui::MenuItem(ICON_FA_HOURGLASS_HALF "  Bake Env"))
					{

					}
					ImGui::EndMenu();
				}
				if (ImGui::BeginMenu(ICON_FA_COG " System"))
				{
					if (ImGui::MenuItem(ICON_FA_POWER_OFF "  Exit"))
					{
						//ImGui::ShowSaveAsScenePopup(this);
						D3D11_GameApp::GameEnd();
					}
					if (ImGui::MenuItem(ICON_FA_REFRESH "  Shader Reload"))
					{
						MeshRender::ReloadShaderAll();
					}
					if (ImGui::MenuItem(ICON_FA_FILES_O "  Logs"))
					{
						EditorSetting.showLogs = !EditorSetting.showLogs;
					}
					ImGui::EndMenu();
				}
			}
			ImGui::EndMainMenuBar();
		}

		//Inspector
		if (GuizmoSetting.SelectObject)
		{
			static bool nextAppearing = false;
			bool isAppearing = nextAppearing;
			constexpr float damp = 20.f;
			ImGui::PushID("Inspector");
			ImGui::Begin("Inspector", nullptr, ImGuiWindowFlags_AlwaysAutoResize); // 사이즈 자동
			{
				if (isAppearing)
				{
					ImVec2 windowSize = ImGui::GetWindowSize();
					ImVec2 windowPos = ImVec2(io.DisplaySize.x - windowSize.x - damp, damp * 2.0f);
					ImGui::SetWindowPos(windowPos);
				}
				nextAppearing = ImGui::IsWindowAppearing();
				ImGui::Text("%s", typeid(*GuizmoSetting.SelectObject).name());
				ImGui::SameLine();
				ImGui::Text("%s", GuizmoSetting.SelectObject->GetNameToString().c_str());
				ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.2f, 0.3f, 0.4f, 1.0f)); // 배경색
				ImGui::BeginChild("tags", ImVec2(0, 0), ImGuiChildFlags_AlwaysAutoResize | ImGuiChildFlags_AutoResizeX | ImGuiChildFlags_AutoResizeY);
				{
					if (ImGui::Button("Add tag"))
					{
						static char newTag[256] = "";
						auto RenamePopup = []()
							{
								ImGui::OpenPopup("Add Tag");  // 팝업을 열기
								if (ImGui::BeginPopupModal("Add Tag", 0, ImGuiWindowFlags_AlwaysAutoResize))
								{
									ImGui::Text("Enter new Tag:");
									ImGui::InputText("New Name", newTag, IM_ARRAYSIZE(newTag));
									if (ImGui::Button("OK"))
									{
										GuizmoSetting.SelectObject->SetTag(utfConvert::utf8_to_wstring(newTag));
										sceneManager.PopImGuiPopupFunc();
									}
									if (ImGui::Button("Cancel"))
									{
										sceneManager.PopImGuiPopupFunc();
									}
									ImGui::EndPopup();
								}
							};
						sceneManager.PushImGuiPopupFunc(RenamePopup);
					}
					int id = 0;
					for (auto& tag : GuizmoSetting.SelectObject->tagSet)
					{
						ImGui::Text(utfConvert::wstring_to_utf8(tag).c_str());
						ImGui::SameLine();
						ImGui::PushID(id++);
						if (ImGui::Button("remove"))
						{
							GuizmoSetting.SelectObject->UnsetTag(tag);
							ImGui::PopID();
							break;
						}
						ImGui::PopID();
					}

				}
				ImGui::EndChild();
				ImGui::PopStyleColor();
				bool active = GuizmoSetting.SelectObject->Active;
				if(ImGui::Checkbox("Active", &active))
					GuizmoSetting.SelectObject->Active = active;

				ImGui::EditTransform(GuizmoSetting.SelectObject);
				GuizmoSetting.SelectObject->InspectorImguiDraw();
			}
			ImGui::End();
			ImGui::PopID();
		}
		
		//logs
		if (EditorSetting.showLogs)
		{
			ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.1f, 0.1f, 0.2f, 1.0f));    // 배경 색
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));        // 텍스트 색
			ImGui::SetNextWindowSizeConstraints(ImVec2(400, 50), ImVec2(FLT_MAX, 500));
			if (ImGui::Begin("logs", &EditorSetting.showLogs))
			{
				for (const auto& message : EditorSetting.logMessages)
				{
					ImGui::TextUnformatted(message.c_str());
				}
				ImGui::SetScrollHereY(1.0f);// 스크롤 자동 이동
			}
			if (!EditorSetting.showLogs)
			{
				constexpr float damp = 10.f;
				ImGui::SetWindowPos(ImVec2(damp, io.DisplaySize.y - ImGui::GetWindowHeight() - damp));
			}
			ImGui::End();
			ImGui::PopStyleColor(2);  
			ImGui::ResetGlobalID();
		}
	}
#endif //_EDITOR
}

void Scene::ImGUIEndDraw()
{
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
	ImGui::ResetGlobalID();
}

void Scene::SetDragEvent(bool value)
{
	DragAcceptFiles(D3D11_GameApp::GetHWND(), value);
}

#ifdef _EDITOR
Scene::EngineEditorSetting::EngineEditorSetting() = default;
Scene::EngineEditorSetting::~EngineEditorSetting() = default;

void Scene::EngineEditorSetting::UpdateEditorCamera(DefferdRenderer& renderer)
{
	bool isPlay = EditorSetting.IsPlay();
	if (!isPlay && EditorSetting.editorCamera.get())
	{
		EditorSetting.mainCam = Camera::GetMainCamera();

		EditorSetting.editorMoveHelper->UpdateMovemont();
		EditorSetting.editorCam->UpdateCameraMatrix();
		renderer.SetCameraMatrix(EditorSetting.editorCam->GetIVM());
		if (EditorSetting.editorCam->isPerspective)
			renderer.SetPerspectiveProjection(EditorSetting.editorCam->FOV * Mathf::Deg2Rad, EditorSetting.editorCam->Near, EditorSetting.editorCam->Far);
		else
			renderer.SetOrthographicProjection(EditorSetting.editorCam->Near, EditorSetting.editorCam->Far);
	}
	if (!isPlay && EditorSetting.DrawSelectCameraFrustum)
	{
		if (CameraObject* selectCamera = dynamic_cast<CameraObject*>(GuizmoSetting.SelectObject))
		{
			if (Camera* select = selectCamera->GetCamera())
			{
				select->UpdateCameraMatrix();
				DebugMeshDrawCommand cameraFrustum;
				cameraFrustum.color = Vector4(0, 1.0, 0, 1);
				if (select->isPerspective)
				{
					cameraFrustum.type = EDebugMeshDraw::Type::Frustom;
					BoundingFrustum::CreateFromMatrix(cameraFrustum.frustom, select->GetPM());
					cameraFrustum.frustom.Transform(cameraFrustum.frustom, select->GetIVM());
				}
				else
				{
					cameraFrustum.type = EDebugMeshDraw::Type::Box;
					const SIZE& size = D3D11_GameApp::GetClientSize();
					float width = size.cx;
					float height = size.cy;
					float depth = select->Far - select->Near;
					cameraFrustum.boundingBox = BoundingOrientedBox(Vector3(0, 0, depth * 0.5f + select->Near), Vector3(width * 0.5f, height * 0.5f, depth * 0.5f), Vector4(0, 0, 0, 1.f));
					cameraFrustum.boundingBox.Transform(cameraFrustum.boundingBox, select->GetIVM());
				}
				renderer.AddDrawCommand(cameraFrustum);
			}
		}
	}
}

void Scene::EngineEditorSetting::PlayScene()
{
	if (isPlay)
		return;

	if (!EditorSetting.mainCam)
		return;

	EditorSetting.mainCam->SetMainCamera();

	std::wstring savePath = sceneManager.GetLastLoadScenePath();
	if (savePath.empty())
	{
		savePath = WinUtility::GetSaveAsFilePath(L"Scene", sceneManager.GetActiveScene()->GetSceneName());
		if (savePath.empty())
		{
			return;
		}
		sceneManager.SaveScene(savePath.c_str());
		sceneManager.LoadScene(savePath.c_str());
	}
	else
	{
		sceneManager.SaveScene(savePath.c_str(), true);
	}
	isPlay = true;
	ImGui::GetIO().ConfigFlags &= ~ImGuiConfigFlags_NavEnableKeyboard;
}

void Scene::EngineEditorSetting::PauseScene()
{
	using namespace TimeSystem;
	if (!isPlay)
		return;

	if (isPause)
	{
		Time.timeScale = 1.f;
		isPause = false;
	}	
	else
	{
		Time.timeScale = 0.f;
		isPause = true;
	}	
}

void Scene::EngineEditorSetting::StopScene()
{
	if (!isPlay)
		return;

	sceneManager.LoadScene(sceneManager.GetLastLoadScenePath().c_str());
	isPlay = false;

	ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
}

void Scene::EngineEditorSetting::AddLogMessage(const char* message)
{
	static std::stringstream time_stream;
	time_stream.clear();
	time_stream.str("");

	auto now = std::chrono::system_clock::now();
	std::time_t now_time = std::chrono::system_clock::to_time_t(now);
	std::tm time_tm;
	localtime_s(&time_tm, &now_time);

	time_stream << std::put_time(&time_tm, "[%H:%M:%S] "); 
	time_stream << message;

	logMessages.push_back(time_stream.str());
	if (logMessages.size() > 100)
		logMessages.erase(logMessages.begin());

	showLogs = true;
}
#endif // _EDITOR