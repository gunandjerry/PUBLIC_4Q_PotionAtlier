#pragma once

#include <Component\Render\PostProcessComponent.h>
#include <Texture.h>



struct EdgeDetection : public PostProcessData
{
public:
	EdgeDetection();
	virtual ~EdgeDetection();
	REGISTER_POST_PROCESS_DATA(EdgeDetection);

public:
	virtual void InspectorImguiDraw() override;
	virtual void Serialized(std::ofstream& ofs) override;
	virtual void Deserialized(std::ifstream& ifs) override;

	struct EdgeDetectionData
	{
		float width{ 3.0f };
		float threadhold{ 0.0f };
		float padp[2];
	} value;
};

struct EdgeDetection2 : public PostProcessData
{
public:
	EdgeDetection2();
	virtual ~EdgeDetection2();
	REGISTER_POST_PROCESS_DATA(EdgeDetection2);

public:
	virtual void InspectorImguiDraw() override;
	virtual void Serialized(std::ofstream& ofs) override;
	virtual void Deserialized(std::ifstream& ifs) override;

	struct EdgeDetection2Data
	{
		float width{ 3.0f };
		float dethThreadhold{ 0.0f };
		float normalThreadhold{ 0.0f };
		float padp[1];
		Vector4 color{ 0.0f, 0.0f, 0.0f, 0.0f };
	} value;
};



struct FXAA : public PostProcessData
{
public:
	FXAA();
	virtual ~FXAA();
	REGISTER_POST_PROCESS_DATA(FXAA);

public:
	virtual void InspectorImguiDraw() override;
	virtual void Serialized(std::ofstream& ofs) override;
	virtual void Deserialized(std::ifstream& ifs) override;
	struct FXAAData
	{
		uint screenWidth;
		uint screenHeight;
		float2 invScreenSize; // = (1.0/width, 1.0/height)

		float FXAA_SPAN_MAX; // 최대 탐색 거리(픽셀 단위)
		float FXAA_REDUCE_MUL; // 감쇠계수 (보정 강도 조절)
		float FXAA_REDUCE_MIN; // 최소 임계값 (에지가 이 값보다 작으면 무시)
		float pad[1];
	} value;
};






struct BlendColorGrading : public PostProcessData
{
public:
	BlendColorGrading();
	virtual ~BlendColorGrading();
	REGISTER_POST_PROCESS_DATA(BlendColorGrading);

public:
	virtual void InspectorImguiDraw() override;
	virtual void Serialized(std::ofstream& ofs) override;
	virtual void Deserialized(std::ifstream& ifs) override;

	void SetTexture(std::wstring path, int index);
	void SetTexture(const Texture& texture, int index);

	void SetWeight(float weight);
	struct BlendColorData
	{
		float weight{ 0.0f };
		float padp[3];
	} value;

private:
	std::wstring texturePath[2];
	
};






struct Bloom : public PostProcessData
{
public:
	Bloom();
	virtual ~Bloom();
	REGISTER_POST_PROCESS_DATA(Bloom);

public:
	virtual void InspectorImguiDraw() override;
	virtual void Serialized(std::ofstream& ofs) override;
	virtual void Deserialized(std::ifstream& ifs) override;


	struct BloomData
	{
		int bloomCurveMethod;
		float curveThreshold;
		float bloomIntensity;
		float pad;
	} value;

private:
	Texture textureMip[3];
	Texture tempTexture;
	Texture blurTexture;

};


