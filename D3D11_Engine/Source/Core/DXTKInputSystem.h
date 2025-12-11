#pragma once
#include <Windows.h>
#include <Core/TSingleton.h>
#include <directXTK/Mouse.h>
#include <directXTK/Keyboard.h>
#include <vector>
#include <windef.h>
#include <Core/GameInputSystem.h>

using namespace DirectX;
class DXTKInputSystem;
extern DXTKInputSystem& DXTKinputSystem;

enum class MouseKeys
{
	leftButton,
	middleButton,
	rightButton,
	xButton1,
	xButton2
};

class DXTKInputSystem : public TSingleton<DXTKInputSystem>
{
	template <typename T>
	friend class TSingleton;
private:
	DXTKInputSystem();
	~DXTKInputSystem() = default;

private:
	HWND hWnd{};
	std::unique_ptr<DirectX::Mouse>                 mouse;

public:
	void Initialize(HWND hWnd);
	void Update();

	bool IsKeyDown(MouseKeys key) const;
	bool IsKey(MouseKeys key) const;
	bool IsKeyUp(MouseKeys key) const;

	int GetRealtiveMouseWheel() const;
	void SetMouseMode(DirectX::Mouse::Mode mode);
	const DirectX::Mouse::State GetMouseState() const;

private:
	DirectX::Mouse::ButtonStateTracker              mouseStateTracker;
};


