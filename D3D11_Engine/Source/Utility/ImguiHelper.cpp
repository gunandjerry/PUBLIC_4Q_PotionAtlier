#include "ImguiHelper.h"
#include "Math\Mathf.h"

#include <framework.h>

#include <unordered_map>
#include <string>
#include <memory>
#include <thread>
#include <mutex>
#include <atomic>

static int g_id = 0;

void ImGui::ResetGlobalID()
{
	g_id = 0;
}

void ImGui::Button(const char* label, bool* v, const ImVec2& size)
{
	if (ImGui::Button(label, size))
	{
		*v = !(*v);
	}
}

bool ImGui::DragUInt(const char* label, uint32_t* value, float speed, uint32_t min, uint32_t max)
{
	return DragScalar(label, ImGuiDataType_U32, value, speed, &min, &max, "%u");
}

bool ImGui::DragVector2(const char* label, const Vector2* pVector, float v_speed, float v_min, float v_max, const char* format, ImGuiSliderFlags flags)
{	
	return ImGui::DragFloat3(label, (float*)pVector, v_speed, v_min, v_max, format, flags);
}

bool ImGui::DragVector3(const char* label, const Vector3* pVector, float v_speed, float v_min, float v_max, const char* format, ImGuiSliderFlags flags)
{
	return ImGui::DragFloat3(label, (float*)pVector, v_speed, v_min, v_max, format, flags);
}

bool ImGui::DragVector4(const char* label, const Vector4* pVector, float v_speed, float v_min, float v_max, const char* format, ImGuiSliderFlags flags)
{
	return ImGui::DragFloat3(label,  (float*)pVector, v_speed, v_min, v_max, format, flags);
}

bool ImGui::DragQuaternionWorld(const char* label, const Quaternion* pQuaternion, float v_speed, float v_min, float v_max, const char* format, ImGuiSliderFlags flags)
{	
	static std::unordered_map<Quaternion*, Vector3> prevEuler;
	bool isEdit = false;
	Quaternion* qu = const_cast<Quaternion*>(pQuaternion);
	Vector3 euler = qu->ToEuler() * Mathf::Rad2Deg;
	Vector3 prev = prevEuler[qu];
	if (ImGui::DragFloat3(label, (float*)&euler, 1.f))
	{
		Quaternion deltaQuat = Quaternion::CreateFromYawPitchRoll(
			(euler.y - prev.y) * Mathf::Deg2Rad,
			(euler.x - prev.x) * Mathf::Deg2Rad,
			(euler.z - prev.z) * Mathf::Deg2Rad
		);
		if (deltaQuat.Length() > Mathf::AngleEpsilon)
		{
			*qu = deltaQuat * (*qu);
			euler = qu->ToEuler() * Mathf::Rad2Deg;
			isEdit = true;
		}
	}
    prevEuler[qu] = euler;
	return isEdit;
}

bool ImGui::DragQuaternionLocal(const char* label, const Quaternion* pQuaternion, float v_speed, float v_min, float v_max, const char* format, ImGuiSliderFlags flags)
{	
	static std::unordered_map<Quaternion*, Vector3> prevEuler;
	bool isEdit = false;
	Quaternion* qu = const_cast<Quaternion*>(pQuaternion);
	Vector3 euler = qu->ToEuler() * Mathf::Rad2Deg;
	Vector3 prev = prevEuler[qu];
	if (ImGui::DragFloat3(label, (float*)&euler, 1.f))
	{
		Quaternion deltaQuat = Quaternion::CreateFromYawPitchRoll(
			(euler.y - prev.y) * Mathf::Deg2Rad,
			(euler.x - prev.x) * Mathf::Deg2Rad,
			(euler.z - prev.z) * Mathf::Deg2Rad
		);
		if (deltaQuat.Length() > Mathf::AngleEpsilon)
		{
			*qu *= deltaQuat;
			euler = qu->ToEuler() * Mathf::Rad2Deg;
			isEdit = true;
		}
	}
	prevEuler[qu] = euler;
	return isEdit;
}

bool ImGui::ColorEdit3(const char* label, const Vector3* pColor, ImGuiColorEditFlags flags)
{
	return ImGui::ColorEdit3(label, (float*)pColor, flags);
}

bool ImGui::ColorEdit3(const char* label, const Color* pColor, ImGuiColorEditFlags flags)
{
	return ImGui::ColorEdit3(label, (float*)pColor, flags);
}

bool ImGui::ColorEdit3(const char* label, const Vector4* pColor, ImGuiColorEditFlags flags)
{
	return ImGui::ColorEdit3(label, (float*)pColor, flags);
}

bool ImGui::ColorEdit4(const char* label, const Color* pColor, ImGuiColorEditFlags flags)
{
	return ImGui::ColorEdit4(label, (float*)pColor, flags);
}

bool ImGui::ColorEdit4(const char* label, const Vector4* pColor, ImGuiColorEditFlags flags)
{
	return ImGui::ColorEdit4(label, (float*)pColor, flags);
}

