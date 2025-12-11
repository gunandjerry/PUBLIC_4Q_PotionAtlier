#include "StartApp.h"
#include "framework.h"
#include <Utility/IconsFontAwesome4.h>

StartApp::StartApp()
{
    this->windowName = L"PotionAtelier";
    this->SetBorderlessWindowed();
    //this->SetOptimalScreenSize();
}

StartApp::~StartApp()
{

}

void StartApp::AddFont(ImFontConfig* fontConfig)
{
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	fontConfig->MergeMode = true;
	const ImWchar iconRanges[] = { ICON_MIN_FA, ICON_MAX_FA, 0 };
	std::string iconPath = "Resource/IconFont/";
				iconPath += FONT_ICON_FILE_NAME_FA;
	io.Fonts->AddFontFromFileTTF(iconPath.c_str(), 15.0f, fontConfig, iconRanges);
	io.Fonts->Build();
}