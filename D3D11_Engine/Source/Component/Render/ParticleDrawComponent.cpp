#include "ParticleDrawComponent.h"
#include <Manager/ResourceManager.h>
#include <Manager/HLSLManager.h>
#include <Manager/TextureManager.h>
#include <Utility/MemoryUtility.h>
#include <ranges>

#include <format>
#include <Utility/ImguiHelper.h>
#include <Manager/SceneManager.h>
#include <NodeEditor/NodeEditor.h>
#include <framework.h>
#include <Utility/SerializedUtility.h>
#include <random>
#include <utility/SQLiteLogger.h>


ParticleSpawnComponent::ParticleSpawnComponent() = default;
ParticleSpawnComponent::~ParticleSpawnComponent() = default;

void ParticleSpawnComponent::Awake()
{
	particleDrawComponent = &gameObject.AddComponent<ParticleDrawComponent>();
	particleDrawComponent->deadParticleProcess = 
		[this](const auto& item) 
		{
			if (subEmitter)
			{
				subEmitter->CreateParticle(item.position);
			}
		};
	


}

void ParticleSpawnComponent::Start()
{
	if (subEmitterName.empty()) return;

	auto findObject = GameObject::Find(subEmitterName.c_str());
	if (findObject)
	{
		auto findSubEmitter = findObject->GetComponentAtIndex<ParticleSpawnComponent>(subEmitterComponentIndex);
		if (findSubEmitter)
		{
			subEmitter = findSubEmitter;
		}
		else
		{
			Debug_printf("SubEmitter is not found");
			SQLiteLogger::EditorLog("Log", "SubEmitter is not found");
		}
	}


}

void ParticleSpawnComponent::Serialized(std::ofstream& ofs)
{
	Binary::Write::data(ofs, std::numeric_limits<uint32_t>::max());
	Binary::Write::data(ofs, 4);

	Binary::Write::data(ofs, spawnParticleAmount);
	Binary::Write::data(ofs, spawnParticleAmountRange[0]);
	Binary::Write::data(ofs, spawnParticleAmountRange[1]);

	Binary::Write::data(ofs, spawnInterval);
	Binary::Write::data(ofs, spawnIntervalRange[0]);
	Binary::Write::data(ofs, spawnIntervalRange[1]);

	Binary::Write::Vector3(ofs, position);
	Binary::Write::Vector3(ofs, positionRange[0]);
	Binary::Write::Vector3(ofs, positionRange[1]);

	Binary::Write::Vector3(ofs, velocity);
	Binary::Write::Vector3(ofs, velocityRange[0]);
	Binary::Write::Vector3(ofs, velocityRange[1]);

	Binary::Write::Vector3(ofs, acceleration);
	Binary::Write::Vector3(ofs, accelerationRange[0]);
	Binary::Write::Vector3(ofs, accelerationRange[1]);

	Binary::Write::data(ofs, lifeTime);
	Binary::Write::data(ofs, lifTimeRange[0]);
	Binary::Write::data(ofs, lifTimeRange[1]);

	Binary::Write::data(ofs, isSpawnParticlesByTime);
	Binary::Write::wstring(ofs, subEmitterName);
	Binary::Write::data(ofs, subEmitterComponentIndex);

	// version 1
	Binary::Write::Vector3(ofs, worldPosition);
	Binary::Write::Quaternion(ofs, worldRotation);
	Binary::Write::Vector3(ofs, worldScale);
	// ~versio 1

	// version 2
	Binary::Write::data(ofs, isTramsformParent);
	// ~versio 2

	// version 3
	Binary::Write::data(ofs, rotation);
	Binary::Write::data(ofs, rotationRange[0]);
	Binary::Write::data(ofs, rotationRange[1]);
	// ~versio 3

	// version 4
	Binary::Write::data(ofs, particleDrawComponent->instanceData.radiance);
	Binary::Write::data(ofs, particleDrawComponent->particleOption.rotationIndex);
	// ~versio 4

}

