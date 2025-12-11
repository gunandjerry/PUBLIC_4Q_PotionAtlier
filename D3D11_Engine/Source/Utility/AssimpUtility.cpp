#include "AssimpUtility.h"
#include <Light/SimpleDirectionalLight.h>
#include <Manager/SceneManager.h>
#include <GameObject\Base\GameObject.h>
#include <Component\Render\SimpleMeshRender.h>
#include <Component/Render/SimpleBoneMeshRender.h>
#include <GameObject/Bone/BoneObject.h>
#include <assimp\Importer.hpp>
#include <assimp\scene.h>
#include <assimp\postprocess.h>
#include <Math/AssimpMath.h>
#include <Utility\utfConvert.h>
#include <Utility/ImguiHelper.h>
#include <Utility/SQLiteLogger.h>
#include <Component/Animation/TransformAnimation.h>
#include <Manager\ResourceManager.h>
#include <Manager/HLSLManager.h>
#include <Manager/TextureManager.h>
#include <Core/WinGameApp.h>
#include <NodeEditor/NodeEditor.h>

#include <Component/Render/PBRMeshRender.h>
#include <Component/Render/PBRBoneMeshRender.h>

#include <Math/Mathf.h>
#include <iostream>
#include <stack>
#include <future>

namespace Utility
{
	GameObject* LoadFBXfunc(const wchar_t* path,
		bool isStatic,
		SURFACE_TYPE surface,
		float* progressOut = nullptr);

	static GameObject* NewMeshObject(
		SURFACE_TYPE surface,
		const wchar_t* name)
	{
		switch (surface)
		{
		case SURFACE_TYPE::PBR:
			return NewGameObject<PBRMeshObject>(name);
		default:
			return NewGameObject(name);
		}
	}

	static SimpleMeshRender& AddMeshComponent(GameObject* obj, SURFACE_TYPE surface)
	{
		switch (surface)
		{
		case SURFACE_TYPE::PBR:
		{
			return obj->AddComponent<PBRMeshRender>();;
		}
		default:
			return obj->AddComponent<SimpleMeshRender>();
		}

	}

	static SimpleBoneMeshRender& AddBoneMeshComponent(GameObject* obj, SURFACE_TYPE surface)
	{
		switch (surface)
		{
		case SURFACE_TYPE::PBR:
		{
			return obj->AddComponent<PBRBoneMeshRender>();
		}
		default:
			return obj->AddComponent<SimpleBoneMeshRender>();
		}
	}

	static std::wstring MakeMaterialHierarchyPath(MeshRender* meshRender, GameObject* rootObject)
	{
		std::wstring materialPath;
		std::vector<GameObject*> rootForMesh = meshRender->gameObject.GetHierarchyToParent(rootObject);
		for (auto& object : rootForMesh)
		{
			materialPath += object->GetName();
			materialPath += L"/";
		}
		return materialPath;
	}

