#include "UIRenderComponenet.h"
#include "Manager\TextureManager.h"
#include <Utility/ImguiHelper.h>
#include <Utility\WinUtility.h>
#include <NodeEditor/NodeEditor.h>
#include <Utility/SerializedUtility.h>
#include <Math/Mathf.h>
#include <Core/TimeSystem.h>
#include <Component/Camera/Camera.h>
#include <Math/Mathf.h>

UIRenderComponenet::UIRenderComponenet()
{
	EventManager::AddUIRenderer(this);
}

UIRenderComponenet::~UIRenderComponenet()
{
	EventManager::RemoveUIRenderer(this);

	if (texturePath.size())
	{
		textureManager.ReleaseSharingTexture(texturePath.data());
	}
}

void UIRenderComponenet::Awake()
{
	transformMatrix = UIMatrixHelper::MakeUIMatrix(0,
												   0,
												   300,
												   200,
												   1920,
												   1080);
	texCoordTransformMatrix = Matrix::Identity;
	Color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
}

void UIRenderComponenet::Start()
{

}

void UIRenderComponenet::FixedUpdate()
{
}

void UIRenderComponenet::Update()
{
}

void UIRenderComponenet::LateUpdate()
{
}

void UIRenderComponenet::Render()
{
	UIElementVS vs;
	UIElementPS ps;

	// 쿼드메시가 -1 ~ 1을 써서 두 배됨
	transformMatrix = UIMatrixHelper::MakeUIMatrix(position.x,
												   position.y,
												   size.x * 0.5f,
												   size.y * 0.5f,
												   D3D11_GameApp::GetClientSize().cx,
												   D3D11_GameApp::GetClientSize().cy,
												   rotation);

	vs.World = XMMatrixTranspose(transformMatrix);
	vs.texcorrdMatrix = XMMatrixTranspose(texCoordTransformMatrix);	
	ps.Color = Color;
	



	if (this_is_mask)
	{
		smc.transformBuffer.Set(vs);
		smc.colorBuffer.Set(ps);
		smc.drawSpeed = drawSpeed;
		D3D11_GameApp::GetRenderer().AddDrawCommand(smc);
	}
	else if (draw_after_masking)
	{
		domc.transformBuffer.Set(vs);
		domc.colorBuffer.Set(ps);
		domc.drawSpeed = drawSpeed;
		D3D11_GameApp::GetRenderer().AddDrawCommand(domc);
	}
	else
	{
		uic.transformBuffer.Set(vs);
		uic.colorBuffer.Set(ps);
		uic.drawSpeed = drawSpeed;
		D3D11_GameApp::GetRenderer().AddDrawCommand(uic);
	}
}

void UIRenderComponenet::SetTransform(size_t positionX, size_t positionY, size_t width, size_t height, float rotation_degree)
{
	//switch (alignVertical)
	//{
	//case AlignVertical::Top:
	//{
	//	positionY += height * 0.5f;
	//	break;
	//}
	//case AlignVertical::Center:
	//{
	//	break;
	//}
	//case AlignVertical::Bottom:
	//{
	//	positionY -= height * 0.5f;
	//	break;
	//}
	//}

	//switch (alignHorizontal)
	//{
	//case AlignHorizontal::Left:
	//{
	//	positionX += width * 0.5f;
	//	break;
	//}
	//case AlignHorizontal::Center:
	//{
	//	break;
	//}
	//case AlignHorizontal::Right:
	//{
	//	positionX -= width * 0.5f;
	//	break;
	//}
	//}

	position.x = positionX;
	position.y = positionY;
	size.x = width;
	size.y = height;
	rotation = rotation_degree;
}

void UIRenderComponenet::SetPosition(size_t positionX, size_t positionY)
{
	position.x = positionX;
	position.y = positionY;
}
void UIRenderComponenet::SetSize(size_t width, size_t height)
{
	size.x = width;
	size.y = height;
}
void UIRenderComponenet::SetRotationDegree(float rotation_degree)
{
	rotation = rotation_degree;
}