void ParticleSpawnComponent::Deserialized(std::ifstream& ifs)
{
	int version = 0;
	auto header = Binary::Read::data<uint32_t>(ifs);
	if (header == std::numeric_limits<uint32_t>::max())
	{
		version = Binary::Read::data<int>(ifs);
		spawnParticleAmount = Binary::Read::data<uint32_t>(ifs);
	}
	else
	{
		spawnParticleAmount = header;
	}
	spawnParticleAmountRange[0] = Binary::Read::data<uint32_t>(ifs);
	spawnParticleAmountRange[1] = Binary::Read::data<uint32_t>(ifs);

	spawnInterval = Binary::Read::data<float>(ifs);
	spawnIntervalRange[0] = Binary::Read::data<float>(ifs);
	spawnIntervalRange[1] = Binary::Read::data<float>(ifs);

	position = Binary::Read::Vector3(ifs);
	positionRange[0] = Binary::Read::Vector3(ifs);
	positionRange[1] = Binary::Read::Vector3(ifs);

	velocity = Binary::Read::Vector3(ifs);
	velocityRange[0] = Binary::Read::Vector3(ifs);
	velocityRange[1] = Binary::Read::Vector3(ifs);

	acceleration = Binary::Read::Vector3(ifs);
	accelerationRange[0] = Binary::Read::Vector3(ifs);
	accelerationRange[1] = Binary::Read::Vector3(ifs);

	lifeTime = Binary::Read::data<float>(ifs);
	lifTimeRange[0] = Binary::Read::data<float>(ifs);
	lifTimeRange[1] = Binary::Read::data<float>(ifs);

	isSpawnParticlesByTime = Binary::Read::data<bool>(ifs);
	subEmitterName = Binary::Read::wstring(ifs);
	subEmitterComponentIndex = Binary::Read::data<int>(ifs);

	if (version >= 1)
	{
		worldPosition = Binary::Read::Vector3(ifs);
		worldRotation = Binary::Read::Quaternion(ifs);
		worldScale  = Binary::Read::Vector3(ifs);
	}
	if (version >= 2)
	{
		isTramsformParent = Binary::Read::data<bool>(ifs);

	}
	if (version >= 3)
	{
		rotation = Binary::Read::data<float>(ifs);
		rotationRange[0] = Binary::Read::data<float>(ifs);
		rotationRange[1] = Binary::Read::data<float>(ifs);
	}
	if (version >= 4)
	{
		particleDrawComponent->instanceData.radiance = Binary::Read::data<float>(ifs);
		particleDrawComponent->particleOption.rotationIndex = Binary::Read::data<int>(ifs);
	}


	auto findObject = GameObject::Find(subEmitterName.c_str());
	if (findObject)
	{
		auto findSubEmitter = findObject->GetComponentAtIndex<ParticleSpawnComponent>(subEmitterComponentIndex);
		if (findSubEmitter)
		{
			subEmitter = findSubEmitter;
		}
		else
		{
			Debug_printf("SubEmitter is not found");
			SQLiteLogger::EditorLog("Log", "SubEmitter is not found");
		}
	}


}

void ParticleSpawnComponent::InspectorImguiDraw()
{
	if (ImGui::TreeNode("ParticleSpawnComponent"))
	{

		ImGui::BeginChild("SubEmitor", ImVec2(0, 0), ImGuiChildFlags_AlwaysAutoResize | ImGuiChildFlags_AutoResizeY | ImGuiChildFlags_Borders);
		ImGui::Text(subEmitter ? std::filesystem::path(subEmitter->gameObject.Name).string().c_str() : "Null");
		ImGui::EndChild(); // Child 끝내기
		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DRAG_TRANSFORM"))
			{
				Transform* droppedTransform = *(Transform**)payload->Data;
				auto findSubEmitter = droppedTransform->gameObject.IsComponent<ParticleSpawnComponent>();
				if (findSubEmitter && findSubEmitter != this)
				{
					subEmitter = findSubEmitter;
					subEmitterName = findSubEmitter->gameObject.Name;
					subEmitterComponentIndex = droppedTransform->gameObject.GetComponentIndex(findSubEmitter);
				}
			}
			ImGui::EndDragDropTarget();
		}
		bool isEdit = false;

		ImGui::DragFloat("radiance", &particleDrawComponent->instanceData.radiance, 0.1f);
		ImGui::Combo("Rotation Mode", (int*)&particleDrawComponent->particleOption.rotationIndex, "billboard\0Velocity\0None\0");
		//bool isVelocityRotate = particleDrawComponent->particleOption.rotationIndex;
		//ImGui::Checkbox("isVeclotiyRotation", &isVelocityRotate);
		//particleDrawComponent->particleOption.isVeclotiyRotation = isVelocityRotate;
		isEdit |= ImGui::DragVector3("position", &worldPosition, 0.1f);
		isEdit |= ImGui::DragQuaternionLocal("Rotation", &worldRotation);
		isEdit |= ImGui::DragVector3("Scale", &worldScale, 0.1f);


		ImGui::Checkbox("isTramsformParent", &isTramsformParent);
		ImGui::Checkbox("Spawn Particles By Time", &isSpawnParticlesByTime);

		ImGui::DragInt("Spawn Particle Amount", (int*)&spawnParticleAmount);
		ImGui::DragInt2("Spawn Particle Amount Range", (int*)spawnParticleAmountRange);

		ImGui::DragFloat("Spawn Interval", &spawnInterval);
		ImGui::DragFloat2("Spawn Interval Range", spawnIntervalRange);

		ImGui::DragVector3("Position", &position);
		ImGui::DragVector3("Position Range Left", &positionRange[0]);
		ImGui::DragVector3("Position Range Right", &positionRange[1]);

		ImGui::DragVector3("Velocity", &velocity);
		ImGui::DragVector3("Velocity Range Left", &velocityRange[0]);
		ImGui::DragVector3("Velocity Range Right", &velocityRange[1]);

		ImGui::DragVector3("Acceleration", &acceleration);
		ImGui::DragVector3("Acceleration Range Left", &accelerationRange[0]);
		ImGui::DragVector3("Acceleration Range Right", &accelerationRange[1]);

		ImGui::DragFloat("Life Time", &lifeTime);
		ImGui::DragFloat2("Life Time Range", lifTimeRange);

		ImGui::DragFloat("Rotation", &rotation);
		ImGui::DragFloat("Rotation Range Left", &rotationRange[0]);
		ImGui::DragFloat("Rotation Range Right", &rotationRange[1]);



		ImGui::TreePop();
	}

}

