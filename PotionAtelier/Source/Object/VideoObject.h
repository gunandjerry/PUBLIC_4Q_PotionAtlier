#pragma once
#include "framework.h"
class VideoObject : public GameObject
{
	SERIALIZED_OBJECT(VideoObject)
public:
	VideoObject();
	virtual void Awake();
};

