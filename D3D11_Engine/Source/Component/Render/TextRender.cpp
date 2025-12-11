#include "TextRender.h"
#include "D3DCore/D3D11_GameApp.h"
#include "../Renderer/DefferdRenderer.h"
#include <Utility/ImguiHelper.h>
#include "Utility/SerializedUtility.h"
#include "Core/TimeSystem.h"
#include "Math/Mathf.h"
#include "../PotionAtelier/Source/Object/TextObject.h"

using namespace TimeSystem;

std::unique_ptr<DirectX::SpriteFont> TextRender::font1{ nullptr };
std::unique_ptr<DirectX::SpriteFont> TextRender::font2{ nullptr };
std::unique_ptr<DirectX::SpriteFont> TextRender::font3{ nullptr };

void TextRender::Render()
{
	if (font1 == nullptr) Initialize();

	textDrawCommand.text = text;
	textDrawCommand.drawSpeed = drawSpeed;

	if (transform.Parent == nullptr)
	{
		textDrawCommand.x = transform.GetPosition().x;
		textDrawCommand.y = transform.GetPosition().y;
	}
	else if (transform.Parent != nullptr)
	{
		textDrawCommand.x = transform.GetLocalPosition().x + transform.Parent->GetPosition().x;// *transform.Parent->GetScale().x;
		textDrawCommand.y = transform.GetLocalPosition().y + transform.Parent->GetPosition().y;// *transform.Parent->GetScale().y;
	}

	SIZE client_size = D3D11_GameApp::GetClientSize();
	textDrawCommand.x = (textDrawCommand.x + 1.0f) / 2.0f * client_size.cx;
	textDrawCommand.y = (1.0f - (textDrawCommand.y + 1.0f) / 2.0f) * client_size.cy;
	
	if (on_animation == false)
	{
		textDrawCommand.color = color;
		textDrawCommand.size_mult = scale;

		/*anim_t += Time.DeltaTime * play_speed;
		if (anim_t >= 0.25f)
		{
			Highlight(1.4f, Color{ 1,1,1,1 }, 1.0f);
		}*/
	}
	else if (on_animation == true)
	{
		anim_t += Time.DeltaTime * play_speed;
		
		if (anim_t >= 0.0f && anim_t < step_ratio[0])
		{
			textDrawCommand.size_mult = Mathf::Lerp(anim_size[0], anim_size[1], anim_t / step_ratio[0]);
			textDrawCommand.color = Color::Lerp(anim_color[0], anim_color[1], anim_t / step_ratio[0]);
		}
		else if (anim_t >= step_ratio[0] && anim_t < step_ratio[0] + step_ratio[1])
		{
			textDrawCommand.size_mult = anim_size[1];
			textDrawCommand.color = anim_color[1];
		}
		else if (anim_t >= step_ratio[0] + step_ratio[1] && anim_t < step_ratio[0] + step_ratio[1] + step_ratio[2])
		{
			float _dT = anim_t - step_ratio[0] - step_ratio[1];
			textDrawCommand.size_mult = Mathf::Lerp(anim_size[1], anim_size[2], _dT / step_ratio[2]);
			textDrawCommand.color = Color::Lerp(anim_color[1], anim_color[2], _dT / step_ratio[0]);
		}
		else if (anim_t >= step_ratio[0] + step_ratio[1] + step_ratio[2])
		{
			textDrawCommand.size_mult = anim_size[2];
			textDrawCommand.color = anim_color[2];
			on_animation = false;
		}
	}

	Vector2 measured_size = GetMeasure();
	textDrawCommand.x -= measured_size.x * textDrawCommand.size_mult * 0.5f;
	textDrawCommand.y -= measured_size.y * textDrawCommand.size_mult * 0.5f;

	if (type == FontType::Omu)
	{
		textDrawCommand.sprite_font = font1.get();
	}
	else if (type == FontType::HangameRegular)
	{
		textDrawCommand.sprite_font = font2.get();
	}
	else if (type == FontType::HangameSemiBold)
	{
		textDrawCommand.sprite_font = font3.get();
	}
	D3D11_GameApp::GetRenderer().AddDrawCommand(textDrawCommand);
}

void TextRender::Initialize()
{
	font1 = std::move(D3D11_GameApp::GetRenderer().CreateSpriteFont(L"./Resource/Font/omu.spritefont"));
	font2 = std::move(D3D11_GameApp::GetRenderer().CreateSpriteFont(L"./Resource/Font/HanGamePoker.spritefont"));
	font3 = std::move(D3D11_GameApp::GetRenderer().CreateSpriteFont(L"./Resource/Font/HanGamePokerSemiBold.spritefont"));
}