void ParticleSpawnComponent::Update()
{
	if (!isSpawnParticlesByTime) return;

	static float spawnTime = 0;
	spawnTime += TimeSystem::Time.DeltaTime;
	if (nextSpawnTime >= 0 && spawnTime >= nextSpawnTime)
	{
		std::random_device rd;
		std::mt19937 gen(rd());
		nextSpawnTime = std::max(spawnInterval + std::uniform_real_distribution<float>(spawnIntervalRange[0], spawnIntervalRange[1])(gen), 0.0f);

		CreateParticle(position);

		spawnTime = 0;

	}


	Matrix parent{};
	if (isTramsformParent)
	{
		parent = gameObject.transform.GetWM();
	}
	particleDrawComponent->instanceData.world = 
		(Matrix::CreateScale(worldScale) 
		 * Matrix::CreateFromQuaternion(worldRotation) 
		 * Matrix::CreateTranslation(worldPosition)
		 * parent).Transpose();
}

void ParticleSpawnComponent::CreateParticle(const Vector3& position, const Vector3& velocity, const Vector3& acceleration, float rotation)
{

	auto uniform_int_distributionHelper =
		[]<typename T>(const T & item, const T & item2) -> std::uniform_int_distribution<T>
	{
		auto minmax = std::minmax(item, item2);
		return std::uniform_int_distribution<T>(minmax.first, minmax.second);
	};
	auto uniform_real_distributionHelper =
		[]<typename T>(const T & item, const T & item2) -> std::uniform_real_distribution<T>
	{
		auto minmax = std::minmax(item, item2);
		return std::uniform_real_distribution<T>(minmax.first, minmax.second);
	};

	std::random_device rd;
	std::mt19937 gen(rd());

	auto dis = uniform_int_distributionHelper(spawnParticleAmount + spawnParticleAmountRange[0], spawnParticleAmount + spawnParticleAmountRange[1]);
	uint32_t spawnAmount = dis(gen);


	for (size_t i = 0; i < spawnAmount; i++)
	{
		Vector3 randomPosition;
		{
			auto disX = uniform_real_distributionHelper(position.x + positionRange[0].x, position.x + positionRange[1].x);
			auto disY = uniform_real_distributionHelper(position.y + positionRange[0].y, position.y + positionRange[1].y);
			auto disZ = uniform_real_distributionHelper(position.z + positionRange[0].z, position.z + positionRange[1].z);

			randomPosition = { disX(gen), disY(gen), disZ(gen) };
		}
		Vector3 randomVelocity;
		{
			auto disX = uniform_real_distributionHelper(velocity.x + velocityRange[0].x, velocity.x + velocityRange[1].x);
			auto disY = uniform_real_distributionHelper(velocity.y + velocityRange[0].y, velocity.y + velocityRange[1].y);
			auto disZ = uniform_real_distributionHelper(velocity.z + velocityRange[0].z, velocity.z + velocityRange[1].z);

			randomVelocity = { disX(gen), disY(gen), disZ(gen) };
		}
		Vector3 randomAcceleration;
		{
			auto disX = uniform_real_distributionHelper(acceleration.x + accelerationRange[0].x, acceleration.x + accelerationRange[1].x);
			auto disY = uniform_real_distributionHelper(acceleration.y + accelerationRange[0].y, acceleration.y + accelerationRange[1].y);
			auto disZ = uniform_real_distributionHelper(acceleration.z + accelerationRange[0].z, acceleration.z + accelerationRange[1].z);

			randomAcceleration = { disX(gen), disY(gen), disZ(gen) };
		}
		float randomRotation;
		{
			auto disRotation = uniform_real_distributionHelper(rotation + rotationRange[0], rotation + rotationRange[1]);
			randomRotation = disRotation(gen);
		}


		std::uniform_real_distribution<float> disLifeTime(lifeTime + lifTimeRange[0], lifeTime + lifTimeRange[1]);
		float randomLifeTime = disLifeTime(gen);
		particleDrawComponent->PushParticle(randomPosition, randomVelocity, randomAcceleration, randomLifeTime, randomRotation);
	}
}



