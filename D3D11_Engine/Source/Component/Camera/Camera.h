#pragma once
#include <Component/Base/Component.h>
#include <Core/Transform.h>

class Camera : public Component
{
public:
	static Camera* GetMainCamera() { return mainCam; }
	static void SetMainCamera(Camera* pCamear) { mainCam = pCamear; }

private:
	inline static Camera* mainCam{};

public:
	float FOV;
	float Near;
	float Far;
	bool isPerspective{ true };

public:
	static Vector2 NdcToScreenPoint(const Vector2& ndcPoint);

	Camera();
	virtual ~Camera() override;
	virtual void InspectorImguiDraw() override;
public:
	const Matrix& GetVM() const;
	const Matrix& GetIVM() const;
	const Matrix& GetPM() const;
	const Matrix& GetIPM() const;
	/*행렬을 업데이트합니다. (MainCamera는 자동으로 호출됩니다.)*/
	void UpdateCameraMatrix();

public:
	void SetMainCamera() { mainCam = this; }
	void SyncCamera(Camera* otherCamera);
	Vector3 ScreenToWorldPoint(float screenX, float screenY, float distance);
	Vector2 WorldToScreenPoint(const Vector3& worldPosition);
	Ray ScreenPointToRay(int pointX, int pointY);
public:
	virtual void Awake() override;
protected:
	virtual void FixedUpdate() override;
	virtual void Update() override;
	virtual void LateUpdate() override;

private:
	Matrix view;
	Matrix inversView;
	Matrix projection;
	Matrix inversProjection;
};