void ImGui::EditTransformHierarchy(Transform* pTransform)
{
	auto ObjectEditUI = [](GameObject* object)
		{
			bool isEdit = false;
			bool active = object->Active;
			if (ImGui::Checkbox("Active", &active))
			{
				object->Active = active;
			}
			if (object->transform.Parent)
			{
				isEdit |= ImGui::DragVector3("Position", &object->transform.localPosition, 0.1f);
				isEdit |= ImGui::DragQuaternionLocal("Rotation", &object->transform.localRotation);
				isEdit |= ImGui::DragVector3("Scale", &object->transform.localScale, 0.1f);
			}
			else
			{
				isEdit |= ImGui::DragVector3("Position", &object->transform.position, 0.1f);
				isEdit |= ImGui::DragQuaternionWorld("Rotation", &object->transform.rotation);
				isEdit |= ImGui::DragVector3("Scale", &object->transform.scale, 0.1f);
			}
			if (isEdit)
				object->transform.PushUpdateList();
		};

	std::function<void(Transform* transform)> TransformBFS = [&](Transform* transform)
		{				
			auto dragFunc = [transform]()
				{
					// 드래그 앤 드롭 소스 설정
					if (ImGui::BeginDragDropSource())
					{
						ImGui::SetDragDropPayload("DRAG_TRANSFORM", &transform, sizeof(Transform*)); // 드래그 데이터 설정
						ImGui::Text("Dragging: %s", transform->gameObject.GetNameToString().c_str());
						ImGui::EndDragDropSource();
					}

					// 드래그 앤 드롭 타겟 설정
					if (ImGui::BeginDragDropTarget())
					{
						if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DRAG_TRANSFORM"))
						{
							// 드래그된 데이터 가져오기
							Transform* droppedTransform = *(Transform**)payload->Data;

							// 드랍 이벤트 처리
							if (droppedTransform)
							{
								// 부모-자식 관계 설정 (예시)
								droppedTransform->SetParent(transform, false);
							}
						}
						ImGui::EndDragDropTarget();
					}
				};
			auto rightClickFunc = [transform]()
				{
					if (ImGui::BeginPopupContextItem("TransformBFSMenu")) 
					{
						if (ImGui::MenuItem("Clear Parent")) 
						{
							transform->SetParent(nullptr);
						}
						if (ImGui::MenuItem("Rename")) 
						{
							static char newName[256] = "";
							auto RenamePopup = [transform]()
								{
									ImGui::OpenPopup("Rename Input");  // 팝업을 열기
									if (ImGui::BeginPopupModal("Rename Input", 0, ImGuiWindowFlags_AlwaysAutoResize))
									{
										ImGui::Text("Enter new name:");
										ImGui::InputText("New Name", newName, IM_ARRAYSIZE(newName));
										if (ImGui::Button("OK"))
										{
											transform->gameObject.SetName(utfConvert::utf8_to_wstring(newName).c_str());
											sceneManager.PopImGuiPopupFunc();
											memset(newName, 0, sizeof(newName));
										}
										if (ImGui::Button("Cancel"))
										{
											sceneManager.PopImGuiPopupFunc();
										}
										ImGui::EndPopup();
									}
								};
							strcpy_s(newName, transform->gameObject.GetNameToString().c_str());
							sceneManager.PushImGuiPopupFunc(RenamePopup);
						}
						if (ImGui::MenuItem("Destroy GameObject")) 
						{
							GameObject::Destroy(transform->gameObject);
						}
						ImGui::EndPopup();
					}
				};
			auto leftClickFunc = [transform]()
				{
					Scene::GuizmoSetting.SelectObjectHelp = &transform->gameObject;
				};
			auto leftClickRealseFunc = [transform]()
				{
					if (Scene::GuizmoSetting.SelectObjectHelp == &transform->gameObject)
					{
						Scene::GuizmoSetting.SelectObject = &transform->gameObject;
						Scene::GuizmoSetting.SelectObjectHelp = nullptr;
					}
				};

			if (transform == nullptr)
				return;

			if (!sceneManager.GetObjectToID(transform->gameObject.GetInstanceID()))
			{
				return;
			}
			ImGui::PushID(g_id++);
			static std::string nodeName;			
			nodeName = transform->gameObject.GetNameToString();
			unsigned int childCount = transform->GetChildCount();

			ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_SpanFullWidth | ImGuiTreeNodeFlags_OpenOnDoubleClick;
			bool isSelect = Scene::GuizmoSetting.SelectObject == &transform->gameObject;

			if (isSelect)
				ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.0f, 0.0f, 0.7f, 1.0f));
			if (ImGui::TreeNodeEx(nodeName.c_str(), flags))
			{
				if (isSelect)
					ImGui::PopStyleColor();

				dragFunc();
				rightClickFunc();
				
				if (!isSelect && ImGui::IsItemClicked(ImGuiMouseButton_Left))
					leftClickFunc();

				if (ImGui::IsItemHovered() && ImGui::IsMouseReleased(ImGuiMouseButton_Left))
				{
					leftClickRealseFunc();
				}

				ObjectEditUI(&transform->gameObject);
				for (size_t i = 0; i < childCount; ++i)
				{
					TransformBFS(transform->GetChild(i));
				}
				ImGui::TreePop();
			}
			else
			{
				if(isSelect)
					ImGui::PopStyleColor();

				dragFunc();
				rightClickFunc();
				if (!isSelect && ImGui::IsItemClicked(ImGuiMouseButton_Left))
					leftClickFunc();


				if (ImGui::IsItemHovered() && ImGui::IsMouseReleased(ImGuiMouseButton_Left))
				{
					leftClickRealseFunc();
				}
			}	
			ImGui::PopID();
		};	
	TransformBFS(pTransform);
}