ParticleDrawComponent::ParticleDrawComponent() = default;
ParticleDrawComponent::~ParticleDrawComponent() = default;

void ParticleDrawComponent::Awake()
{
	materialAsset.OpenAsset(GetDefaultMaterialNodePath().c_str());


	instanceData.radiance = 3.0f;
	instanceBuffer.Init(instanceData);
	DrawData drawData{};
	instanceCountBuffer.Init(D3D11_BUFFER_DESC
							 {	
								 .ByteWidth = sizeof(drawData),
								 .Usage = D3D11_USAGE_DEFAULT,
								 .BindFlags = 0,
								 .CPUAccessFlags = 0,
								 .MiscFlags = D3D11_RESOURCE_MISC_DRAWINDIRECT_ARGS
							 }, sizeof(drawData), & drawData);

	instanceCountStagingBuffer.Init(D3D11_BUFFER_DESC
									{
										.ByteWidth = sizeof(drawData),
										.Usage = D3D11_USAGE_STAGING,
										.BindFlags = 0,
										.CPUAccessFlags = D3D11_CPU_ACCESS_READ,
										.MiscFlags = 0
									}, sizeof(drawData), & drawData);


	deadParticleStagingBuffer.Init(D3D11_BUFFER_DESC
								   {
									   .ByteWidth = (UINT)sizeof(ParticleData) * 64,
									   .Usage = D3D11_USAGE_STAGING,
									   .BindFlags = 0,
									   .CPUAccessFlags = D3D11_CPU_ACCESS_READ,
									   .MiscFlags = 0
								   }, sizeof(ParticleData) * 64);


	deadParticleStagingBufferSize = 64;
	particleBuffer[0].CreateBuffer(sizeof(ParticleData), 64, { .isSRV = true, .isUAV = true, .isAppend = true });
	particleBuffer[1].CreateBuffer(sizeof(ParticleData), 64, { .isSRV = true, .isUAV = true, .isAppend = true });
	deadParticleBuffer.CreateBuffer(sizeof(ParticleData), 64, { .isUAV = true, .isAppend = true });
	addParticleBuffer.CreateBuffer(sizeof(ParticleData), 64, { .isSRV = true, .isDynamic = true });

	particleBuffer[0].CreateView(0);
	particleBuffer[1].CreateView(0);
	deadParticleBuffer.CreateView(0);

	particles.emplace_back();
	particles.pop_back();
}

void ParticleDrawComponent::Serialized(std::ofstream& ofs)
{
	using namespace Binary;
	Write::data(ofs, std::numeric_limits<size_t>::max());
	Write::data(ofs, 1);
	Write::wstring(ofs, materialAsset.GetAssetPath());
	Write::data(ofs, instanceData.radiance);
	materialAsset.SaveAsset();
}

