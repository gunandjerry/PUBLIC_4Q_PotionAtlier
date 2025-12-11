#pragma once
#include <Component/Base/RenderComponent.h>


#include <Manager/InputManager.h>	
#include <vector>

#include <DrawCommand.h>

struct PostProcessData 
{
	virtual ~PostProcessData() = default;
	virtual void InspectorImguiDraw();
	virtual void Serialized(std::ofstream& ofs) {};
	virtual void Deserialized(std::ifstream& ifs) {};
	virtual std::string_view GetTypeName() = 0;

public:
	PostProcesCommand postProcesCommand;
	/** 옵션을위한 상수버퍼  */
	ConstantBuffer cnstantBuffer;
	int drawSpeed;
};
#define REGISTER_POST_PROCESS_DATA(type) 									\
virtual std::string_view GetTypeName() override { return #type; }			\
static inline bool isRegistered =											\
PostProcessDataFactory::Register(#type, 									 \
[]() 																			\
{ 																					\
	return std::dynamic_pointer_cast<PostProcessData>(std::make_shared<type>());	\
}); 																				\

struct PostProcessDataFactory 
{
public:
	static std::shared_ptr<PostProcessData> Create(std::string_view typeName);
	static bool Register(std::string_view typeName, std::function<std::shared_ptr<PostProcessData>()> function);
	static auto GetFactory() { return postProcessDataFactory; }

private:
	static inline std::map<std::string, std::function<std::shared_ptr<PostProcessData>()>> postProcessDataFactory{};
};





class PostProcessComponent : public RenderComponent
{
public:
	PostProcessComponent();
	virtual ~PostProcessComponent() override;

public:
	virtual void Awake() override;
	virtual void InspectorImguiDraw() override;
	virtual void Serialized(std::ofstream& ofs) override;
	virtual void Deserialized(std::ifstream& ifs) override;

public:
	template<typename T>
	void AddPostProcessData();
	template<typename T>
	void PopPostProcessData();
	template<typename T>
	T* GetPostProcessData();
protected:
	virtual void FixedUpdate() override;
	virtual void Update() override;
	virtual void LateUpdate() override;
	virtual void Render() override;

public:
	std::vector<std::shared_ptr<PostProcessData>> postProcessDatas;
};


template<typename T>
inline void PostProcessComponent::AddPostProcessData()
{
	postProcessDatas.emplace_back(std::make_shared<T>());
}

template<typename T>
inline void PostProcessComponent::PopPostProcessData()
{
	std::erase_if(postProcessDatas,
				  [](const std::shared_ptr<PostProcessData>& data)
				  {
					  return dynamic_cast<T*>(data.get());
				  });
}

template<typename T>
inline T* PostProcessComponent::GetPostProcessData()
{
	for (auto& item : postProcessDatas)
	{
		if (dynamic_cast<T*>(item.get()))
		{
			return dynamic_cast<T*>(item.get());
		}
	}
	return nullptr;
}



struct ColorGradingTexture
{
	static Texture Get(std::filesystem::path texturePath, _Out_opt_ Texture* originTexture = nullptr);
};

struct ColorGrading : public PostProcessData
{
public:
	ColorGrading();
	virtual ~ColorGrading();
	REGISTER_POST_PROCESS_DATA(ColorGrading)

public:
	virtual void InspectorImguiDraw() override;
	virtual void Serialized(std::ofstream& ofs) override;
	virtual void Deserialized(std::ifstream& ifs) override;

	void SetTexture(std::filesystem::path path);
private:
	std::wstring texturePath;
	
};

struct ToneMapping : public PostProcessData
{
public:
	ToneMapping();
	virtual ~ToneMapping();
	REGISTER_POST_PROCESS_DATA(ToneMapping)

public:
	virtual void InspectorImguiDraw() override;
	virtual void Serialized(std::ofstream& ofs) override;
	virtual void Deserialized(std::ifstream& ifs) override;


private:

	struct ToneMappingData
	{
		int toneMappingType{0};
		float exposure{1.0f};
		float pad[2];
	} value;
};