void ImGui::EditHierarchyView()
{
	ObjectList objects = sceneManager.GetObjectList();
	PushID("EditHierarchyView");
	for (auto& object : objects)
	{
		if (object->transform.Parent == nullptr)
		{		
			EditTransformHierarchy(&object->transform);
		}		
	}
	PopID();
}

void ImGui::EditTransform(GameObject* gameObject)
{
	ImGui::PushID(g_id);
	bool isEdit = false;
	if (gameObject->transform.Parent)
	{	
		isEdit |= ImGui::DragVector3("Position", &gameObject->transform.localPosition, 0.1f);
		isEdit |= ImGui::DragQuaternionLocal("Rotation", &gameObject->transform.localRotation);
		isEdit |= ImGui::DragVector3("Scale", &gameObject->transform.localScale, 0.1f);
	}
	else
	{
		isEdit |= ImGui::DragVector3("Position", &gameObject->transform.position, 0.1f);
		isEdit |= ImGui::DragQuaternionWorld("Rotation", &gameObject->transform.rotation);
		isEdit |= ImGui::DragVector3("Scale", &gameObject->transform.scale, 0.1f);
	}
	if (isEdit)
		gameObject->transform.PushUpdateList();
	ImGui::PopID();
	g_id++;
}

void ImGui::EditCamera(const char* label, Camera* pCamera, CameraMoveHelper* pCameraMoveHelper)
{
	ImGui::Text(label);
	EditTransform(&pCamera->gameObject);
	ImGui::PushID(g_id);

	ImGui::Checkbox("WireFrame", &D3D11_GameApp::GetRenderer().isWireFrame);
	ImGui::Checkbox("Perspective", &pCamera->isPerspective);
	if (pCamera->isPerspective)
	{
		ImGui::SliderFloat("FOV", &pCamera->FOV, 10, 120);
	}
	ImGui::SliderFloat("Near", &pCamera->Near, 0.05f, 10.f);
	ImGui::SliderFloat("Far", &pCamera->Far, 15.f, 10000.f);
	if (pCameraMoveHelper)
	{
		ImGui::DragFloat("Move Speed", &pCameraMoveHelper->moveSpeed, 1.f, 1.f, 1000.f);
		ImGui::DragFloat("Rotation Speed", &pCameraMoveHelper->rotSpeed, 0.1f, 0.1f, 1.f);
	}
	ImGui::Text("");
	ImGui::PopID();
	g_id++;
}

void ImGui::EditLight(cb_PBRDirectionalLight* Light)
{
	constexpr char label[cb_PBRDirectionalLight::MAX_LIGHT_COUNT][20] = {
		{"Directional Light 1"},
		{"Directional Light 2"},
		{"Directional Light 3"},
		{"Directional Light 4"} };
	if (Button("Add Directional Light"))
		Light->PushLight();
	if (Button("Sub Directional Light"))
		Light->PopLight();

	for (int i = 0; i < Light->LightsCount; i++)
	{
		ImGui::Text(label[i]);
		ImGui::PushID(g_id);
		ImGui::ColorEdit4("Color", &Light->Lights[i].LightColor);
		ImGui::DragVector3("Dir", &Light->Lights[i].LightDir, 0.01f, -1.f, 1.f);
		ImGui::DragFloat("Intensity", &Light->Lights[i].LightIntensity, 1.f, 0.0000001f, 100.f);
		ImGui::Text("");
		ImGui::PopID();
		g_id++;
	}
}