void UIRenderComponenet::InspectorImguiDraw()
{
	ImGui::PushID(GetComponentIndex());
	if (ImGui::TreeNode("UIRenderComponenet1"))
	{
		ImGui::Text("This component is intended for temporary UI images\nthat adding in code and is not serialized.");

		ImGui::Checkbox("IsMask", &this_is_mask);
		ImGui::Checkbox("DrawOnMask", &draw_after_masking);
		ImGui::SliderFloat("DrawSpeed", &drawSpeed, 0.0f, 10.0f);

		ImGui::TreePop();
	}
	ImGui::PopID();
}

//void UIRenderComponenet::SetUVTransform(size_t positionX, size_t positionY, size_t width, size_t height)
//{
//	texCoordTransformMatrix = UIMatrixHelper::MakeUIMatrix(positionX,
//														   positionY,
//														   width,
//														   height,
//														   1,
//														   1);
//}

//void UIRenderComponenet::InspectorImguiDraw()
//{
//	if (ImGui::TreeNode("UIRenderComponenet"))
//	{
//		bool isCahnged = false;
//
//		ImGui::PushItemWidth(100);
//		isCahnged |= ImGui::DragFloat("left", (float*)&left, 1.0f);
//		ImGui::SameLine();
//		isCahnged |= ImGui::DragFloat("top", (float*)&top, 1.0f);
//		isCahnged |= ImGui::DragFloat("right", (float*)&right, 1.0f);
//		ImGui::SameLine();
//		isCahnged |= ImGui::DragFloat("bottom", (float*)&bottom, 1.0f);
//		ImGui::PopItemWidth();
//		isCahnged |= ImGui::DragFloat("Rotation", &rotation_degree, 1.0f);
//		isCahnged |= ImGui::DragFloat2("Pivot", &pivot.x, 0.05f);
//		isCahnged |= ImGui::DragFloat2("AnchorMin", &anchorMin.x, 0.05f);
//		isCahnged |= ImGui::DragFloat2("AnchorMax", &anchorMax.x, 0.05f);
//
//
//		if (isCahnged)
//		{
//			SetTransform(left, top, right, bottom, rotation_degree);
//		}
//
//		if (ImGui::Button("Texture Path Load"))
//		{
//			std::wstring texturePath = WinUtility::GetOpenFilePath();
//			if (texturePath.size())
//			{
//				SetTexture(texturePath);
//			}
//		}
//		ImGui::ColorEdit4("Color", &Color.x);
//
//		ImGui::TreePop();
//	}
//}

//void UIRenderComponenet::Serialized(std::ofstream& ofs)
//{
//	Binary::Write::wstring(ofs, texturePath);
//	Binary::Write::data(ofs, left);
//	Binary::Write::data(ofs, top);
//	Binary::Write::data(ofs, right);
//	Binary::Write::data(ofs, bottom);
//	Binary::Write::data(ofs, rotation_degree);
//	Binary::Write::Vector2(ofs, pivot);
//	Binary::Write::Vector2(ofs, anchorMin);
//	Binary::Write::Vector2(ofs, anchorMax);
//
//	Binary::Write::Vector4(ofs, Color);
//}
//
//void UIRenderComponenet::Deserialized(std::ifstream& ifs)
//{
//	texturePath = Binary::Read::wstring(ifs);
//	SetTexture(texturePath);
//	left = Binary::Read::data<float>(ifs);
//	top = Binary::Read::data<float>(ifs);
//	right = Binary::Read::data<float>(ifs);
//	bottom = Binary::Read::data<float>(ifs);
//	rotation_degree = Binary::Read::data<float>(ifs);
//	pivot = Binary::Read::Vector2(ifs);
//	anchorMin = Binary::Read::Vector2(ifs);
//	anchorMax = Binary::Read::Vector2(ifs);
//	Color = Binary::Read::Vector4(ifs);
//	SetTransform(left, top, right, bottom, rotation_degree);
//}

void UIRenderComponenet::SetTexture(std::wstring_view texturePath)
{
	if (!std::filesystem::exists(texturePath))
	{
		return;
	}

	this->texturePath = texturePath;
	ComPtr<ID3D11ShaderResourceView> textureSRV;

	textureManager.CreateSharingTexture(texturePath.data(), &textureSRV);
	texture.LoadTexture(textureSRV.Get());
	textureSRV->AddRef();

	uic.texture = texture;
	smc.texture = texture;
	domc.texture = texture;
}

