#pragma once
#include <vector>
#include <Component/Base/RenderComponent.h>
#include <Asset/MaterialAsset.h>
#include <directxtk/SpriteFont.h>


enum class FontType
{
	Omu = 0,
	HangameRegular = 1,
	HangameSemiBold = 2
};

class TextRender : public RenderComponent
{
	static std::unique_ptr<DirectX::SpriteFont> font1;
	static std::unique_ptr<DirectX::SpriteFont> font2;
	static std::unique_ptr<DirectX::SpriteFont> font3;

	//const float default_size{ 23.0f };
	std::vector<const char*> font_name{ "Omu", "HangameRegular", "HangameSemiBold" };

	Color color{ 0, 0, 0, 1 };
	float scale{ 1.0f };
	int font_type_index{ 0 };
	FontType type{ FontType::Omu };

	TextDrawCommand textDrawCommand;

public:
	float drawSpeed{ 0 };
	virtual void Awake() {}

protected:
	virtual void FixedUpdate() {}
	virtual void Update() {}
	virtual void LateUpdate() {}
	virtual void Render();

private:
	void Initialize();
	Vector2 GetMeasure();

public:
	std::wstring text;
	void SetText(const std::wstring& text);
	void SetColor(Color color);
	void SetScale(float scale);
	void SetType(int font_index);
	void SetType(FontType type);

public:
	virtual void InspectorImguiDraw() override;
	virtual void Serialized(std::ofstream& ofs) override;
	virtual void Deserialized(std::ifstream& ifs) override;



private:
	float anim_size[3];
	Color anim_color[3];
	float play_speed{ 10.0f };
	float anim_t{ 0.0f };
	int anim_step{ 0 };

	bool on_animation{ false };
	const float step_ratio[3]{ 0.4f, 0.6f, 0.4f };
public:
	void Highlight(float size_mult, float duration);
	void Highlight(float size_mult, Color change_color, float duration);
};