	static void LoadTexture(aiMaterial* ai_material, const wchar_t* directory, MeshRender* meshRender, GameObject* rootObject, SURFACE_TYPE surface)
	{
		using namespace ImGui;
		std::wstring materialPath = MakeMaterialHierarchyPath(meshRender, rootObject);
		std::filesystem::path assetPath = meshRender->MakeDefaultMaterialAssetPath(materialPath.c_str());
		bool isAsset = std::filesystem::exists(assetPath);
		std::filesystem::path nodePath = meshRender->MakeDefaultMaterialNodePath(materialPath.c_str());
		bool isNode = std::filesystem::exists(nodePath);	
		meshRender->CreateDefaultMaterial(materialPath.c_str());
		if (isAsset)
		{
			meshRender->materialAsset.OpenAsset(assetPath.c_str());
		}
		if (isNode)
		{
			return;
		}
		std::unique_ptr<ShaderNodeEditor> nodeEditor = meshRender->GetDefaultNodeEditor();
		aiString path;
		std::wstring basePath;
		{
			if (AI_SUCCESS == ai_material->GetTexture(aiTextureType_DIFFUSE, 0, &path) ||
				AI_SUCCESS == ai_material->GetTexture(aiTextureType_BASE_COLOR, 0, &path))
			{
				if (Utility::ParseFileName(path))
				{
					basePath = directory;
					basePath += L"\\";
					basePath += utfConvert::utf8_to_wstring(path.C_Str());

					if(std::filesystem::exists(basePath))
						nodeEditor->SetResultNode<TextureNode>(utfConvert::wstring_to_utf8(basePath).c_str(), (char*)u8"RGB", EShaderResult::Albedo);
				}
			}
			else if (aiColor4D baseColor; ai_material->Get(AI_MATKEY_COLOR_DIFFUSE, baseColor) == AI_SUCCESS)
			{
				SimpleMeshRender* simpleMeshRender = reinterpret_cast<SimpleMeshRender*>(meshRender);

				simpleMeshRender->baseColor.x = baseColor.r;
				simpleMeshRender->baseColor.y = baseColor.g;
				simpleMeshRender->baseColor.z = baseColor.b;
				simpleMeshRender->baseColor.w = baseColor.a;
				nodeEditor->SetResultNode<ConstantVector3Node>(simpleMeshRender->baseColor.ToVector3(), (const char*)u8"값", EShaderResult::Albedo);
			}
			if (AI_SUCCESS == ai_material->GetTexture(aiTextureType_NORMALS, 0, &path))
			{
				if (Utility::ParseFileName(path))
				{
					basePath = directory;
					basePath += L"\\";
					basePath += utfConvert::utf8_to_wstring(path.C_Str());

					if (std::filesystem::exists(basePath))
						nodeEditor->SetResultNode<TextureNode>(utfConvert::wstring_to_utf8(basePath).c_str(), (char*)u8"RGB", EShaderResult::Normal);
				}
			}
			if (AI_SUCCESS == ai_material->GetTexture(aiTextureType_SPECULAR, 0, &path))
			{
				if (Utility::ParseFileName(path))
				{
					basePath = directory;
					basePath += L"\\";
					basePath += utfConvert::utf8_to_wstring(path.C_Str());

					if (std::filesystem::exists(basePath))
						nodeEditor->SetResultNode<TextureNode>(utfConvert::wstring_to_utf8(basePath).c_str(), (char*)u8"RGB", EShaderResult::Specular);
				}
			}
			if (AI_SUCCESS == ai_material->GetTexture(aiTextureType_EMISSIVE, 0, &path))
			{
				if (Utility::ParseFileName(path))
				{
					basePath = directory;
					basePath += L"\\";
					basePath += utfConvert::utf8_to_wstring(path.C_Str());

					if (std::filesystem::exists(basePath))
						nodeEditor->SetResultNode<TextureNode>(utfConvert::wstring_to_utf8(basePath).c_str(), (char*)u8"RGB", EShaderResult::Emissive);
				}
			}
			if (AI_SUCCESS == ai_material->GetTexture(aiTextureType_OPACITY, 0, &path))
			{
				if (Utility::ParseFileName(path))
				{
					basePath = directory;
					basePath += L"\\";
					basePath += utfConvert::utf8_to_wstring(path.C_Str());

					if (std::filesystem::exists(basePath))
						nodeEditor->SetResultNode<TextureNode>(utfConvert::wstring_to_utf8(basePath).c_str(), (char*)u8"R", EShaderResult::Alpha);

					std::wstring path = HLSLManager::EngineShaderPath;
					switch (surface)
					{
					case SURFACE_TYPE::PBR:
						//포워드 변경 필요

						break;
					default:
						break;
					}
				}
			}

			//블랜더 기준 메탈릭 맵
			if (AI_SUCCESS == ai_material->GetTexture(aiTextureType_METALNESS, 0, &path))
			{
				if (Utility::ParseFileName(path))
				{
					basePath = directory;
					basePath += L"\\";
					basePath += utfConvert::utf8_to_wstring(path.C_Str());

					if (std::filesystem::exists(basePath))
						nodeEditor->SetResultNode<TextureNode>(utfConvert::wstring_to_utf8(basePath).c_str(), (char*)u8"R", EShaderResult::Metallic);
				}
			}
			else if (float scala; ai_material->Get(AI_MATKEY_METALLIC_FACTOR, scala) == AI_SUCCESS)
			{
				nodeEditor->SetResultNode<ConstantValueNode>(scala, (char*)u8"값", EShaderResult::Metallic);
			}

			//블랜더 기준 러프니스 맵
			if (AI_SUCCESS == ai_material->GetTexture(aiTextureType_SHININESS, 0, &path))
			{
				if (Utility::ParseFileName(path))
				{
					basePath = directory;
					basePath += L"\\";
					basePath += utfConvert::utf8_to_wstring(path.C_Str());

					if (std::filesystem::exists(basePath))
						nodeEditor->SetResultNode<TextureNode>(utfConvert::wstring_to_utf8(basePath).c_str(), (char*)u8"R", EShaderResult::Roughness);
				}
			}
			else if (float scala; ai_material->Get(AI_MATKEY_ROUGHNESS_FACTOR, scala) == AI_SUCCESS)
			{
				nodeEditor->SetResultNode<ConstantValueNode>(scala, (char*)u8"값", EShaderResult::Roughness);
			}
			nodeEditor->Save();
			nodeEditor->Export(assetPath.c_str(), true);
			meshRender->materialAsset.OpenAsset(assetPath.c_str());
		}
	}

