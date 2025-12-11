#pragma once
#include <Scene/Base/Scene.h>

class ImageCompressorScene : public Scene
{
public:
	static void PngHandler(const wchar_t* filepath);
public:
	ImageCompressorScene() = default;
	virtual ~ImageCompressorScene() override = default;
	virtual void Start() override;

	virtual void ImGUIRender();
};