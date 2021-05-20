#pragma once
#include "Component.h"

class CameraComponent : public Component
{
public:
	CameraComponent(class Actor* actor);
	void Update(float deltaTime) override;
	
	float GetPitchSpeed() const { return mPitchSpeed; }
	void SetPitchSpeed(float speed) { mPitchSpeed = speed; }
	
private:
	float mPitchAngle;
	float mPitchSpeed;
};