void ImGui::EditDirectionalLights(DefferdRenderer* renderer)
{
	ImGui::BeginChild("DirectLight", ImVec2(0.0f, 0.0f), ImGuiChildFlags_Borders | ImGuiChildFlags_AutoResizeX | ImGuiChildFlags_AutoResizeY | ImGuiChildFlags_AlwaysAutoResize, ImGuiWindowFlags_NoMove);

	size_t lightsCount = renderer->directLight.size();
	if (Button("Add Directional Light"))
	{
		renderer->directLight.PushDirectLight(std::format("Light {}", lightsCount), 
			DirectionLightData
			{
				.Color = {1, 1, 1, 1},
				.Directoin = {0, -1, 1, 0}
			});
	}	
	if (lightsCount > 1 && Button("Sub Directional Light"))
		renderer->directLight.PopDirectLight(std::format("Light {}", lightsCount - 1));
	lightsCount = renderer->directLight.size();
	int size = renderer->directLight.GetTextureSize();
	if (ImGui::InputInt("Shadow Map Size", &size))
	{
		renderer->directLight.SetTextureSize(size);
	}

	for (size_t i = 0; i < lightsCount; i++)
	{
		ImGui::PushID(g_id);
		auto& light = renderer->directLight.GetDirectLight(i);
		ImGui::Text(std::format("Direct Light {}", i).c_str());
		ImGui::ColorEdit3("Color", (float*)&light.Color.x);
		ImGui::DragFloat("Intensity", (float*)&light.Color.w, 0.1f, 0.1f);
		ImGui::DragFloat3("Dir", (float*)&light.Directoin, 0.1f, -1.f, 1.f);
		ImGui::PopID();
		g_id++;
	}

	ImGui::EndChild();

	ImGui::BeginChild("PointLight", ImVec2(0.0f, 0.0f), ImGuiChildFlags_Borders | ImGuiChildFlags_AutoResizeX | ImGuiChildFlags_AutoResizeY | ImGuiChildFlags_AlwaysAutoResize, ImGuiWindowFlags_NoMove);

	lightsCount = renderer->pointLight.size();
	if (Button("Add Point Light"))
	{
		renderer->pointLight.PushPointLight(std::format("Light {}", lightsCount),
											PointLightData
											  {
												  .Color = {1, 1, 1, 1},
												  .position = {0, -1, 1, 0}
											  });
	}
	if (lightsCount > 0 && Button("Sub Point Light"))
		renderer->pointLight.PopPointLight(std::format("Light {}", lightsCount - 1));
	lightsCount = renderer->pointLight.size();

	for (size_t i = 0; i < lightsCount; i++)
	{
		ImGui::PushID(g_id);
		auto& light = renderer->pointLight.GetPointLight(i);
		ImGui::Text(std::format("Direct Light {}", i).c_str());
		ImGui::ColorEdit3("Color", (float*)&light.Color.x);
		ImGui::DragFloat("Intensity", (float*)&light.Color.w, 0.1f, 0.1f);
		ImGui::DragFloat3("position", (float*)&light.position, 0.1f);
		ImGui::PopID();
		g_id++;
	}

	ImGui::EndChild();
}

namespace CompressPopupField
{
	static int popupCount = 0;
	static std::queue<std::string>   str_queue;
	static std::queue<std::wstring>  wstr_queue;
	static std::queue<std::wstring>  savePath_Queue;
	static std::vector<std::thread>  compressThreads;
	static std::vector<ID3D11ShaderResourceView*> tempSRVvec;
	static std::unordered_set<GameObject*> DestroyObjects;	//압축 파괴할 오브젝트 목록
	static std::atomic_int threadsCounter;
}

