#pragma once
#include <Component/Base/RenderComponent.h>
#include <Asset/MaterialAsset.h>

struct DrawData
{
	UINT VertexCountPerInstance{ 0 };
	UINT InstanceCount{ 1 };
	UINT StartVertexLocation{ 0 };
	UINT StartInstanceLocation{ 0 };
};

struct ParticleData
{
	float3 position;
	float lifeTime;

	float3 velocity;
	float elapsedTime;

	float3 acceleration;
	float pad;

	float3x3 rotation;
};

struct ParticleOption
{
	int rotationIndex;
	float padding[3];
};



/**
 * @brief 자동적으로 파티클 생성을 지원하기위한 파티클 컴포넌트
 */
class ParticleSpawnComponent : public Component
{
public:
	ParticleSpawnComponent();
	virtual ~ParticleSpawnComponent() override;

public:
	virtual void Awake() override;
	virtual void Start() override;
	virtual void Serialized(std::ofstream& ofs) override;
	virtual void Deserialized(std::ifstream& ifs) override;
	virtual void InspectorImguiDraw();

public:
	virtual void Update() override;
	virtual void FixedUpdate() override {}
	virtual void LateUpdate() override {}

	void CreateParticle(const Vector3& position, const Vector3& velocity, const Vector3& acceleration, float rotation);
	void CreateParticle(const Vector3& position)
	{
		CreateParticle(position, velocity, acceleration, rotation);
	}
	void CreateParticle()
	{
		CreateParticle(position);
	}

public:
	uint32_t spawnParticleAmount{};
	uint32_t spawnParticleAmountRange[2]{};

	float spawnInterval{};
	float spawnIntervalRange[2]{};

	Vector3 position{};
	Vector3 positionRange[2]{};

	Vector3 velocity{};
	Vector3 velocityRange[2]{};

	Vector3 acceleration{};
	Vector3 accelerationRange[2]{};


	float lifeTime{};
	float lifTimeRange[2]{};
	
	Vector3 worldPosition{};
	Quaternion worldRotation{};
	Vector3 worldScale{1.0f,1.0f ,1.0f };

	float rotation{};
	float rotationRange[2]{};

	bool isSpawnParticlesByTime{};
	bool isTramsformParent{};
	bool isVelocityRotate{};
	bool velocityType;

private:
	float nextSpawnTime{};
	std::wstring subEmitterName{};
	int subEmitterComponentIndex{};
	ParticleSpawnComponent* subEmitter{};
	class ParticleDrawComponent* particleDrawComponent{};
};

/**
 * @brief 프로그래머가 직접 사용가능한 파티클 컴포넌트
 */
class ParticleDrawComponent : public RenderComponent
{
	struct InstanceData
	{
		Matrix world;
		float radiance{ 3.0f };
		float pad[3];
	};


public:
	ParticleDrawComponent();
	virtual ~ParticleDrawComponent() override;

public:
	virtual void Awake() override;
	virtual void Serialized(std::ofstream& ofs) override;
	virtual void Deserialized(std::ifstream& ifs) override;
	virtual void InspectorImguiDraw();

	void PushParticle(const Vector3& position, const Vector3& velocity, const Vector3& acceleration, float lifeTime, float rotation);
	ParticleOption particleOption{};
	InstanceData instanceData{};
	std::function<void(const ParticleData& particle)> deadParticleProcess;
protected:
	virtual void FixedUpdate() override;
	virtual void Update() override;
	virtual void LateUpdate() override;
	virtual void Render() override;

	//virtual void CreateDefaultMaterial();
	//std::unique_ptr<ShaderNodeEditor> GetDefaultNodeEditor();
	std::wstring GetDefaultMaterialNodePath();


private:
	std::vector< ParticleData> particles{};

	ConstantBuffer instanceBuffer{};
	ConstantBuffer optionBuffer{};

	StructuredBuffer particleBuffer[2]{};
	StructuredBuffer addParticleBuffer{};
	StructuredBuffer deadParticleBuffer;
	
	RendererBuffer instanceCountBuffer{};
	RendererBuffer instanceCountStagingBuffer{};
	RendererBuffer deadParticleStagingBuffer{};


	ParticleDrawCommand drawCommand{};
	MaterialAsset materialAsset{};

	int particleCount{};
	int addParticleCount{};
	int deadParticleStagingBufferSize{};
	bool flipBuffer;
};



;