	static TransformAnimation* SetTransformAnimation(const aiScene* pScene, GameObject& _gameObject, std::unordered_map<std::wstring, GameObject*>& addObjMap)
	{
		using Clip = TransformAnimation::Clip;
		using PositionKey = TransformAnimation::Clip::NodeAnimation::PositionKey;
		using RotationKey = TransformAnimation::Clip::NodeAnimation::RotationKey;
		using ScaleKey = TransformAnimation::Clip::NodeAnimation::ScaleKey;
		TransformAnimation& anime = _gameObject.AddComponent<TransformAnimation>();
		for (unsigned int clipNum = 0; clipNum < pScene->mNumAnimations; clipNum++)
		{
			aiAnimation* currAnimation = pScene->mAnimations[clipNum];
			Clip clip;
			clip.Duration = (float)currAnimation->mDuration;
			clip.TickTime = (float)currAnimation->mTicksPerSecond;
			std::wstring clipName = utfConvert::utf8_to_wstring(pScene->mAnimations[clipNum]->mName.C_Str());
			for (unsigned int nodeIndex = 0; nodeIndex < currAnimation->mNumChannels; nodeIndex++)
			{
				aiNodeAnim* currNodeAnim = currAnimation->mChannels[nodeIndex];
				using NodeAnime = Clip::NodeAnimation;
				NodeAnime nodeAnime;
				std::wstring currNodeName = utfConvert::utf8_to_wstring(currNodeAnim->mNodeName.C_Str());
				nodeAnime.targetName = currNodeName;

				//make keyList
				if (currNodeAnim->mNumPositionKeys > 0 && !nodeAnime.positionKeys)
				{
					nodeAnime.positionKeys = anime.get_position_key(clipName.c_str(), nodeIndex);
				}
				if (currNodeAnim->mNumRotationKeys > 0 && !nodeAnime.rotationKeys)
				{
					nodeAnime.rotationKeys = anime.get_rotation_key(clipName.c_str(), nodeIndex);
				}
				if (currNodeAnim->mNumScalingKeys > 0 && !nodeAnime.scaleKeys)
				{
					nodeAnime.scaleKeys = anime.get_scale_key(clipName.c_str(), nodeIndex);
				}

				for (unsigned int k = 0; k < currNodeAnim->mNumPositionKeys; k++)
				{
					NodeAnime::PositionKey key;
					key.position.x = currNodeAnim->mPositionKeys[k].mValue.x;
					key.position.y = currNodeAnim->mPositionKeys[k].mValue.y;
					key.position.z = currNodeAnim->mPositionKeys[k].mValue.z;
					key.Time = (float)currNodeAnim->mPositionKeys[k].mTime;
					nodeAnime.positionKeys->push_back(key);
				}
				for (unsigned int k = 0; k < currNodeAnim->mNumRotationKeys; k++)
				{
					NodeAnime::RotationKey key;
					key.rotation.x = currNodeAnim->mRotationKeys[k].mValue.x;
					key.rotation.y = currNodeAnim->mRotationKeys[k].mValue.y;
					key.rotation.z = currNodeAnim->mRotationKeys[k].mValue.z;
					key.rotation.w = currNodeAnim->mRotationKeys[k].mValue.w;

					key.Time = (float)currNodeAnim->mRotationKeys[k].mTime;
					nodeAnime.rotationKeys->push_back(key);
				}
				for (unsigned int k = 0; k < currNodeAnim->mNumScalingKeys; k++)
				{
					NodeAnime::ScaleKey key;
					key.scale.x = currNodeAnim->mScalingKeys[k].mValue.x;
					key.scale.y = currNodeAnim->mScalingKeys[k].mValue.y;
					key.scale.z = currNodeAnim->mScalingKeys[k].mValue.z;
					key.Time = (float)currNodeAnim->mScalingKeys[k].mTime;
					nodeAnime.scaleKeys->push_back(key);
				}
				clip.nodeAnimations.push_back(nodeAnime);
			}
			anime.AddClip(clipName.c_str(), clip);
		}
		return &anime;
	}