bool ImGui::ShowCompressPopup(const wchar_t* path, ETextureType texType)
{
	if (!sceneManager.IsImGuiActive())
		return false;

	std::filesystem::path originPath = path;
	originPath.replace_extension(L".dds");
	constexpr wchar_t textuers[] = L"Textures";
	std::filesystem::path savePath = originPath.parent_path() / textuers / originPath.filename();
	bool isExists = std::filesystem::exists(savePath);
#ifdef _DEBUG
	if (!isExists)
		return false;
#endif 
	using namespace CompressPopupField;
	popupCount++;
	str_queue.push(utfConvert::wstring_to_utf8(path));
	wstr_queue.push(path);
	savePath_Queue.push(savePath.c_str());

	/*추천 포멧!!
	Albedo		BC1/BC3/BC7	 알파 채널 유무에 따라 선택. 색상 데이터의 높은 품질 유지 필요시 BC7 //생각보다 압축티 많이남..
	Normal		BC5	         2채널 사용하는 노말은 BC5.
	Specular	BC1/BC7		 단순 데이터면 BC1, 고품질 필요 시 BC7.
	Emissive	BC1/BC3/BC7  불투명은 BC1, 알파 필요 시 BC3. 고품질 필요시 BC7
	Opacity		BC4/BC3		 단일 채널은 BC4, RGBA 필요 시 BC3.
	Metalness	BC4			 단일 채널 데이터로 효율적인 압축.
	Roughness	BC4			 단일 채널 데이터로 미세 디테일 유지.
	AO			BC4			 단일 채널 데이터로 충분한 품질 유지.
	*/
	constexpr const char* compressTypeStr[Utility::E_COMPRESS::MAX] =
	{
		"BC1",
		"BC3",
		"BC4",
		"BC5",
		"BC6",
		"BC7"
	};
	constexpr const char* textureTypeStr[] =
	{
		"Albedo",
		"Normal",
		"Specular",
		"Emissive",
		"Opacity",
		"Metalness",
		"Roughness",
		"Ambient Occulusion"
	};

	auto popupFunc = [texType, compressTypeStr, textureTypeStr, isExists]()
		{
			std::wstring& wstr_path = wstr_queue.front();
			std::string& str_path = str_queue.front();
			std::wstring& save_path = savePath_Queue.front();

			ImGui::OpenPopup("Compress Texture");
			ImGui::SetNextWindowSize(ImVec2(750, 480));
			if (ImGui::BeginPopupModal("Compress Texture", nullptr, ImGuiWindowFlags_NoResize))
			{
				static bool initialized = false;
				static bool UseAutoCompress = false;
				static int textureType = 0;
				static Utility::E_COMPRESS::TYPE compressType = Utility::E_COMPRESS::MAX;
				static bool showAdvancedSettings = false;
				if (!initialized)
				{
					textureType = (int)texType;
					initialized = true;
				}
				size_t pos = str_path.find_last_of('\\');
				pos = pos != std::string::npos ? pos : str_path.find_last_of('/');
				if (pos != std::string::npos)
				{
					ImGui::Text(str_path.c_str() + pos + 1);
				}
				ImGui::Checkbox("Use Advanced", &showAdvancedSettings);
				if (showAdvancedSettings)
				{
					ImGui::Combo("Compress Type", (int*)&compressType, compressTypeStr, Utility::E_COMPRESS::MAX);
				}
				else
				{
					ImGui::Combo("Texture Type", &textureType, textureTypeStr, std::size(textureTypeStr));
					{
						ETextureType type = (ETextureType)textureType;
						switch (type)
						{
						case ETextureType::Normal:
							compressType = Utility::E_COMPRESS::BC5;
							break;
						case ETextureType::Albedo:
						case ETextureType::Specular:
						case ETextureType::Emissive:
						case ETextureType::Opacity:
							compressType = Utility::E_COMPRESS::BC7;
							break;
						case ETextureType::Metalness:
						case ETextureType::Roughness:
						case ETextureType::AmbientOcculusion:
							compressType = Utility::E_COMPRESS::BC4;
							break;
						case ETextureType::Null:
							compressType = Utility::E_COMPRESS::MAX;
							break;
						default:
							compressType = Utility::E_COMPRESS::BC7;
						}
					}
				}
				ImGui::Button("Auto Compress", &UseAutoCompress);
				if (compressType == Utility::E_COMPRESS::MAX)
					UseAutoCompress = false;

				if (ImGui::Button("OK") || ImGui::IsKeyPressed(ImGuiKey_Enter) || UseAutoCompress)
				{
					auto compressThreadFunc = [save_path, path = wstr_path, compType = compressType, isExists]()
						{
							static std::mutex mt;
							std::shared_ptr<DirectX::ScratchImage> image;
							mt.lock();
							Utility::CheckHRESULT(CoInitializeEx(nullptr, COINIT_MULTITHREADED)); //작업 스레드의 Com 객체 사용 활성화
							if (textureManager.IsTextureLoaded(path.c_str()))
							{
								textureManager.ReleaseSharingTexture(path.c_str());
							}
							ID3D11ShaderResourceView* tempSRV;
							std::filesystem::path filePath = path;
							if (isExists)
								Utility::CheckHRESULT(Utility::CreateTextureFromFile(RendererUtility::GetDevice(), save_path.c_str(), nullptr, &tempSRV));
							else
								image = Utility::CreateCompressTexture(RendererUtility::GetDevice(), path.c_str(), nullptr, &tempSRV, compType);
							textureManager.InsertTexture(path.c_str(), tempSRV);
							tempSRVvec.push_back(tempSRV);
							CoUninitialize();
							mt.unlock();
							if (image)
							{
								Utility::SaveTextureForDDS(save_path.c_str(), image);
							}
							return;
						};
					compressThreads.emplace_back(compressThreadFunc); //스레드 처리

					str_queue.pop();
					wstr_queue.pop();
					savePath_Queue.pop();
					ImGui::CloseCurrentPopup();
					sceneManager.PopImGuiPopupFunc();
					popupCount--;
					initialized = false;
					if (popupCount == 0)
					{
						UseAutoCompress = false;

						threadsCounter = compressThreads.size();
						static float threadsStart; 
						threadsStart = threadsCounter;
						sceneManager.SetLodingImguiFunc([]()
							{
								float currCounter = threadsCounter;
								float progress = 1.f - currCounter / threadsStart;
								ImGui::OpenPopup("Compress Image");
								ImGuiIO& io = ImGui::GetIO();
								if (ImGui::BeginPopupModal("Compress Image"))
								{
									// 진행 상태 표시
									ImGui::Text("Compress Image...");
									ImGui::ProgressBar(progress); // Progress Bar

									// 작업이 끝났을 때 팝업 닫기
									if (progress >= 1.0f)
									{
										ImGui::CloseCurrentPopup();
									}
									ImGui::SetWindowSize(ImVec2(500.0f, 0.0f));
									ImGui::SetWindowPos(ImVec2
									(io.DisplaySize.x * 0.5f - ImGui::GetWindowWidth() * 0.5f,
										io.DisplaySize.y * 0.5f - ImGui::GetWindowHeight() * 0.5f));
									ImGui::EndPopup();
								}

								if (threadsCounter == 0)
								{
									sceneManager.EndLodingImguiFunc();
								}
							});

						auto lodingWaitThreadsFunc = []()
							{
								for (auto& threads : compressThreads)
								{
									threads.join();
									--threadsCounter;
								}
								for (auto& obj : DestroyObjects)
								{
									sceneManager.DestroyObject(obj);
								}
								for (auto& tempSRV : tempSRVvec)
								{
									tempSRV->Release();
								}
								compressThreads.clear();
								compressThreads.shrink_to_fit();
								tempSRVvec.clear();
								tempSRVvec.shrink_to_fit();
								return;
							};
						std::thread lodingWaitThread(lodingWaitThreadsFunc);
						lodingWaitThread.detach();
					}
				}
				ImGui::EndPopup();
			}
		};

	sceneManager.PushImGuiPopupFunc(popupFunc);
	return true;
}