void UIRenderComponenet::SetTexture(Texture texture)
{
	this->texture = texture;
	uic.texture = texture;
	smc.texture = texture;
	domc.texture = texture;
}










void UIRenderComponenet2::SetTransform(float left, float top, float right, float bottom, float rotation_degree)
{
	this->left = left;
	this->top = top;
	this->right = right;
	this->bottom = bottom;
	this->rotation_degree = rotation_degree;

	// 현재 UI 컨테이너(부모)의 크기를 가져옴
	SIZE clientSize = D3D11_GameApp::GetClientSize();

	Vector2 lerpMin
	{
		left / clientSize.cx,
		top / clientSize.cy,
	};
	Vector2 lerpMax
	{
		(clientSize.cx - right)  / clientSize.cx,
		(clientSize.cy - bottom) / clientSize.cy
	};
	Vector2 realAnchorMin
	{
		Mathf::Lerp(lerpMin.x, lerpMax.x, anchorMin.x),
		Mathf::Lerp(lerpMin.y, lerpMax.y, anchorMin.y)
	};
	Vector2 realAnchorMax
	{
		Mathf::Lerp(lerpMin.x, lerpMax.x, anchorMax.x),
		Mathf::Lerp(lerpMin.y, lerpMax.y, anchorMax.y)
	};

	left = (realAnchorMin.x * clientSize.cx);
	top = (realAnchorMin.y * clientSize.cy);
	right = (realAnchorMax.x * clientSize.cx);
	bottom = (realAnchorMax.y * clientSize.cy);

	size.x = (right - left) * 0.5f;
	size.x = (std::max)(0.f, size.x);
	size.y = (bottom - top) * 0.5f;
	size.y = (std::max)(0.f, size.y);
	float posX = left + size.x * 2.0f * (0.5f - pivot.x);
	float posY = top + size.y * 2.0f * (0.5f - pivot.y);


	Matrix uiMatrix = flipY ? UIMatrixHelper::MakeUIMatrixYFlip(
		posX,
		posY,
		size.x,
		size.y,
		clientSize.cx,
		clientSize.cy,
		rotation_degree) :
		UIMatrixHelper::MakeUIMatrix(
			posX,
			posY,
			size.x,
			size.y,
			clientSize.cx,
			clientSize.cy,
			rotation_degree);

	transform.SetWorldMatrix(uiMatrix);
	transform.SetLocalMatrix(uiMatrix * Matrix::CreateScale(1, -1, 1));
}

void UIRenderComponenet2::SetTransform()
{
	SetTransform(left, top, right, bottom, rotation_degree);
}

void UIRenderComponenet2::MoveX(float ndcX)
{
	SetPosX(center.x + ndcX);
}

void UIRenderComponenet2::MoveY(float ndcY)
{
	SetPosY(center.y + ndcY);
}

void UIRenderComponenet2::SetPosX(float ndcX)
{
	const SIZE& clientSize = D3D11_GameApp::GetClientSize();
	float x = ndcX;
	x = std::clamp(x, -1.f, 1.f);
	float clientWidth = (float)clientSize.cx;

	float currScreen = (x + 1.0f) * 0.5f * clientWidth;
	float halfWidth = GetWidth() * 0.5f;
	left = currScreen - halfWidth;
	right = clientWidth - (currScreen + halfWidth);
	center.x = ndcX;
	SetTransform();
}

void UIRenderComponenet2::SetPosY(float ndcY)
{
	const SIZE& clientSize = D3D11_GameApp::GetClientSize();
	float y = ndcY;
	y = std::clamp(y, -1.f, 1.f);
	float clientHeight = (float)clientSize.cy;

	float currScreen = (1.0f - y) * 0.5f * clientHeight;
	float halfWidth = GetHeight() * 0.5f;
	top = currScreen - halfWidth;
	bottom = clientHeight - (currScreen + halfWidth);
	center.y = ndcY;
	SetTransform();
}

