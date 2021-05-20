#include "CameraComponent.h"
#include "Math.h"
#include "Game.h"
#include "Actor.h"
#include "Renderer.h"


// ============================================================================
// ============================================================================
CameraComponent::CameraComponent(class Actor* owner)
:Component(owner)
,mPitchAngle(0.0f)
,mPitchSpeed(0.0f)
{
	
}


// ============================================================================
// ============================================================================
void CameraComponent::Update(float deltaTime)
{
	mPitchAngle += mPitchSpeed * deltaTime;
	if (mPitchAngle < -Math::PiOver4)
	{
		mPitchAngle = -Math::PiOver4;
	}
	else if (mPitchAngle > Math::PiOver4)
	{
		mPitchAngle = Math::PiOver4;
	}
	
	Matrix4 yaw = Matrix4::CreateRotationZ(mOwner->GetRotation());
	Matrix4 pitch = Matrix4::CreateRotationY(mPitchAngle);
	
	Matrix4 rotation = pitch * yaw;
	Vector3 forward = Vector3::Transform(Vector3::UnitX, rotation);
	Vector3 pos = mOwner->GetPosition();
	Vector3 target = forward + pos;
	
	Matrix4 mat4 = Matrix4::CreateLookAt(pos, target, Vector3::UnitZ);
	mOwner->GetGame()->GetRenderer()->SetViewMatrix(mat4);
}