bool ImGui::DestroyObjTextureCompressEnd(GameObject* obj)
{
	using namespace CompressPopupField;
	if (popupCount > 0)
	{
		DestroyObjects.insert(obj);
		return true;
	}
	else
	{
		sceneManager.DestroyObject(obj);
		return false;
	}
}

bool ImGui::ShowOpenGameObjectPopup()
{
	constexpr float sizeX = 1280;
	constexpr float halfX = 1280 * 0.5f;
	constexpr float sizeY = 720;
	constexpr float halfY = 720 * 0.5f;
	auto popupFunc = []()
		{
			SIZE size = D3D11_GameApp::GetClientSize();
			static bool first = true;
			ImGui::OpenPopup("Open GameObject");
			ImGui::SetNextWindowSize({ sizeX, sizeY });
			ImGui::SetNextWindowPos({ size.cx * 0.5f - halfX, size.cy * 0.5f - halfY });
			if (ImGui::BeginPopupModal("Open GameObject", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove))
			{
				if (first)
				{
					ImGuiFileDialog::Instance()->OpenDialog(
						"Open Path Dlg",
						"Choose Path",
						".GameObject"
						);
					first = false;
				}		
				ImGui::SetNextWindowSize({ sizeX, sizeY });
				ImGui::SetNextWindowPos({ size.cx * 0.5f - halfX, size.cy * 0.5f - halfY});
				if (ImGuiFileDialog::Instance()->Display("Open Path Dlg"))
				{
					if (ImGuiFileDialog::Instance()->IsOk())
					{
						std::string filePath = ImGuiFileDialog::Instance()->GetFilePathName();
						if (!filePath.empty())
						{
							gameObjectFactory.DeserializedObject(utfConvert::utf8_to_wstring(filePath).c_str());
						}
						sceneManager.PopImGuiPopupFunc();
						ImGui::CloseCurrentPopup();
						ImGuiFileDialog::Instance()->Close();
						first = true;
					}
					else
					{
						sceneManager.PopImGuiPopupFunc();
						ImGui::CloseCurrentPopup();
						ImGuiFileDialog::Instance()->Close();
						first = true;
					}
				}
				ImGui::EndPopup();
			}
		};

	bool ActiveImgui = sceneManager.IsImGuiActive();
	if (ActiveImgui)
	{
		sceneManager.PushImGuiPopupFunc(popupFunc);
		return true;
	}
	else
		return false;
}