Vector2 TextRender::GetMeasure()
{
	XMVECTOR vector;
	switch (type)
	{
	case FontType::Omu:
		vector = font1->MeasureString(text.c_str());
		break;
	case FontType::HangameRegular:
		vector = font2->MeasureString(text.c_str());
		break;
	case FontType::HangameSemiBold:
		vector = font3->MeasureString(text.c_str());
		break;
	}

	return Vector2{ XMVectorGetX(vector), XMVectorGetY(vector) };
}

void TextRender::SetText(const std::wstring& text)
{
	this->text = text;
}

void TextRender::SetColor(Color color)
{
	this->color = color;
}

void TextRender::SetScale(float scale)
{
	this->scale = scale;
}

void TextRender::SetType(int font_index)
{
	font_type_index = font_index;
	type = static_cast<FontType>(font_index);
}

void TextRender::SetType(FontType type)
{
	font_type_index = static_cast<int>(type);
	this->type = type;
}





void TextRender::InspectorImguiDraw()
{
	ImGui::PushID(GetComponentIndex());
	if (ImGui::TreeNode("TextRenderer"))
	{
		SIZE client_size = D3D11_GameApp::GetClientSize();


		char buffer[1024];
		strcpy_s(buffer, utfConvert::wstring_to_utf8(text).c_str());
		buffer[sizeof(buffer) - 1] = '\0';

		if (ImGui::InputTextMultiline("Text", buffer, sizeof(buffer)))
		{			
			text = utfConvert::utf8_to_wstring(buffer);
		}

		ImGui::ColorEdit3("Color", reinterpret_cast<Vector4*>(&color));
		ImGui::SliderFloat("Scale", &scale, 0.0f, 10.0f, "%.1f");
		if (ImGui::Combo("Select Font", &font_type_index, font_name.data(), font_name.size()))
		{
			SetType(font_type_index);
		}
		if (ImGui::Button("Highlight"))
		{
			Highlight(1.4f, Color{ 1,1,1,1 }, 1.0f);
		}
		ImGui::SliderFloat("Draw Speed", &drawSpeed, -100.f, 100.f);

		ImGui::TreePop();
	}
	ImGui::PopID();
}

void TextRender::Serialized(std::ofstream& ofs)
{
	if (typeid(TextObject) != typeid(gameObject))
		return;

	constexpr size_t header = (std::numeric_limits<size_t>::max)();
	constexpr uint32_t version = 1;
	Binary::Write::data(ofs, header); //헤더
	Binary::Write::data(ofs, version); //버전

	Binary::Write::wstring(ofs, text);
	Binary::Write::Color(ofs, color);
	Binary::Write::data<float>(ofs, scale);
	Binary::Write::data<int>(ofs, font_type_index);
	Binary::Write::data<int>(ofs, static_cast<int>(type));

	if constexpr (version > 0)
	{
		Binary::Write::data<float>(ofs, drawSpeed);

	}
}

void TextRender::Deserialized(std::ifstream& ifs)
{
	if (typeid(TextObject) != typeid(gameObject))
		return;

	size_t header = Binary::Read::data<size_t>(ifs);
	uint32_t version = 0;
	if (header != (std::numeric_limits<size_t>::max)())
	{
		text.resize(header);
		ifs.read(reinterpret_cast<char*>(text.data()), header * sizeof(wchar_t));
	}
	else
	{
		version = Binary::Read::data<uint32_t>(ifs);
		text = Binary::Read::wstring(ifs);
	}

	color = Binary::Read::Color(ifs);
	scale = Binary::Read::data<float>(ifs);
	font_type_index = Binary::Read::data<int>(ifs);
	type = static_cast<FontType>(Binary::Read::data<int>(ifs));

	if (version > 0)
	{
		drawSpeed = Binary::Read::data<float>(ifs);
	}
}

void TextRender::Highlight(float size_mult, float duration)
{
	Highlight(size_mult, this->color, duration);
}

void TextRender::Highlight(float size_mult, Color change_color, float duration)
{
	if (on_animation == true) return;
	
	float total_t = step_ratio[0] + step_ratio[1] + step_ratio[2];
	play_speed = 1.f / duration * total_t;

	anim_size[0] = this->scale;
	anim_size[1] = this->scale * size_mult;
	anim_size[2] = this->scale;

	anim_color[0] = this->color;
	anim_color[1] = change_color;
	anim_color[2] = this->color;

	anim_t = 0.0f;
	anim_step = 0;

	on_animation = true;
}