void ParticleDrawComponent::Deserialized(std::ifstream& ifs)
{
	using namespace Binary;

	int version = 0;
	size_t len = 0;
	std::wstring data;
	auto header = Binary::Read::data<size_t>(ifs);
	if (header == std::numeric_limits<size_t>::max())
	{
		version = Binary::Read::data<int>(ifs);
		len = Binary::Read::data<size_t>(ifs);
	}
	else
	{
		len = header;
	}

	data.resize(len);
	ifs.read(reinterpret_cast<char*>(data.data()), len * sizeof(wchar_t));
	materialAsset.OpenAsset(data.c_str());


	if (version >= 1)
	{
		instanceData.radiance = Binary::Read::data<float>(ifs);
	}
}

void ParticleDrawComponent::InspectorImguiDraw()
{
	if (Scene* scene = sceneManager.GetActiveScene())
	{
		ImGui::PushID(GetComponentIndex());
		ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.2f, 0.3f, 0.4f, 1.0f)); // 배경색
		ImGui::BeginChild("MeshRenderChild", ImVec2(0, 0), ImGuiChildFlags_AlwaysAutoResize | ImGuiChildFlags_AutoResizeY);
		if (ImGui::TreeNode("ParticleDrawComponent"))
		{
			if (ImGui::Button("Edit Material"))
			{
				auto editorPath = GetDefaultMaterialNodePath();
				ShaderNodeEditor* editor = scene->MakeShaderNodeEditor(editorPath.c_str());
				editor->EndPopupEvent = [scene, editorPath]()
					{
						scene->EraseShaderNodeEditor(editorPath.c_str());
					};
			}
			if (ImGui::Button("Load Material Asset"))
			{
				materialAsset.OpenAssetWithDialog();
			}
			if (ImGui::Button(utfConvert::wstring_to_utf8(materialAsset.GetAssetPath()).c_str()))
			{
				materialAsset.OpenAsset(materialAsset.GetAssetPath().c_str());
			}

			//if (ImGui::Button("Reload Material Asset"))
			//{
			//	materialAsset.OpenAsset(materialAsset.GetAssetPath().c_str());
			//}

			for (auto& item : materialAsset.customData.GetFieldData())
			{
				if (item.second.size == 4)
				{
					ImGui::DragFloat(item.first.c_str(), materialAsset.customData.GetField<float>(item.first));
				}
				else if (item.second.size == 8)
				{
					ImGui::DragVector2(item.first.c_str(), materialAsset.customData.GetField<Vector2>(item.first));
				}
				else if (item.second.size == 12)
				{
					ImGui::DragVector3(item.first.c_str(), materialAsset.customData.GetField<Vector3>(item.first));
				}
				else if (item.second.size == 16)
				{
					ImGui::DragVector4(item.first.c_str(), materialAsset.customData.GetField<Vector4>(item.first));
				}

			}

			ImGui::TreePop();
		}
		ImGui::EndChild(); // Child 끝내기
		ImGui::PopStyleColor(); // 스타일 복구
		ImGui::PopID();
	}

}

void ParticleDrawComponent::FixedUpdate()
{
}

void ParticleDrawComponent::Update()
{
	DynamicBufferModifier modifier;
	DrawData* drawData = (DrawData*)modifier.Map(instanceCountStagingBuffer, sizeof(DrawData), 0, D3D11_MAP_READ);
	if (drawData == nullptr)
		return;

	auto newParticleCount = drawData->VertexCountPerInstance;
	modifier.Unmap();
	int destroyCount = addParticleCount + particleCount - newParticleCount;
	particleCount = newParticleCount;

	if (deadParticleProcess && destroyCount > 0)
	{
		if (deadParticleStagingBufferSize < destroyCount)
		{
			deadParticleStagingBuffer.Init(D3D11_BUFFER_DESC
										   {
											   .ByteWidth = (UINT)sizeof(ParticleData) * destroyCount,
											   .Usage = D3D11_USAGE_STAGING,
											   .BindFlags = 0,
											   .CPUAccessFlags = D3D11_CPU_ACCESS_READ,
											   .MiscFlags = 0
										   }, sizeof(ParticleData) * destroyCount);

			deadParticleBuffer.CreateBuffer(sizeof(ParticleData), destroyCount, { .isUAV = true, .isAppend = true });
			deadParticleBuffer.CreateView(0);

			deadParticleStagingBufferSize = destroyCount;
		}

		DynamicBufferModifier modifier;
		ParticleData* deadParticle = (ParticleData*)modifier.Map(deadParticleStagingBuffer, sizeof(ParticleData) * destroyCount, 0, D3D11_MAP_READ);
		for (size_t i = 0; i < destroyCount; i++)
		{
			deadParticleProcess(deadParticle[i]);
		}
		modifier.Unmap();
	}


}