bool ImGui::ShowSaveAsGameObjectPopup(GameObject* object)
{
	constexpr float sizeX = 1280;
	constexpr float halfX = 1280 * 0.5f;
	constexpr float sizeY = 720;
	constexpr float halfY = 720 * 0.5f;
	auto popupFunc = [object]()
		{
			SIZE size = D3D11_GameApp::GetClientSize();
			static bool first = true;
			ImGui::OpenPopup("Save As GameObject");
			ImGui::SetNextWindowSize({ sizeX, sizeY });
			ImGui::SetNextWindowPos({ size.cx * 0.5f - halfX, size.cy * 0.5f - halfY });
			if (ImGui::BeginPopupModal("Save As GameObject", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove))
			{
				static IGFD::FileDialogConfig config{};
				GameObject* selectObject = Scene::GuizmoSetting.SelectObject;
				if (first)
				{
					config.fileName = selectObject->GetNameToString();
					ImGuiFileDialog::Instance()->OpenDialog(
						"Save Path Dlg",
						"Choose Path",
						".GameObject",
						config);
					first = false;
				}
				ImGui::SetNextWindowSize({ sizeX, sizeY });
				ImGui::SetNextWindowPos({ size.cx * 0.5f - halfX, size.cy * 0.5f - halfY });
				if (ImGuiFileDialog::Instance()->Display("Save Path Dlg")) 
				{
					if (ImGuiFileDialog::Instance()->IsOk()) 
					{
						std::string filePath = ImGuiFileDialog::Instance()->GetFilePathName();
						if (!filePath.empty())
						{
							gameObjectFactory.SerializedObject(selectObject, utfConvert::utf8_to_wstring(filePath).c_str());
						}
						sceneManager.PopImGuiPopupFunc();
						ImGui::CloseCurrentPopup();
						ImGuiFileDialog::Instance()->Close();
						first = true;
					}				
					else
					{
						sceneManager.PopImGuiPopupFunc();
						ImGui::CloseCurrentPopup();
						ImGuiFileDialog::Instance()->Close();
						first = true;
					}
				}
				ImGui::EndPopup();
			}
		};

	bool ActiveImgui = sceneManager.IsImGuiActive();
	if (ActiveImgui)
	{
		sceneManager.PushImGuiPopupFunc(popupFunc);
		return true;
	}
	else
		return false;
}

bool ImGui::ShowLoadScenePopup()
{
	constexpr float sizeX = 1280;
	constexpr float halfX = 1280 * 0.5f;
	constexpr float sizeY = 720;
	constexpr float halfY = 720 * 0.5f;
	auto popupFunc = []()
		{
			SIZE size = D3D11_GameApp::GetClientSize();
			static bool first = true;
			ImGui::OpenPopup("Load Scene");
			ImGui::SetNextWindowSize({ sizeX, sizeY });
			ImGui::SetNextWindowPos({ size.cx * 0.5f - halfX, size.cy * 0.5f - halfY });
			if (ImGui::BeginPopupModal("Load Scene", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove))
			{
				if (first)
				{
					ImGuiFileDialog::Instance()->OpenDialog(
						"Open Path Dlg",
						"Choose Path",
						".Scene"
					);
					first = false;
				}
				ImGui::SetNextWindowSize({ sizeX, sizeY });
				ImGui::SetNextWindowPos({ size.cx * 0.5f - halfX, size.cy * 0.5f - halfY });
				if (ImGuiFileDialog::Instance()->Display("Open Path Dlg"))
				{
					if (ImGuiFileDialog::Instance()->IsOk())
					{
						std::string filePath = ImGuiFileDialog::Instance()->GetFilePathName();
						if (!filePath.empty())
						{
							sceneManager.LoadScene(utfConvert::utf8_to_wstring(filePath).c_str());
						}
						sceneManager.PopImGuiPopupFunc();
						ImGui::CloseCurrentPopup();
						ImGuiFileDialog::Instance()->Close();
						first = true;
					}
					else
					{
						sceneManager.PopImGuiPopupFunc();
						ImGui::CloseCurrentPopup();
						ImGuiFileDialog::Instance()->Close();
						first = true;
					}
				}
				ImGui::EndPopup();
			}
		};

	bool ActiveImgui = sceneManager.IsImGuiActive();
	if (ActiveImgui)
	{
		sceneManager.PushImGuiPopupFunc(popupFunc);
		return true;
	}
	else
		return false;
}

bool ImGui::ShowSaveAsScenePopup(Scene* scene)
{
	constexpr float sizeX = 1280;
	constexpr float halfX = 1280 * 0.5f;
	constexpr float sizeY = 720;
	constexpr float halfY = 720 * 0.5f;
	auto popupFunc = [scene]()
		{
			SIZE size = D3D11_GameApp::GetClientSize();
			static bool first = true;
			ImGui::OpenPopup("Save As Scene");
			ImGui::SetNextWindowSize({ sizeX, sizeY });
			ImGui::SetNextWindowPos({ size.cx * 0.5f - halfX, size.cy * 0.5f - halfY });
			if (ImGui::BeginPopupModal("Save As Scene", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove))
			{
				static IGFD::FileDialogConfig config{};
				GameObject* selectObject = Scene::GuizmoSetting.SelectObject;
				if (first)
				{
					config.fileName = utfConvert::wstring_to_utf8(scene->GetSceneName());
					ImGuiFileDialog::Instance()->OpenDialog(
						"Save Path Dlg",
						"Choose Path",
						".Scene",
						config);
					first = false;
				}
				ImGui::SetNextWindowSize({ sizeX, sizeY });
				ImGui::SetNextWindowPos({ size.cx * 0.5f - halfX, size.cy * 0.5f - halfY });
				if (ImGuiFileDialog::Instance()->Display("Save Path Dlg"))
				{
					if (ImGuiFileDialog::Instance()->IsOk())
					{
						std::string filePath = ImGuiFileDialog::Instance()->GetFilePathName();
						if (!filePath.empty())
						{
							std::wstring savePath = utfConvert::utf8_to_wstring(filePath);
							sceneManager.SaveScene(savePath.c_str());
							sceneManager.LoadScene(savePath.c_str());
						}
						sceneManager.PopImGuiPopupFunc();
						ImGui::CloseCurrentPopup();
						ImGuiFileDialog::Instance()->Close();
						first = true;
					}
					else
					{
						sceneManager.PopImGuiPopupFunc();
						ImGui::CloseCurrentPopup();
						ImGuiFileDialog::Instance()->Close();
						first = true;
					}
				}
				ImGui::EndPopup();
			}
		};


	bool ActiveImgui = sceneManager.IsImGuiActive();
	if (ActiveImgui)
	{
		sceneManager.PushImGuiPopupFunc(popupFunc);
		return true;
	}
	else
		return false;
}