void UIRenderComponenet2::SetWidth(float width)
{
	const SIZE& clientSize = D3D11_GameApp::GetClientSize();
	float clientWidth = (float)clientSize.cx;
	float ndcX = center.x;
	float screenX = (ndcX + 1.0f) * 0.5f * clientWidth;
	left = screenX - width * 0.5;
	right = clientWidth - screenX - width * 0.5;
	SetTransform();
}

float UIRenderComponenet2::GetWidth() const
{
	const SIZE& clientSize = D3D11_GameApp::GetClientSize();
	return std::abs((clientSize.cx - right) - left);
}

void UIRenderComponenet2::SetHeight(float height)
{
	const SIZE& clientSize = D3D11_GameApp::GetClientSize();
	float clientHeight = (float)clientSize.cy;
	float ndcY = center.y;
	float screenY = (1.0f - ndcY) * 0.5f * clientHeight;
	top = screenY - height * 0.5f;
	bottom = clientHeight - screenY - height * 0.5f;
	SetTransform();
}

float UIRenderComponenet2::GetHeight() const
{
	const SIZE& clientSize = D3D11_GameApp::GetClientSize();
	return std::abs((clientSize.cy - bottom) - top);
}

void UIRenderComponenet2::SetRotationZ(float angle_degree)
{
	rotation_degree = angle_degree;
	SetTransform();
}

void UIRenderComponenet2::PositionAnimation(Vector2 targetPosition, float duration, bool loopBack)
{
	if (posAnime.onAnimation == true)
		return;

	posAnime.start.vec2 = center;
	posAnime.end.vec2 = posAnime.start.vec2 + targetPosition;

	posAnime.AnimeStep = 0;
	posAnime.AnimeTime = duration;

	posAnime.onAnimation = true;
	posAnime.loopBack = loopBack;
}

void UIRenderComponenet2::UpdatePositionAnimation()
{
	using namespace TimeSystem;

	if (posAnime.onAnimation == false)
		return;

	posAnime.AnimeStep += Time.DeltaTime;
	Vector2 animePos;
	float t = posAnime.AnimeStep / posAnime.AnimeTime;
	t = std::clamp(t, 0.f, 1.f);
	if (posAnime.loopBack)
	{
		if (t < 0.5f)
		{
			animePos = Vector2::Lerp(posAnime.start.vec2, posAnime.end.vec2, t * 2.f);
		}
		else
		{
			animePos = Vector2::Lerp(posAnime.end.vec2, posAnime.start.vec2, (t - 0.5f) * 2.f);
		}
	}
	else
	{
		animePos = Vector2::Lerp(posAnime.start.vec2, posAnime.end.vec2, t);
	}
	
	SetPosX(animePos.x);
	SetPosY(animePos.y);

	if (posAnime.AnimeStep >= posAnime.AnimeTime)
	{
		posAnime.onAnimation = false;
	}
}

void UIRenderComponenet2::ScaleAnimation(float size_mult, float duration, bool loopBack)
{
	if (scaleAnime.onAnimation == true)
		return;


	scaleAnime.start.vec2 = Vector2(GetWidth());
	scaleAnime.end.vec2 = scaleAnime.start.vec2 * size_mult;

	scaleAnime.AnimeStep = 0;
	scaleAnime.AnimeTime = duration;

	scaleAnime.onAnimation = true;
	scaleAnime.loopBack = loopBack;
}

void UIRenderComponenet2::UpdateScaleAnimation()
{
	using namespace TimeSystem;

	if (scaleAnime.onAnimation == false)
		return;

	scaleAnime.AnimeStep += Time.DeltaTime;
	Vector2 animeSize;
	float t = scaleAnime.AnimeStep / scaleAnime.AnimeTime;
	t = std::clamp(t, 0.f, 1.f);
	if (scaleAnime.loopBack)
	{
		if (t < 0.5f)
		{
			animeSize = Vector2::Lerp(scaleAnime.start.vec2, scaleAnime.end.vec2, t * 2.f);
		}
		else
		{
			animeSize = Vector2::Lerp(scaleAnime.end.vec2, scaleAnime.start.vec2, (t - 0.5f) * 2.f);
		}
	}
	else
	{
		animeSize = Vector2::Lerp(scaleAnime.start.vec2, scaleAnime.end.vec2, t);
	}
	
	SetWidth(animeSize.x);
	SetHeight(animeSize.y);
	if (scaleAnime.AnimeStep >= scaleAnime.AnimeTime)
	{
		scaleAnime.onAnimation = false;
	}
}

