#pragma once
#include <Component/Base/RenderComponent.h>
#include <Asset/MaterialAsset.h>



enum class AlignHorizontal
{
	Left,
	Center,
	Right
};
enum class AlignVertical
{
	Top,
	Center,
	Bottom
};

struct UIElementVS
{
	Matrix World;
	Matrix texcorrdMatrix;
};

struct UIElementPS
{
	Vector4 Color;
};


class UIRenderComponenet : public RenderComponent
{
	friend class EventManager;

public:
	UIRenderComponenet();
	virtual ~UIRenderComponenet() override;

public:
	virtual void Awake() override;
	virtual void Start() override;

protected:
	virtual void FixedUpdate() override;
	virtual void Update() override;
	virtual void LateUpdate() override;
	virtual void Render() override;
	//virtual void InspectorImguiDraw() override;
	//virtual void Serialized(std::ofstream& ofs);
	//virtual void Deserialized(std::ifstream& ifs);

public:
	void SetTexture(std::wstring_view texturePath);
	void SetTexture(Texture texture);
	void SetTransform(size_t positionX,
					  size_t positionY,
					  size_t width,
					  size_t height,
					  float rotation_degree = 0);
	void SetPosition(size_t positionX, size_t positionY);
	void SetSize(size_t width, size_t height);
	void SetRotationDegree(float rotation_degree);


	/*void SetUVTransform(size_t positionX,
						size_t positionY,
						size_t width,
						size_t height);*/

	void SetColor(Vector4 Color) { this->Color = Color; }

private:
	Texture texture;
	UIDrawCommand uic;
	UIRender1StencilMaskingCommand smc;
	UIRender1DrawingOnMaskingCommand domc;

	std::wstring texturePath;
	Vector2 position;
	Vector2 size;
	float rotation{ 0 };
public:
	bool this_is_mask{ false };
	bool draw_after_masking{ false };
	float drawSpeed{ 0 };
	Vector2 GetCenterPosition() { return position; }
	Vector2 GetSize() { return size; }
	virtual void InspectorImguiDraw() override;

private:
	Matrix transformMatrix;
	Matrix texCoordTransformMatrix;
	Vector4 Color{ 1,1,1,1 };

//	AlignHorizontal alignHorizontal{ AlignHorizontal::Center };
//	AlignVertical alignVertical{ AlignVertical::Center };
//
//public:
//	void SetAlign(AlignHorizontal horizontal, AlignVertical vertical)
//	{
//		this->alignHorizontal = horizontal;
//		this->alignVertical = vertical;
//	}
//	void SetAlign(AlignHorizontal horizontal)
//	{
//		this->alignHorizontal = horizontal;
//	}
//	void SetAlign(AlignVertical vertical)
//	{
//		this->alignVertical = vertical;
//	}
};


class UIRenderComponenet2 : public RenderComponent
{
	friend class EventManager;

public:
	UIRenderComponenet2();
	virtual ~UIRenderComponenet2() override;


public:
	virtual void Awake() override;
	virtual void Start() override;

protected:
	virtual void FixedUpdate() override {}
	virtual void Update() override;
	virtual void LateUpdate() override {}
	virtual void Render() override;
	virtual void InspectorImguiDraw() override;
	virtual void Serialized(std::ofstream& ofs);
	virtual void Deserialized(std::ifstream& ifs);

public:
	void SetTransform(float left,
					  float top,
					  float right,
					  float bottom,
					  float rotation_degree = 0);

	/*값 유지하고 업데이트만*/
	void SetTransform();

	/*비율 유지 상대 이동 X축*/     
	void MoveX(float ndcX);
	/*비율 유지 상대 이동 Y축*/
	void MoveY(float ndcY);

	/*비율 유지 절대 이동 X축*/
	void SetPosX(float ndcX);
	/*비율 유지 절대 이동 Y축*/
	void SetPosY(float ndcY);

	/*너비 조정*/
	void SetWidth(float width);
	float GetWidth() const;
	/*높이 조정*/
	void SetHeight(float height);
	float GetHeight() const;
	
	/*회전 조정*/
	void SetRotationZ(float angle_degree);
	Matrix texcorrdMatrix;
	float uvScale;
public:
	void PositionAnimation(Vector2 targetPosition, float duration, bool loopBack = true);
	void ScaleAnimation(float size_mult, float duration, bool loopBack = true);
	void RotationAnimation(float targetAngleZ, float duration, bool loopBack = true);
private:
	struct AnimeData
	{
		bool onAnimation = false;
		bool loopBack = false;
		float AnimeStep = 0;
		float AnimeTime = 0;
		union Key
		{
			Vector2 vec2;
			float rot;
		};
		Key start{};
		Key end{};
	};

	AnimeData posAnime;
	void UpdatePositionAnimation();

	AnimeData scaleAnime;
	void UpdateScaleAnimation();

	AnimeData rotAnime;
	void UpdateRotationAnimation();

	Vector2 center;
	Vector2 size;
public:
	
	bool this_is_mask{ false };
	bool draw_after_masking{ false };
	bool isdontMaskingDraw{ false };
	bool flipY = false;

	float drawSpeed{ 0 };
	Vector2 GetCenterPosition() const;
	Vector2 GetSize() const { return Vector2{ GetWidth(), GetHeight() }; }
	CustomData& GetCustomData() { return materialAsset.customData; }

	float left{};
	float top{};
	float right{};
	float bottom{};
	float rotation_degree{};
	Vector2 pivot{ 0.f, 0.f };
	Vector2 anchorMin{ 0.f, 0.f };
	Vector2 anchorMax{ 1.f, 1.f };
private:
	UIMaterialDrawCommand uiDrawCommand;
	StencilMaskingCommand smc;
	DrawingOnMaskingCommand domc;


	MaterialAsset materialAsset;
};