bool ImGui::ShowAddScenePopup()
{
	constexpr float sizeX = 1280;
	constexpr float halfX = 1280 * 0.5f;
	constexpr float sizeY = 720;
	constexpr float halfY = 720 * 0.5f;
	auto popupFunc = []()
		{
			SIZE size = D3D11_GameApp::GetClientSize();
			static bool first = true;
			ImGui::OpenPopup("Add Scene");
			ImGui::SetNextWindowSize({ sizeX, sizeY });
			ImGui::SetNextWindowPos({ size.cx * 0.5f - halfX, size.cy * 0.5f - halfY });
			if (ImGui::BeginPopupModal("Add Scene", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove))
			{
				if (first)
				{
					ImGuiFileDialog::Instance()->OpenDialog(
						"Open Path Dlg",
						"Choose Path",
						".Scene"
					);
					first = false;
				}
				ImGui::SetNextWindowSize({ sizeX, sizeY });
				ImGui::SetNextWindowPos({ size.cx * 0.5f - halfX, size.cy * 0.5f - halfY });
				if (ImGuiFileDialog::Instance()->Display("Open Path Dlg"))
				{
					if (ImGuiFileDialog::Instance()->IsOk())
					{
						std::string filePath = ImGuiFileDialog::Instance()->GetFilePathName();
						if (!filePath.empty())
						{
							sceneManager.AddScene(utfConvert::utf8_to_wstring(filePath).c_str());
						}
						sceneManager.PopImGuiPopupFunc();
						ImGui::CloseCurrentPopup();
						ImGuiFileDialog::Instance()->Close();
						first = true;
					}
					else
					{
						sceneManager.PopImGuiPopupFunc();
						ImGui::CloseCurrentPopup();
						ImGuiFileDialog::Instance()->Close();
						first = true;
					}
				}
				ImGui::EndPopup();
			}
		};

	bool ActiveImgui = sceneManager.IsImGuiActive();
	if (ActiveImgui)
	{
		sceneManager.PushImGuiPopupFunc(popupFunc);
		return true;
	}
	else
		return false;
}

bool ImGui::ShowSubScenePopup()
{
	constexpr float sizeX = 1280;
	constexpr float halfX = 1280 * 0.5f;
	constexpr float sizeY = 720;
	constexpr float halfY = 720 * 0.5f;
	auto popupFunc = []()
		{
			SIZE size = D3D11_GameApp::GetClientSize();
			static int selectSceneNum = 0;
			static std::vector<char> sceneListTemp;
			ImGui::OpenPopup("Sub Scene"); 
			ImGui::SetNextWindowSize({ sizeX, sizeY });
			ImGui::SetNextWindowPos({ size.cx * 0.5f - halfX, size.cy * 0.5f - halfY });
			if (ImGui::BeginPopupModal("Sub Scene", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove))
			{
				auto sceneList = sceneManager.GetSceneList();
				size_t count = sceneList.size();
				if (count > 0)
				{
					std::string sceneName;
					sceneListTemp.clear();
					for (size_t i = 0; i < count; i++)
					{
						sceneName = utfConvert::wstring_to_utf8(sceneList[i]);
						size_t startIndex = sceneListTemp.size();
						sceneListTemp.resize(startIndex + sceneName.length() + 1);
						strcpy_s(&sceneListTemp[startIndex], sceneName.length() + 1, sceneName.c_str());
					}
					sceneListTemp.emplace_back('\0');
					ImGui::Combo("Scene List", &selectSceneNum, sceneListTemp.data(), count);
				}
				if (ImGui::Button("OK"))
				{
					ImGui::CloseCurrentPopup();
					sceneManager.PopImGuiPopupFunc();
					selectSceneNum = 0;
					if (count > 0)
					{
						sceneManager.SubScene(sceneList[selectSceneNum].c_str());
					}
				}
				ImGui::EndPopup();
			}
		};

	bool ActiveImgui = sceneManager.IsImGuiActive();
	if (ActiveImgui)
	{
		sceneManager.PushImGuiPopupFunc(popupFunc);
		return true;
	}
	else
		return false;
}