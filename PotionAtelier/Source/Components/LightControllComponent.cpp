#include "LightControllComponent.h"
#include "framework.h"	
#include "Object\GameManager.h"	
LightControllComponent::LightControllComponent()
{
}

void LightControllComponent::Serialized(std::ofstream& ofs)
{
	Binary::Write::data(ofs, 1);
	Binary::Write::data(ofs, lightType);
	Binary::Write::data(ofs, lightIndex);
	Binary::Write::data(ofs, LightKeyFrameData.size());
	for (auto& item : LightKeyFrameData)
	{
		Binary::Write::data(ofs, item.time);
		Binary::Write::data(ofs, item.blendType);
		if (lightType == 0)
		{
			Binary::Write::Vector4(ofs, item.directLight.Color);
			Binary::Write::Vector4(ofs, item.directLight.Directoin);
		}
		else if (lightType == 1)
		{
			Binary::Write::Vector4(ofs, item.pointLight.Color);
			Binary::Write::Vector4(ofs, item.pointLight.position);
		}
	}
}

void LightControllComponent::Deserialized(std::ifstream& ifs)
{
	int version = Binary::Read::data<int>(ifs);
	lightType = Binary::Read::data<int>(ifs);
	lightIndex = Binary::Read::data<int>(ifs);
	size_t size = Binary::Read::data<size_t>(ifs);
	LightKeyFrameData.resize(size);
	for (auto& item : LightKeyFrameData)
	{
		item.time = Binary::Read::data<float>(ifs);
		item.blendType = Binary::Read::data<int>(ifs);
		if (lightType == 0)
		{
			item.directLight.Color = Binary::Read::Vector4(ifs);
			item.directLight.Directoin = Binary::Read::Vector4(ifs);
		}
		else if (lightType == 1)
		{
			item.pointLight.Color = Binary::Read::Vector4(ifs);
			item.pointLight.position = Binary::Read::Vector4(ifs);
		}
	}
}

void LightControllComponent::InspectorImguiDraw()
{
	if (ImGui::TreeNode("LightControllComponent"))
	{
		ImGui::DragFloat("CurrentTime", &currentTime, 0.1f, 0.0f, 1.0f);
		ImGui::Combo("LightType", &lightType, "Directional\0Point\0");
		ImGui::InputInt("LightIndex", &lightIndex);

		if (ImGui::Button("Add KeyFrame"))
		{
			LightKeyFrameData.emplace_back();
		}

		std::ranges::sort(LightKeyFrameData, [](const LightData& a, const LightData& b) { return a.time < b.time; });
		for (size_t i = 0; i < LightKeyFrameData.size(); i++)
		{
			ImGui::BeginChild(std::format("{}", i).c_str(),
							  ImVec2(0, 0),
							  ImGuiChildFlags_Borders | ImGuiChildFlags_AutoResizeX | ImGuiChildFlags_AutoResizeY | ImGuiChildFlags_AlwaysAutoResize, ImGuiWindowFlags_NoMove);
			if (ImGui::BeginPopupContextWindow())
			{
				bool isDelete = false;
				if (ImGui::Button("Delete"))
				{
					LightKeyFrameData.erase(LightKeyFrameData.begin() + i);
					ImGui::CloseCurrentPopup();
					i--;
					isDelete = true;
				}
				ImGui::EndPopup();
				if (isDelete)
				{
					ImGui::EndChild();
					continue;
				}
			}

			ImGui::DragFloat("Time", &LightKeyFrameData[i].time);

			ImGui::Combo("BlendType", &LightKeyFrameData[i].blendType, "Linear\SmoothStep\0");
			if (lightType == 0)
			{
				Vector3 dir;
				float intensity = LightKeyFrameData[i].directLight.Color.w;
				dir = Vector3(LightKeyFrameData[i].directLight.Directoin);

				ImGui::ColorEdit4("Color", &LightKeyFrameData[i].directLight.Color);
				ImGui::DragVector3("Direction", &dir, 0.01f, -1.f, 1.f);
				ImGui::DragFloat("Intensity", &intensity, 1.f, 0.0000001f, 100.f);

				LightKeyFrameData[i].directLight.Color.w = intensity;
				LightKeyFrameData[i].directLight.Directoin = Vector4(dir.x, dir.y, dir.z, 0);
			}
			else if (lightType == 1)
			{
				Vector3 dir;
				float intensity = LightKeyFrameData[i].directLight.Color.w;
				dir = Vector3(LightKeyFrameData[i].pointLight.position);

				ImGui::ColorEdit4("Color", &LightKeyFrameData[i].pointLight.Color);
				ImGui::DragVector3("Position", &dir, 0.01f, -1.f, 1.f);
				ImGui::DragFloat("Intensity", &intensity, 1.f, 0.0000001f, 100.f);

				LightKeyFrameData[i].pointLight.Color.w = intensity;
				LightKeyFrameData[i].pointLight.position = Vector4(dir.x, dir.y, dir.z, 0);
			}
			ImGui::EndChild();
		}
		ImGui::TreePop();
	}
}

