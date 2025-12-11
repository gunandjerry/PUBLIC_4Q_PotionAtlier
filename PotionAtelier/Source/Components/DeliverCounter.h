#pragma once
#include "framework.h"
#include "Interactable.h"

class DeliverCounter : public Interactable
{
	class TextPoping* earning_ui{ nullptr };
	Vector2 start_ui_anchor{ 0, -60.0f };
	Vector2 goal_ui_anchor{ 0, -90.f };
	bool ui_show{ false };
	float earning_t{ 0 };
	float ui_speed = 2.0f;

	Color init_color{ 1,0.843f,0,1 };
	Color goal_color{ 1,0.843f,0,0 };

	PBRMeshRender* face_quad{ nullptr };

	bool calm_down_it_just_a_kind_of_joke{ false };

	int face_step{ 0 };
	float show_face_t{ 0.0f };
	bool punch{ false };
	float punch_point_t{ 0.7f };
	float show_face_time{ 1.5f };
	float face_additional_maintain_time{ 2.0f };
	float face_animation_time{ 5.0f };
	TransformAnimation* animator{ nullptr };


	class AudioBankClip* SFX_Laughter{ nullptr };
	class AudioBankClip* SFX_Drawer{ nullptr };
	class AudioBankClip* SFX_GetHit{ nullptr };
	class AudioBankClip* SFX_Fly{ nullptr };

public:
	DeliverCounter();
	virtual void Awake();
	virtual void Start();
protected:
	virtual void FixedUpdate() {}
	virtual void Update();
	virtual void LateUpdate() {}


public:
	virtual void OnFocusIn(class PlayerController* controller);
	virtual void OnFocusOut(class PlayerController* controller);
	virtual bool OnInteract(class PlayerController* controller);
	virtual void OnAttacked(class PlayerController* controller);

	class ParticleSpawnComponent* VFX_Serve{ nullptr };

	inline static constexpr const wchar_t* VFX_ServeDataPath = L"Resource/VFX/VFX_Serve.BinaryData";
	std::string serve{};

public:
	void ShowFace();
	void StopTremble();
	void TrembleLoop();
	void Punch();

	void Tutorial_ShowFace();
	void Tutorial_PunchAndHideFace();


private:
	void FindFaceQuad(Transform* parent);
	void FindAnimator(Transform* parent);

public:
	virtual void InspectorImguiDraw() override;
	virtual void Serialized(std::ofstream& ofs) override {}
	virtual void Deserialized(std::ifstream& ifs) override {}

private:
	void SaveVFX();
	void LoadVFX();

};