	static bool IsBone(GameObject* obj)
	{
		std::queue<GameObject*> objQue;
		objQue.push(obj);
		GameObject* currObj = nullptr;
		while (!objQue.empty())
		{
			currObj = objQue.front();
			objQue.pop();
			if (typeid(BoneObject) == typeid(*currObj))
			{
				return true;
			}
			for (unsigned int i = 0; i < currObj->transform.GetChildCount(); i++)
			{
				objQue.push(&currObj->transform.GetChild(i)->gameObject);
			}
		}
		return false;
	}	
}

bool Utility::ParseFileName(aiString& str)
{
	const char* findBackSlash = strrchr(str.C_Str(), '\\');
	const char* findSlash = strrchr(str.C_Str(), '/');
	const char* findDot = strrchr(str.C_Str(), '.');
	if (findBackSlash)
	{
		str.Set(findBackSlash + 1);
		return true;
	}
	else if (findSlash)
	{
		str.Set(findSlash + 1);
		return true;
	}
	else if (findDot)
	{
		return true;
	}
	else
		return false;
}

std::map<std::wstring, std::vector<MeshRender*>> Utility::CollectMeshComponents(GameObject* root)
{
	std::map<std::wstring, std::vector<MeshRender*>> out;
	std::stack<GameObject*> objStack;
	objStack.push(root);

	while (!objStack.empty())
	{
		GameObject* curr = objStack.top();
		objStack.pop();

		for (size_t i = 0; i < curr->GetComponentCount(); ++i)
		{
			MeshRender* mesh = curr->GetComponentAtIndex<MeshRender>(i);
			if (mesh)
			{
				out[curr->Name].emplace_back(mesh);
			}
		}

		for (size_t i = 0; i < curr->transform.GetChildCount(); i++)
		{
			objStack.push(&curr->transform.GetChild(i)->gameObject);
		}
	}
	return out;
}