void UIRenderComponenet2::RotationAnimation(float targetAngleZ, float duration, bool loopBack)
{
	if (rotAnime.onAnimation == true)
		return;

	rotAnime.start.rot = rotation_degree;
	rotAnime.end.rot = rotAnime.start.rot + targetAngleZ;

	rotAnime.AnimeStep = 0;
	rotAnime.AnimeTime = duration;

	rotAnime.onAnimation = true;
	rotAnime.loopBack = loopBack;
}

void UIRenderComponenet2::UpdateRotationAnimation()
{
	using namespace TimeSystem;

	if (rotAnime.onAnimation == false)
		return;

	rotAnime.AnimeStep += Time.DeltaTime;
	float t = rotAnime.AnimeStep / rotAnime.AnimeTime;
	float animeRot;
	t = std::clamp(t, 0.f, 1.f);
	if (rotAnime.loopBack)
	{
		if (t < 0.5f)
		{
			animeRot = Mathf::Lerp(rotAnime.start.rot, rotAnime.end.rot, t * 2.f);
		}
		else
		{
			animeRot = Mathf::Lerp(rotAnime.end.rot, rotAnime.start.rot, (t - 0.5f) * 2.f);
		}
	}
	else
	{
		animeRot = Mathf::Lerp(rotAnime.start.rot, rotAnime.end.rot, t);
	}
	SetRotationZ(animeRot);

	if (rotAnime.AnimeStep >= rotAnime.AnimeTime)
	{
		rotAnime.onAnimation = false;
	}
}

Vector2 UIRenderComponenet2::GetCenterPosition() const
{
	Vector2 screen = Camera::NdcToScreenPoint(center);
	return screen;
}

UIRenderComponenet2::UIRenderComponenet2()
{
	EventManager::AddUIRenderer(this);
}
UIRenderComponenet2::~UIRenderComponenet2()
{
	EventManager::RemoveUIRenderer(this);
	uiDrawCommand.~UIMaterialDrawCommand();
}

void UIRenderComponenet2::Awake()
{
	materialAsset.OpenAsset(L"UIRenderComponenet2Temp/UIRenderComponenet2.materialAsset");
	transform.UseManualMatrix = true;
}

void UIRenderComponenet2::Start()
{

}

void UIRenderComponenet2::Update()
{	
	UpdatePositionAnimation();
	UpdateScaleAnimation();
	UpdateRotationAnimation();
}

void UIRenderComponenet2::Render()
{
	const SIZE& clientSize = D3D11_GameApp::GetClientSize();
	const Matrix& uiMatrix = transform.GetWM();

	UIElementVS vs;
	smc.shaderResources.clear();
	uiDrawCommand.shaderResources.clear();
	domc.shaderResources.clear();
	if (materialAsset.GetCustomBuffer())
	{
		materialAsset.GetCustomBuffer().Update(materialAsset.customData.Data());
		uiDrawCommand.shaderResources.push_back(
			Binadble
			{
				.shaderType = EShaderType::Pixel,
				.bindableType = EShaderBindable::ConstantBuffer,
				.slot = 5,
				.bind = (ID3D11Buffer*)materialAsset.GetCustomBuffer()
			}
		);
		smc.shaderResources.push_back(
			Binadble
			{
				.shaderType = EShaderType::Pixel,
				.bindableType = EShaderBindable::ConstantBuffer,
				.slot = 5,
				.bind = (ID3D11Buffer*)materialAsset.GetCustomBuffer()
			}
		);
		domc.shaderResources.push_back(
			Binadble
			{
				.shaderType = EShaderType::Pixel,
				.bindableType = EShaderBindable::ConstantBuffer,
				.slot = 5,
				.bind = (ID3D11Buffer*)materialAsset.GetCustomBuffer()
			}
		);
	}
	if (this_is_mask)
	{
		vs.World = XMMatrixTranspose(uiMatrix);
		vs.texcorrdMatrix = XMMatrixTranspose(texcorrdMatrix);
		smc.transformBuffer.Set(vs);
		smc.pixelShader = materialAsset.GetPS();
		smc.drawSpeed = drawSpeed;

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
			smc.shaderResources.push_back(bind);
		}

		D3D11_GameApp::GetRenderer().AddDrawCommand(smc);
	}
	else if (draw_after_masking)
	{
		vs.World = XMMatrixTranspose(uiMatrix);
		vs.texcorrdMatrix = texcorrdMatrix;
		domc.transformBuffer.Set(vs);
		domc.pixelShader = materialAsset.GetPS();

		domc.drawSpeed = drawSpeed;
		domc.isdontMaskingDraw = isdontMaskingDraw;
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
			domc.shaderResources.push_back(bind);
		}

		D3D11_GameApp::GetRenderer().AddDrawCommand(domc);
	}
	else
	{
		vs.World = XMMatrixTranspose(uiMatrix);
		vs.texcorrdMatrix = texcorrdMatrix;
		uiDrawCommand.transformBuffer.Set(vs);
		uiDrawCommand.pixelShader = materialAsset.GetPS();

		uiDrawCommand.drawSpeed = drawSpeed;

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
			uiDrawCommand.shaderResources.push_back(bind);
		}

		D3D11_GameApp::GetRenderer().AddDrawCommand(uiDrawCommand);
	}
}

