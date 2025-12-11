#pragma once
#include <D3DCore/D3D11_GameApp.h>
#include <Manager/SceneManager.h>

class ImageCompressorApp : public D3D11_GameApp
{
public:
	ImageCompressorApp();
	~ImageCompressorApp();

protected:
	virtual void AddFont(ImFontConfig* fontConfig) override;
};