// 선형 보간 함수: a와 b 사이를 t 비율로 보간합니다.
float4 LerpLinear(const float4& a, const float4& b, float t)
{
	return a + (b - a) * t;
}

// 스무스 스텝 보간 함수: t 값에 스무스 스텝 함수를 적용하여 보간합니다.
float4 LerpSmoothStep(const float4& a, const float4& b, float t)
{
	float smoothT = t * t * (3.0f - 2.0f * t);
	return a + (b - a) * smoothT;
}

// blendType 값에 따라 적절한 보간 방식을 선택하는 함수.
// 예: blendType == 0이면 선형 보간, blendType == 1이면 스무스 스텝 보간
float4 Interpolate(const float4& a, const float4& b, float t, int blendType)
{
	switch (blendType)
	{
	case 0: // 선형 보간
		return LerpLinear(a, b, t);
	case 1: // 스무스 스텝 보간
		return LerpSmoothStep(a, b, t);
	default:
		// 기본 보간 방식은 선형 보간으로 처리합니다.
		return LerpLinear(a, b, t);
	}
}


void LightControllComponent::Update()
{
#ifdef _EDITOR
	if (!Scene::EditorSetting.IsPlay()) return;
#endif
	auto& renderer = D3D11_GameApp::GetRenderer();
	currentTime = GameManager::GetGM().GetProgressPercentage();

	// 키프레임 데이터가 비어있는 경우를 체크
	if (LightKeyFrameData.empty())
		return;

	// currentTime이 첫 키프레임보다 작거나 같으면 첫 키프레임 사용,
	// 마지막 키프레임보다 크면 마지막 키프레임 사용.
	if (currentTime <= LightKeyFrameData.front().time)
	{
		currentCount = 0;
	}
	else if (currentTime >= LightKeyFrameData.back().time)
	{
		currentCount = static_cast<int>(LightKeyFrameData.size()) - 1;
	}
	else
	{
		// LightKeyFrameData가 시간 순으로 정렬되어 있다고 가정.
		while (currentCount < static_cast<int>(LightKeyFrameData.size()) - 1 &&
			   LightKeyFrameData[currentCount].time < currentTime)
		{
			++currentCount;
		}
	}

	// 이전 키프레임과 현재 키프레임 사이에서 보간을 진행합니다.
	if (currentCount == 0)
	{
		// 첫 키프레임인 경우에는 보간 없이 값을 그대로 사용.
		if (lightType == 0)
		{
			renderer.directLight.GetDirectLight(lightIndex).Color =
				LightKeyFrameData[currentCount].directLight.Color;
			renderer.directLight.GetDirectLight(lightIndex).Directoin =
				LightKeyFrameData[currentCount].directLight.Directoin;
		}
		else if (lightType == 1)
		{
			renderer.pointLight.GetPointLight(lightIndex).Color =
				LightKeyFrameData[currentCount].pointLight.Color;
			renderer.pointLight.GetPointLight(lightIndex).position =
				LightKeyFrameData[currentCount].pointLight.position;
		}
	}
	else
	{
		// kf0: 이전 키프레임, kf1: 현재 키프레임
		const auto& kf0 = LightKeyFrameData[currentCount - 1];
		const auto& kf1 = LightKeyFrameData[currentCount];

		float timeSpan = kf1.time - kf0.time;
		float t = (currentTime - kf0.time) / timeSpan;

		// 보간 방식은 kf1의 blendType 값을 사용 (필요에 따라 kf0이나 두 값의 조합을 사용할 수 있음)
		int blendType = kf1.blendType;

		if (lightType == 0)
		{
			// 방향성 라이트: 색상과 방향을 보간합니다.
			renderer.directLight.GetDirectLight(lightIndex).Color =
				Interpolate(kf0.directLight.Color, kf1.directLight.Color, t, blendType);
			renderer.directLight.GetDirectLight(lightIndex).Directoin =
				Interpolate(kf0.directLight.Directoin, kf1.directLight.Directoin, t, blendType);
			// 필요시 VP 행렬 보간도 고려 (단, 행렬 보간은 별도 처리 필요)
		}
		else if (lightType == 1)
		{
			// 포인트 라이트: 색상과 위치를 보간합니다.
			renderer.pointLight.GetPointLight(lightIndex).Color =
				Interpolate(kf0.pointLight.Color, kf1.pointLight.Color, t, blendType);
			renderer.pointLight.GetPointLight(lightIndex).position =
				Interpolate(kf0.pointLight.position, kf1.pointLight.position, t, blendType);
		}
	}
}

LightControllComponent::LightData::LightData() :
	time(0.0f),
	blendType(0)
{
}