void UIRenderComponenet2::InspectorImguiDraw()
{
	if (Scene* scene = sceneManager.GetActiveScene())
	{
		ImGui::PushID(GetComponentIndex());
		ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.2f, 0.3f, 0.4f, 1.0f)); // 배경색
		ImGui::BeginChild("MeshRenderChild", ImVec2(0, 0), ImGuiChildFlags_AlwaysAutoResize | ImGuiChildFlags_AutoResizeY | ImGuiChildFlags_AutoResizeX);
		if (ImGui::TreeNode("UIRenderComponenet2"))
		{
			if (ImGui::Button("Edit Material"))
			{
				auto editorPath = L"Resource/Materials/Temp/DefaultUI.MaterialAsset";
				ShaderNodeEditor* editor = scene->MakeShaderNodeEditor(editorPath);
				editor->EndPopupEvent = [scene, editorPath]()
					{
						scene->EraseShaderNodeEditor(editorPath);
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

			bool isCahnged = false;
			ImGui::PushItemWidth(200);
			{
				//Move
				{
					float x = center.x;
					if (ImGui::DragFloat("Set Pos X", (float*)&x, 0.01f))
					{
						x = std::clamp(x, -1.f, 1.f);
						SetPosX(x);
					}
				}
				ImGui::SameLine();
				{
					float width = GetWidth();
					if (ImGui::DragFloat("Width", &width))
					{
						width = std::abs(width);
						SetWidth(width);
					}
				}
				{
					float y = center.y;
					if (ImGui::DragFloat("Set Pos Y", (float*)&y, 0.01f))
					{
						y = std::clamp(y, -1.f, 1.f);
						SetPosY(y);
					}
				}
				ImGui::SameLine();
				{
					float height = GetHeight();
					if (ImGui::DragFloat("Height", &height))
					{
						height = std::abs(height);
						SetHeight(height);
					}
				}
			}
			//isCahnged |= ImGui::DragFloat("left", (float*)&left, 1.0f);
			//ImGui::SameLine();
			//isCahnged |= ImGui::DragFloat("top", (float*)&top, 1.0f);
			//isCahnged |= ImGui::DragFloat("right", (float*)&right, 1.0f);
			//ImGui::SameLine();
			//isCahnged |= ImGui::DragFloat("bottom", (float*)&bottom, 1.0f);
			ImGui::PopItemWidth();

			isCahnged |= ImGui::DragFloat("Rotation", &rotation_degree);

			//isCahnged |= ImGui::DragFloat2("Pivot", &pivot.x, 0.05f, Mathf::Epsilon, 1.f);
			isCahnged |= ImGui::DragFloat2("AnchorMin", &anchorMin.x, 0.05f, Mathf::Epsilon, 1.f);
			isCahnged |= ImGui::DragFloat2("AnchorMax", &anchorMax.x, 0.05f, Mathf::Epsilon, 1.f);
			isCahnged |= ImGui::Checkbox("Flip Y", &flipY);

			if (isCahnged)
			{
				SetTransform();
			}

			ImGui::DragFloat("DrawSpeed", &drawSpeed, 1.f);


			ImGui::Checkbox("IsMask", &this_is_mask);
			ImGui::Checkbox("IsDrawOnMask", &draw_after_masking);
			ImGui::Checkbox("IsdontMaskingDraw", &isdontMaskingDraw);


			ImGui::TreePop();
		}
		ImGui::EndChild(); // Child 끝내기
		ImGui::PopStyleColor(); // 스타일 복구
		ImGui::PopID();
	}

}

