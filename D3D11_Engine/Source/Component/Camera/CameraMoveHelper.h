#pragma once
#include <Component\Base\Component.h>
#include <Manager/InputManager.h>	

class CameraMoveHelper : public Component, public InputProcesser 
{
public:
	CameraMoveHelper();
	virtual ~CameraMoveHelper() override = default;

public:
	virtual void Awake()		override;
	virtual void InspectorImguiDraw() override;
protected:
	virtual void FixedUpdate()  override;
	virtual void Update()		override;
	virtual void LateUpdate()	override;

public:
	virtual void OnInputProcess(InputManager::Input& input) override;

public:
	/*카메라 무브먼트를 실행합니다. (MainCamera면 자동으로 호출합니다.)*/
	void UpdateMovemont();

private:
	void AddPitch(float angle);
	void AddYaw(float angle);

public:
	float moveSpeed;
	float rotSpeed;

private:
	Vector3 inputVector{};
	float yawRotation = 0;
	float pitchRotation = 0;
	float inputSpeed;
};