void Utility::LoadFBX(const wchar_t* path, bool isStatic, SURFACE_TYPE surface, GameObject** pRootObjectOut)
{
	static std::vector<std::future<void>> isLoaded{};

	static GameObject* ret = nullptr;
	static std::wstring loadPath;
	static float progress = 0.f;

	if (isLoaded.empty())
	{
		//d3d 멀티스레드 허용
		ID3D10Multithread* multithread = nullptr;
		RendererUtility::GetDevice()->QueryInterface(__uuidof(ID3D10Multithread), (void**)&multithread);
		if (multithread)
		{
			multithread->SetMultithreadProtected(TRUE);
			multithread->Release();
		}

		sceneManager.SetLodingImguiFunc([]()
										{
											ImGui::OpenPopup("Import FBX");
											ImGuiIO& io = ImGui::GetIO();
											if (ImGui::BeginPopupModal("Import FBX"))
											{
												// 진행 상태 표시
												ImGui::Text("Import FBX...");
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
											for (size_t i = 0; i < isLoaded.size(); i++)
											{
												auto status = isLoaded[i].wait_for(std::chrono::seconds(0));
												if (status == std::future_status::ready)
												{
													isLoaded.erase(isLoaded.begin() + i);
													i--;
												}
											}

											if (isLoaded.empty())
											{
												sceneManager.EndLodingImguiFunc();

												ComPtr<ID3D11DeviceContext> context;
												RendererUtility::GetDevice()->GetImmediateContext(&context);

												D3D11_QUERY_DESC queryDesc = {};
												queryDesc.Query = D3D11_QUERY_EVENT;

												ID3D11Query* pQuery = nullptr;
												RendererUtility::GetDevice()->CreateQuery(&queryDesc, &pQuery);

												if (pQuery)
												{
													context->End(pQuery);
													while (S_OK != context->GetData(pQuery, nullptr, 0, 0))
													{
														std::this_thread::sleep_for(std::chrono::milliseconds(0));
													}
													pQuery->Release();
												}

												//d3d 멀티스레드 허용 해제
												ID3D10Multithread* multithread = nullptr;
												RendererUtility::GetDevice()->QueryInterface(__uuidof(ID3D10Multithread), (void**)&multithread);
												if (multithread)
												{
													multithread->SetMultithreadProtected(FALSE);
													multithread->Release();
												}
											}
										});

	}

	loadPath = path;
	isLoaded.emplace_back(std::async(std::launch::async, [isStatic, surface]()
						  {
							  progress = 0.f;
							  Utility::CheckHRESULT(CoInitializeEx(nullptr, COINIT_MULTITHREADED)); //작업 스레드의 Com 객체 사용 활성화
							  ret = LoadFBXfunc(loadPath.c_str(), isStatic, surface, &progress);
							  CoUninitialize();
						  }));

}

GameObject* Utility::LoadFBXfunc(const wchar_t* path,
	bool isStatic,
	SURFACE_TYPE surface,
	float* progressOut)
{
	using namespace utfConvert;

	Assimp::Importer importer;
	unsigned int importFlags =
		aiProcess_Triangulate |         // vertex 삼각형 으로 출력
		aiProcess_GenNormals |          // Normal 정보 생성  
		aiProcess_GenUVCoords |         // 텍스처 좌표 생성
		aiProcess_CalcTangentSpace |    // 탄젠트 벡터 생성
		aiProcess_LimitBoneWeights |  // 본 영향 정점 개수 제한
		aiProcess_ConvertToLeftHanded;  // DX용 왼손좌표계 변환

	if (isStatic)
		importFlags |= aiProcess_PreTransformVertices;   // 노드의 변환행렬을 적용한 버텍스 생성한다.  *사용하면 모든 메쉬가 합쳐져서 애니메이션 사용이 불가능하다.

	importer.SetPropertyBool(AI_CONFIG_IMPORT_FBX_PRESERVE_PIVOTS, 0);    // $assimp_fbx$ 노드 생성안함
	importer.SetPropertyInteger(AI_CONFIG_PP_LBW_MAX_WEIGHTS, 12);

	std::wstring wstr_path = std::filesystem::path(path).filename().c_str();
	std::string str_path = wstring_to_utf8(path);
	std::filesystem::path resourcePath(GetTempResourcePath(wstr_path.c_str(), isStatic));

	//첫 로드시 반드시 리소스 생성
	static std::unordered_set<std::wstring> resourceSetFlag;
	auto find = resourceSetFlag.find(wstr_path);
	if (find != resourceSetFlag.end())
	{
		if (std::filesystem::exists(resourcePath)) //리소스 존재 확인
		{
			if (progressOut)
				*progressOut = 0.7f;

			GameObject* obj = gameObjectFactory.DeserializedObject(resourcePath.c_str());

			if (progressOut)
				*progressOut = 1.0f;
			return obj;
		}
	}
	else
	{
		if (!std::filesystem::exists(resourcePath.parent_path()))
		{
			std::filesystem::create_directories(resourcePath.parent_path());
		}
	}
	resourceSetFlag.insert(wstr_path);

	const aiScene* pScene = importer.ReadFile(str_path, importFlags);
	if (pScene == nullptr)
	{
		__debugbreak(); //파일이 존재하지 않음.
		return nullptr;
	}	
	std::wstring directory = utfConvert::utf8_to_wstring(str_path.substr(0, str_path.find_last_of("/\\")));

	std::queue<aiNode*> nodeQue;
	nodeQue.push(pScene->mRootNode);
	const aiNode* currNode = nullptr;

	std::queue<GameObject*> objQue;
	GameObject* rootObject = NewMeshObject(surface, wstr_path.c_str());
	objQue.push(rootObject);
	GameObject* currObj = nullptr;

	std::unordered_map<std::wstring, GameObject*> addObjMap;
	std::unordered_map<std::wstring, int> boneIndexMap(128);
	int boneIndexManager = 0;
	auto getBoneIndex = [&boneIndexMap, &boneIndexManager](const std::wstring& name)->int
		{
			auto findIter = boneIndexMap.find(name);
			if (findIter != boneIndexMap.end())
				return findIter->second;
			else
			{
				boneIndexMap[name] = boneIndexManager;
				return boneIndexManager++;
			}
		};

	//count bone
	int boneCount = 0;
	for (unsigned int i = 0; i < pScene->mNumMeshes; ++i)
	{
		aiMesh* mesh = pScene->mMeshes[i];
		for (unsigned int j = 0; j < mesh->mNumBones; ++j)
		{
			getBoneIndex(utf8_to_wstring(mesh->mBones[j]->mName.C_Str()));
		}
	}
	boneCount = boneIndexMap.size();

	std::vector<SimpleBoneMeshRender*> meshList;
	TransformAnimation* transformAnimation = nullptr;
	//set animation
	if (pScene->mAnimations)
	{
		transformAnimation = SetTransformAnimation(pScene, *rootObject, addObjMap);
	}

	if (progressOut)
	{
		*progressOut = 0.3f;
	}

	while (!nodeQue.empty())
	{
		currNode = nodeQue.front();
		nodeQue.pop();

		currObj = objQue.front();
		objQue.pop();

		addObjMap[currObj->Name] = currObj;

		if (currNode)
		{			
			std::wstring currNodeName = utf8_to_wstring(currNode->mName.C_Str());
			if (boneCount > 0)
			{				
				if (currNode->mNumMeshes > 0) 
				{			
					for (unsigned int i = 0; i < currNode->mNumMeshes; i++)
					{
						SimpleBoneMeshRender& meshComponent = AddBoneMeshComponent(currObj, surface);
						meshList.push_back(&meshComponent);

						unsigned int meshIndex = currNode->mMeshes[i];
						std::wstring OffsetMatricesKey = currNodeName + std::to_wstring(meshIndex);

						aiMesh* pMesh = pScene->mMeshes[meshIndex];		
						aiMaterial* pMaterial = pScene->mMaterials[pMesh->mMaterialIndex];
						std::string materialName = pMaterial->GetName().C_Str();
						
						//offsetMatrices
						meshComponent.offsetMatrices = GetResourceManager<OffsetMatrices>().GetResource(OffsetMatricesKey.c_str());
						meshComponent.offsetMatrices->data.resize(boneCount);

						for (unsigned int i = 0; i < pMesh->mNumBones; i++)
						{
							aiMatrix4x4 ai_matrix = pMesh->mBones[i]->mOffsetMatrix;
							std::wstring name = utf8_to_wstring(pMesh->mBones[i]->mName.C_Str());
							meshComponent.offsetMatrices->data[getBoneIndex(name)] = Matrix(&ai_matrix.a1).Transpose();
						}

						//cash vertex bone info
						std::unordered_map<int, std::vector<float>>		 weightsMap;
						std::unordered_map<int, std::list<std::wstring>> nameMap;
						if (pMesh->mNumVertices > 0)
						{
							weightsMap.reserve(pMesh->mNumVertices);
							nameMap.reserve(pMesh->mNumVertices);
						}
						for (unsigned int j = 0; j < pMesh->mNumBones; j++)
						{
							aiBone* pBone = pMesh->mBones[j];
							for (unsigned int k = 0; k < pBone->mNumWeights; k++)
							{
								aiVertexWeight weight = pBone->mWeights[k];
								weightsMap[weight.mVertexId].emplace_back(weight.mWeight);
								nameMap[weight.mVertexId].emplace_back(utfConvert::utf8_to_wstring(pBone->mName.C_Str()));				
							}
						}

						bool noWeightVertex = false;
						for (unsigned int vertexIndex = 0; vertexIndex < pMesh->mNumVertices; vertexIndex++)
						{
							SimpleBoneMeshRender::Vertex vertex;

							Matrix vertexWorld(&currNode->mTransformation.a1);
							vertexWorld = vertexWorld.Transpose();
							Matrix vertexWIT = vertexWorld.Invert().Transpose();

							aiVector3D position = pMesh->mVertices[vertexIndex];
							vertex.position.x = position.x;
							vertex.position.y = position.y;
							vertex.position.z = position.z;
							vertex.position = Vector4::Transform(vertex.position, vertexWorld);
							vertex.position.w = 1.0f;

							aiVector3D normal = pMesh->mNormals[vertexIndex];
							vertex.normal.x = normal.x;
							vertex.normal.y = normal.y;
							vertex.normal.z = normal.z;
							vertex.normal = Vector3::Transform(vertex.normal, vertexWIT);
							vertex.normal.Normalize();

							aiVector3D Tangents = pMesh->mTangents[vertexIndex];
							vertex.Tangent.x = Tangents.x;
							vertex.Tangent.y = Tangents.y;
							vertex.Tangent.z = Tangents.z;
							vertex.Tangent = Vector3::Transform(vertex.Tangent, vertexWIT);
							vertex.Tangent.Normalize();

							aiVector3D texCoord = pMesh->mTextureCoords[0][vertexIndex];
							vertex.Tex.x = texCoord.x;
							vertex.Tex.y = texCoord.y;
							
							size_t weightsCount = weightsMap[vertexIndex].size();
							float weightSum = 0.f;
							if (weightsCount == 0)
							{
								//가중치 없는 애들은 루트 본 사용
								vertex.BlendWeights[0] = 1.f;
								vertex.BlendIndecses[0] = 0;
								weightSum = 1.f;
								noWeightVertex = true;
							}
							else
							{
								//가중치 적용						
								auto n = nameMap[vertexIndex].begin();
								for (int j = 0; j < weightsCount; j++)
								{
									vertex.BlendWeights[j] = weightsMap[vertexIndex][j];
									vertex.BlendIndecses[j] = getBoneIndex(*n);
									weightSum += vertex.BlendWeights[j];
									n++;
								}
							}
							//가중치 정규화
							for (int j = 0; j < weightsCount; j++)
							{
								vertex.BlendWeights[j] /= weightSum; // 비율 유지하며 정규화
							}
							meshComponent.vertices.push_back(vertex);	 							
						}

						//Create Index
						for (unsigned int faceIndex = 0; faceIndex < pMesh->mNumFaces; faceIndex++)
						{
							const aiFace& face = pMesh->mFaces[faceIndex];
							for (unsigned int numIndex = 0; numIndex < face.mNumIndices; numIndex++)
							{
								meshComponent.indices.push_back(face.mIndices[numIndex]);
							}
						}
			
						meshComponent.SetMeshID(materialName);
						meshComponent.CreateMesh();

						//Load Texture
						aiMaterial* ai_material = pScene->mMaterials[pMesh->mMaterialIndex];
						LoadTexture(ai_material, directory.c_str(), &meshComponent, rootObject, surface);

						//로그 작성
						if (noWeightVertex)
						{
							std::string logMessage = std::format("LoadFBX Warning : The \"{}\" file's '{}' mesh contains vertices with no bone weight assigned. (Adjusted to 1.)", 
								wstring_to_utf8(std::filesystem::path(wstr_path).filename()), 
								materialName);
							Debug_printf("%s\n", logMessage.c_str());
							SQLiteLogger::EditorLog("Warning", logMessage.c_str());
						}
					}
				}
			}
			else
			{
				if (currNode->mNumMeshes > 0)
				{
					//Create Vertex
					for (unsigned int i = 0; i < currNode->mNumMeshes; i++)
					{
						SimpleMeshRender& meshComponent = AddMeshComponent(currObj, surface);

						unsigned int meshIndex = currNode->mMeshes[i];
						aiMesh* pMesh = pScene->mMeshes[meshIndex];

						aiMaterial* pMaterial = pScene->mMaterials[pMesh->mMaterialIndex];
						std::string materialName = pMaterial->GetName().C_Str();
						
						for (unsigned int vertexIndex = 0; vertexIndex < pMesh->mNumVertices; vertexIndex++)
						{
							Matrix vertexWorld(&currNode->mTransformation.a1);
							vertexWorld = vertexWorld.Transpose();
							Matrix vertexWIT = vertexWorld.Invert().Transpose();

							SimpleMeshRender::Vertex vertex;
							aiVector3D position = pMesh->mVertices[vertexIndex];
							vertex.position.x = position.x;
							vertex.position.y = position.y;
							vertex.position.z = position.z;
							vertex.position = Vector4::Transform(vertex.position, vertexWorld);
							vertex.position.w = 1.0f;
		
							aiVector3D normal = pMesh->mNormals[vertexIndex];
							vertex.normal.x = normal.x;
							vertex.normal.y = normal.y;
							vertex.normal.z = normal.z;
							vertex.normal = Vector3::Transform(vertex.normal, vertexWIT);
							vertex.normal.Normalize();

							aiVector3D Tangents = pMesh->mTangents[vertexIndex];
							vertex.Tangent.x = Tangents.x;
							vertex.Tangent.y = Tangents.y;
							vertex.Tangent.z = Tangents.z;
							vertex.Tangent = Vector3::Transform(vertex.Tangent, vertexWIT);
							vertex.Tangent.Normalize();

							aiVector3D texCoord = pMesh->mTextureCoords[0][vertexIndex];
							vertex.Tex.x = texCoord.x;
							vertex.Tex.y = texCoord.y;

							meshComponent.vertices.push_back(vertex);
						}

						//Create Index
						for (unsigned int faceIndex = 0; faceIndex < pMesh->mNumFaces; faceIndex++)
						{
							const aiFace& face = pMesh->mFaces[faceIndex];
							for (unsigned int numIndex = 0; numIndex < face.mNumIndices; numIndex++)
							{
								meshComponent.indices.push_back(face.mIndices[numIndex]);
							}
						}

						meshComponent.SetMeshID(materialName);
						meshComponent.CreateMesh();

						//Load Texture
						aiMaterial* ai_material = pScene->mMaterials[pMesh->mMaterialIndex];
						LoadTexture(ai_material, directory.c_str(), &meshComponent, rootObject, surface);
					}
				}
			}
			
			//create childs
			for (unsigned int i = 0; i < currNode->mNumChildren; i++)
			{
				nodeQue.push(currNode->mChildren[i]);
				std::wstring childName = utf8_to_wstring(currNode->mChildren[i]->mName.C_Str());
				GameObject* childObj = nullptr;
				auto findIndex = boneIndexMap.find(childName);
				if (findIndex != boneIndexMap.end())
				{
					BoneObject* chidBone = NewGameObject<BoneObject>(childName.c_str());
					int index = findIndex->second;
					chidBone->myIndex = index;
					childObj = chidBone;
				}
				else
				{
					childObj = NewMeshObject(surface, childName.c_str());
				}			
				childObj->transform.SetParent(currObj->transform, false);
				objQue.push(childObj);
			}
		}
	}

	if (progressOut)
	{
		*progressOut = 0.7f;
	}

	//set bone info
	for (auto& mesh : meshList)
	{
		mesh->AddBonesFromRoot();
	}

	//set Target
	if (transformAnimation)
	{
		transformAnimation->AddChildrenToTargets();
	}

	//행렬 업데이트 등록
	rootObject->transform.PushUpdateList();

	if (progressOut)
	{
		*progressOut = 0.9f;
	}

	//리소스 등록
	gameObjectFactory.SerializedObject(rootObject, resourcePath.c_str(), true); //리소스 직렬화.

	if (progressOut)
	{
		*progressOut = 1.0f;
	}

	return rootObject;
}

void Utility::LoadFBXResource(const wchar_t* path,
	bool isStatic,
	SURFACE_TYPE surface)
{
	GameObject* destory = nullptr;
	LoadFBX(path, isStatic, surface, &destory);
	ImGui::DestroyObjTextureCompressEnd(destory);
}

std::filesystem::path Utility::GetTempResourcePath(const wchar_t* flieName, bool isStatic)
{
	//임시 리소스 경로
	wchar_t tempPath[MAX_PATH];
	GetTempPath(MAX_PATH, tempPath);
	std::filesystem::path resourcePath = tempPath;

	//파일 이름 설정
	GetModuleFileName(nullptr, tempPath, MAX_PATH);
	resourcePath /= L"power12562";
	resourcePath /= WinGameApp::GetAppName();
	resourcePath /= isStatic ? L"Static" : L"Dynamic";
	resourcePath /= std::filesystem::path(flieName).filename();
	resourcePath.replace_extension(L".GameObject");

	return resourcePath;
}