void UIRenderComponenet2::Serialized(std::ofstream& ofs)
{
	constexpr size_t header = (std::numeric_limits<size_t>::max)();
	constexpr uint32_t version = 5;
	Binary::Write::data(ofs, header); //헤더
	Binary::Write::data(ofs, version); //버전

	Binary::Write::wstring(ofs, materialAsset.GetAssetPath());
	materialAsset.SaveAsset();

	Binary::Write::data(ofs, left);
	Binary::Write::data(ofs, top);
	Binary::Write::data(ofs, right);
	Binary::Write::data(ofs, bottom);
	Binary::Write::data(ofs, rotation_degree);
	Binary::Write::Vector2(ofs, pivot);
	Binary::Write::Vector2(ofs, anchorMin);
	Binary::Write::Vector2(ofs, anchorMax);

	if constexpr (version > 0)
	{
		Binary::Write::data(ofs, drawSpeed);
	}
	if constexpr (version > 1)
	{
		Binary::Write::data<bool>(ofs, this_is_mask);
		Binary::Write::data<bool>(ofs, draw_after_masking);
	}
	if constexpr (version > 2)
	{
		Binary::Write::data(ofs, center.x);
		Binary::Write::data(ofs, center.y);
		Binary::Write::data(ofs, size.x);
		Binary::Write::data(ofs, size.y);
	}
	else
	{
		//가독성 위해 비워둠
		//
	}
	if constexpr (version > 3)
	{
		Binary::Write::data(ofs, flipY);
	}
	if constexpr (version > 4)
	{
		Binary::Write::data(ofs, isdontMaskingDraw);
	}
}

void UIRenderComponenet2::Deserialized(std::ifstream& ifs)
{
	size_t header = Binary::Read::data<size_t>(ifs);
	uint32_t version = 0;
	std::wstring materialPath;
	if (header != (std::numeric_limits<size_t>::max)())
	{
		materialPath.resize(header);
		ifs.read(reinterpret_cast<char*>(materialPath.data()), header * sizeof(wchar_t));
	}
	else
	{
		version = Binary::Read::data<uint32_t>(ifs);
		materialPath = Binary::Read::wstring(ifs);
	}
	materialAsset.OpenAsset(materialPath.c_str());

	left = Binary::Read::data<float>(ifs);
	top = Binary::Read::data<float>(ifs);
	right = Binary::Read::data<float>(ifs);
	bottom = Binary::Read::data<float>(ifs);
	rotation_degree = Binary::Read::data<float>(ifs);
	pivot = Binary::Read::Vector2(ifs);
	anchorMin = Binary::Read::Vector2(ifs);
	anchorMax = Binary::Read::Vector2(ifs);

	if (version > 0)
	{
		drawSpeed = Binary::Read::data<float>(ifs);
	}
	if (version > 1)
	{
		this_is_mask = Binary::Read::data<bool>(ifs);
		draw_after_masking = Binary::Read::data<bool>(ifs);
	}
	if (version > 2)
	{
		center.x = Binary::Read::data<float>(ifs);
		center.y = Binary::Read::data<float>(ifs);
		size.x = Binary::Read::data<float>(ifs);
		size.y = Binary::Read::data<float>(ifs);
	}
	else
	{
		size.x = GetWidth();
		size.y = GetHeight();
	}
	if (version > 3)
	{
		flipY = Binary::Read::data<bool>(ifs);
	}
	if (version > 4)
	{
		isdontMaskingDraw = Binary::Read::data<bool>(ifs);
	}
	SetTransform();
}