void ParticleDrawComponent::LateUpdate()
{

}

void ParticleDrawComponent::Render()
{
	addParticleCount = particles.size();
	addParticleBuffer.Set(particles);
	particles.clear();

	if (addParticleCount + particleCount > particleBuffer->GetCapicity())
	{
		// 2의 배수로 증가
		auto newSIze = std::max<uint32_t>(addParticleCount + particleCount, particleBuffer->GetCapicity() * 2);

		particleBuffer[0].CreateBuffer(sizeof(ParticleData), newSIze, { .isSRV = true, .isUAV = true, .isAppend = true });
		particleBuffer[1].CreateBuffer(sizeof(ParticleData), newSIze, { .isSRV = true, .isUAV = true, .isAppend = true });;

		particleBuffer[0].CreateView(0);
		particleBuffer[1].CreateView(0);
	}

	optionBuffer.Set(particleOption);

	drawCommand.PSshaderResources.clear();
	drawCommand.instanceCountBuffer = instanceCountBuffer;
	flipBuffer = !flipBuffer;
	drawCommand.particleBuffer = particleBuffer[flipBuffer ? 1 : 0];
	drawCommand.addParticleBuffer[0] = particleBuffer[flipBuffer ? 0 : 1];
	drawCommand.addParticleBuffer[1] = addParticleBuffer;
	drawCommand.deadParticleBuffer = deadParticleBuffer;
	drawCommand.instanceCount = particleCount;
	drawCommand.optionBuffer = optionBuffer;

	drawCommand.instanceCountStagingBuffer = instanceCountStagingBuffer;
	drawCommand.deadParticleStagingBuffer = deadParticleStagingBuffer;


	drawCommand.addParticleCount = std::max(particleCount, addParticleCount);

	drawCommand.pixelShader = materialAsset.GetPS();

	if (materialAsset.GetCustomBuffer())
	{
		materialAsset.GetCustomBuffer().Update(materialAsset.customData.Data());
		drawCommand.PSshaderResources.push_back(
			Binadble
			{
				.shaderType = EShaderType::Pixel,
				.bindableType = EShaderBindable::ConstantBuffer,
				.slot = 5,
				.bind = (ID3D11Buffer*)materialAsset.GetCustomBuffer()
			}
		);
	}

	drawCommand.PSshaderResources.emplace_back(
		Binadble
		{
			.shaderType = EShaderType::Vertex,
			.bindableType = EShaderBindable::ShaderResource,
			.slot = 0,
			.bind = (ID3D11ShaderResourceView*)drawCommand.particleBuffer
		}
	); 


	instanceBuffer.Update(instanceData);
	drawCommand.PSshaderResources.emplace_back(
		Binadble
		{
			.shaderType = EShaderType::Geometry,
			.bindableType = EShaderBindable::ConstantBuffer,
			.slot = 4,
			.bind = (ID3D11Buffer*)instanceBuffer
		}
	);

	//텍스쳐 등록
	size_t textureCount = materialAsset.GetTexturesV2().size();
	const auto& textures = materialAsset.GetTexturesV2();
	const auto& textureSlot = materialAsset.GetTexturesSlot();
	for (size_t i = 0; i < textureCount; i++)
	{
		Binadble bind{};
		bind.bindableType = EShaderBindable::ShaderResource;
		bind.shaderType = EShaderType::Pixel;
		bind.slot = textureSlot[i];
		bind.bind = (ID3D11ShaderResourceView*)textures[i];
		drawCommand.PSshaderResources.push_back(bind);
	}
	D3D11_GameApp::GetRenderer().AddDrawCommand(drawCommand);
}

std::wstring ParticleDrawComponent::GetDefaultMaterialNodePath()
{
	return L"Resource/Materials/Temp/DefaultParticle.MaterialAsset";
}

void ParticleDrawComponent::PushParticle(const Vector3& position, const Vector3& velocity, const Vector3& acceleration, float lifeTime, float rotation)
{

	auto rotationMatrix = DirectX::XMMatrixRotationZ(rotation);
	float3x3 rotationMatrix3x3{};
	XMStoreFloat3x3(&rotationMatrix3x3, rotationMatrix);
	particles.push_back(
		ParticleData
		{
			.position = position,
			.lifeTime = lifeTime,
			.velocity = velocity,
			.acceleration = acceleration,
			.rotation = rotationMatrix3x3
		});

}

