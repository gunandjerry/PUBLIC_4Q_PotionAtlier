#include "CameraObject.h"
#include <Manager/SceneManager.h>
#include <Utility/SerializedUtility.h>
#include <Component/Camera/CameraMoveHelper.h>
#include <Utility/ImguiHelper.h>

CameraObject::CameraObject()
{
	cam = &AddComponent<Camera>();
}

CameraObject::~CameraObject()
{


}

void CameraObject::Serialized(std::ofstream& ofs)
{
	using namespace Binary;
	constexpr uint32_t version = 1;
	Write::data<uint8_t	>(ofs, (std::numeric_limits<uint8_t>::max)()); //Çì´õ
	Write::data(ofs, version);

	bool isMainCam = false;
	if (cam == Camera::GetMainCamera())
	{
		isMainCam = true;
	}
	Write::data<bool>(ofs, isMainCam);

	bool isHelper = !!IsComponent<CameraMoveHelper>();
	Write::data<bool>(ofs, isHelper);

	if (version > 0)
	{
		Write::data(ofs, cam->FOV);
		Write::data(ofs, cam->Near);
		Write::data(ofs, cam->Far);
		Write::data(ofs, cam->isPerspective);
	}
}

void CameraObject::Deserialized(std::ifstream& ifs)
{
	using namespace Binary;
	uint32_t version = 0;
	uint8_t header = Read::data<uint8_t>(ifs);
	bool isMainCam;
	if (header == (std::numeric_limits<uint8_t>::max)())
	{
		version = Read::data<uint32_t>(ifs);
		isMainCam = Read::data<bool>(ifs);
	}
	else
	{
		isMainCam = bool(header);
	}
	if (isMainCam)
	{
		SetMainCamera();
	}
	if (bool isHelper = Read::data<bool>(ifs))
	{
		AddComponent<CameraMoveHelper>();
	}

	if (version > 0)
	{
		cam->FOV = Read::data<float>(ifs);
		cam->Near = Read::data<float>(ifs);
		cam->Far = Read::data<float>(ifs);
		cam->isPerspective = Read::data<bool>(ifs);
	}
}

void CameraObject::InspectorImguiDraw()
{
	if (ImGui::Button("Set Main Camera"))
	{
		SetMainCamera();
	}
	GameObject::InspectorImguiDraw();
}

void CameraObject::SetMainCamera()
{
	cam->SetMainCamera();
}

