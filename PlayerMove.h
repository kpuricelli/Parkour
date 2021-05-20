#pragma once
#include "MoveComponent.h"
#include "Math.h"

class PlayerMove : public MoveComponent
{
public:
	enum MoveState
	{
		OnGround, Jump, Falling, WallClimb, WallRun
	};
	
	enum CollSide
	{
		None, Top, Bottom, SideX1, SideX2, SideY1, SideY2
	};
	
	PlayerMove(class Actor* owner);
	virtual ~PlayerMove();
	
	void Update(float deltaTime) override;
	void ProcessInput(const Uint8* keyState) override;
	
	void ChangeState(MoveState state) { mCurrentState = state; }
	
	void PhysicsUpdate(float deltaTime);
	void AddForce(const Vector3& force) { mPendingForces += force; }
	
	void FixXYVelocity();
	
protected:
	void UpdateOnGround(float deltaTime);
	void UpdateJump(float deltaTime);
	void UpdateFalling(float deltaTime);
	void UpdateWallClimb(float deltaTime);
	void UpdateWallRun(float deltaTime);
	bool CanWallClimb(CollSide side);
	bool CanWallRun(CollSide side);
	
	CollSide FixCollision(class CollisionComponent* self,
						  class CollisionComponent* block);
	
	MoveState mCurrentState;
	
	Vector3 mVelocity;
	Vector3 mAcceleration;
	Vector3 mPendingForces;
	Vector3 mGravity;
	Vector3 mJumpForce;
	Vector3 mWallClimbForce;
	Vector3 mWallRunForce;
	float mMass;
	float mWallClimbTimer;
	float mWallRunTimer;
	int mRunningSFX;
	bool mSpacePressed;
	bool mPlayedSound;
};
