#include "Arrow.h"
#include "Game.h"
#include "MeshComponent.h"
#include "Renderer.h"
#include "Checkpoint.h"


// ============================================================================
// ============================================================================
Arrow::Arrow(class Game* game)
:Actor(game)
{
	mMesh = new MeshComponent(this);
	mMesh->SetMesh(mGame->GetRenderer()->GetMesh("Assets/Arrow.gpmesh"));
	SetScale(0.15f);
}


// ============================================================================
// ============================================================================
void Arrow::UpdateActor(float deltaTime)
{
	// If there is no next checkpoint, set mQuat to the identity quaternion
	if (GetGame()->mCheckpoints.empty())
	{
		mQuat = Quaternion::Identity;
		mPosition =
			GetGame()->GetRenderer()->Unproject(Vector3(0.0f, 250.0f, 0.1f));
		return;
	}
	
	// Vector from arrow to active checkpoint
	Checkpoint* cp = GetGame()->mCheckpoints.front();
	Vector3 pos = cp->GetPosition() - mPosition;
	pos.Normalize();
	
	// Axis
	Vector3 axis = Vector3::Cross(Vector3::UnitX, pos);
	axis.Normalize();
	
	// Angle
	float angle = acos((Vector3::Dot(Vector3::UnitX, pos)) / pos.Length());
	
	// Handle the case where the cross product is near zero
	// If the cross product length is near zero, that means the two vectors are
	// parallel, and hence they cannot form a plane
	if (Math::NearZero(axis.Length()))
	{
		mQuat = Quaternion::Identity;
	}
	
	mQuat = Quaternion(axis, angle);

	mPosition =
		GetGame()->GetRenderer()->Unproject(Vector3(0.0f, 250.0f, 0.1f));
}
