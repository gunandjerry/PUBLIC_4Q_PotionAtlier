#pragma once


#include <Component\Base\Component.h>
#include <Light.h>

class LightControllComponent : public Component
{
public:
	LightControllComponent();
	virtual ~LightControllComponent() override = default;

public:
	virtual void Serialized(std::ofstream& ofs);
	virtual void Deserialized(std::ifstream& ifs);
	virtual void InspectorImguiDraw();

public:
	virtual void Awake() {}
	virtual void Start() 
	{
		currentCount = 0;
	}
protected:
	virtual void FixedUpdate() {}
	virtual void Update();
	virtual void LateUpdate() {}

	int lightType = 0;
	int lightIndex = 0;
	struct LightData 
	{
		LightData();

		float time;
		int blendType;
		union 
		{
			DirectionLightData directLight;
			PointLightData pointLight;
		};;

	};

	std::vector<LightData> LightKeyFrameData;


	int currentCount;
	float currentTime;
};

