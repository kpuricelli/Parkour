#include "PlayerMove.h"
#include "Actor.h"
#include "Player.h"
#include "Block.h"
#include "Game.h"
#include "CollisionComponent.h"
#include "CameraComponent.h"
#include <SDL/SDL.h>


// ============================================================================
// ============================================================================
PlayerMove::PlayerMove(class Actor* owner)
:MoveComponent(owner)
,mMass(1.0f)
,mSpacePressed(false)
,mGravity(Vector3(0.0f, 0.0f, -980.0f))
,mJumpForce(Vector3(0.0f, 0.0f, 35000.0f))
,mWallClimbForce(Vector3(0.0f, 0.0f, 1800.0f))
,mWallRunForce(Vector3(0.0f, 0.0f, 1200.0f))
,mWallClimbTimer(0.0f)
,mWallRunTimer(0.0f)
,mPlayedSound(false)
{
	mRunningSFX =
		Mix_PlayChannel(-1,
						mOwner->GetGame()->GetSound("Assets/Sounds/Running.wav"),
						-1);
	Mix_Pause(mRunningSFX);
	ChangeState(MoveState::Falling);
}


// ============================================================================
// ============================================================================
PlayerMove::~PlayerMove()
{
	Mix_HaltChannel(mRunningSFX);
}


// ============================================================================
// ============================================================================
void PlayerMove::Update(float deltaTime)
{
	// Respawn
	if (mOwner->GetPosition().z < -750.0f)
	{
		mOwner->SetPosition(static_cast<Player*>(mOwner)->GetRespawnPos());
		mOwner->SetRotation(0.0f);
		mVelocity = Vector3::Zero;
		mPendingForces = Vector3::Zero;
		ChangeState(MoveState::Falling);
	}
	
	// Sound
	if ((mCurrentState == MoveState::OnGround && mVelocity.Length() > 50.0f) ||
		 mCurrentState == MoveState::WallClimb ||
		 mCurrentState == MoveState::WallRun)
	{
		Mix_Resume(mRunningSFX);
	}
	else
	{
		Mix_Pause(mRunningSFX);
	}
	
	switch (mCurrentState)
	{
		case OnGround:
			UpdateOnGround(deltaTime);
			break;
		case Jump:
			UpdateJump(deltaTime);
			break;
		case Falling:
			UpdateFalling(deltaTime);
			break;
		case WallClimb:
			UpdateWallClimb(deltaTime);
			break;
		case WallRun:
			UpdateWallRun(deltaTime);
			break;
		default:
			break;
	}
}


// ============================================================================
// ============================================================================
void PlayerMove::ProcessInput(const Uint8* keyState)
{
	// Forward / back
	if (keyState[SDL_SCANCODE_W])
	{
		AddForce(mOwner->GetForward() * 700.0f);
	}
	else if (keyState[SDL_SCANCODE_S])
	{
		AddForce(mOwner->GetForward() * -700.0f);
	}
	
	// Strafe
	if (keyState[SDL_SCANCODE_D])
	{
		AddForce(mOwner->GetRight() * 700.0f);
	}
	else if (keyState[SDL_SCANCODE_A])
	{
		AddForce(mOwner->GetRight() * -700.0f);
	}
	
	int x, y;
	SDL_GetRelativeMouseState(&x, &y);
	float xf = x / 500.0f;
	xf *= Math::Pi * 10.0f;
	SetAngularSpeed(xf);
	
	float yf = y / 500.0f;
	yf *= Math::Pi * 10.0f;
	mOwner->GetCamera()->SetPitchSpeed(yf);
	
	// Check for leading edge of space bar (jump)
	if (!mSpacePressed && keyState[SDL_SCANCODE_SPACE] &&
		mCurrentState == MoveState::OnGround)
	{
		mSpacePressed = true;
		AddForce(mJumpForce);
		ChangeState(MoveState::Jump);
	}
	else if (!keyState[SDL_SCANCODE_SPACE])
	{
		mSpacePressed = false;
	}
}


// ============================================================================
// ============================================================================
void PlayerMove::UpdateOnGround(float deltaTime)
{
	PhysicsUpdate(deltaTime);
	bool onTop = false;
	bool onNothing = false;
	const auto& blocks = mOwner->GetGame()->GetBlocks();
	
	// Fix collision on every block and check if we need to change state
	for (auto& block : blocks)
	{
		CollSide side = FixCollision(mOwner->GetCollision(),
									 block->GetCollision());
		
		if (side == CollSide::Top)
		{
			onTop = true;
		}
		
		else if (side == CollSide::None)
		{
			onNothing = true;
		}
		
		else if (side == CollSide::SideX1 || side == CollSide::SideX2 ||
				 side == CollSide::SideY1 || side == CollSide::SideY2)
		{
			if (CanWallClimb(side))
			{
				ChangeState(MoveState::WallClimb);
				mWallClimbTimer = 0.0f;
				return;
			}
		}
	}
	
	if (onNothing && !onTop)
	{
		ChangeState(MoveState::Falling);
	}
}


// ============================================================================
// ============================================================================
void PlayerMove::UpdateJump(float deltaTime)
{
	AddForce(mGravity);
	PhysicsUpdate(deltaTime);
	
	// Only play the jump sound once per jump
	if (!mPlayedSound)
	{
		Mix_PlayChannel(-1,
						mOwner->GetGame()->GetSound("Assets/Sounds/Jump.wav"),
						0);
		mPlayedSound = true;
	}
	
	const auto& blocks = mOwner->GetGame()->GetBlocks();
	for (auto& block : blocks)
	{
		CollSide side = FixCollision(mOwner->GetCollision(),
									 block->GetCollision());
		
		// Hit something above us
		if (side == CollSide::Bottom)
		{
			mVelocity.z = 0.0f;
		}
		
		else if (side == CollSide::SideX1 || side == CollSide::SideX2 ||
				 side == CollSide::SideY1 || side == CollSide::SideY2)
		{
			if (CanWallClimb(side))
			{
				ChangeState(MoveState::WallClimb);
				mWallClimbTimer = 0.0f;
				return;
			}
			else if (CanWallRun(side))
			{
				ChangeState(MoveState::WallRun);
				mWallRunTimer = 0.0f;
				return;
			}
		}
	}
	
	// Check for the apex of jump
	if (mVelocity.z <= 0.0f)
	{
		mPlayedSound = false;
		ChangeState(MoveState::Falling);
	}
}


// ============================================================================
// ============================================================================
void PlayerMove::UpdateFalling(float deltaTime)
{
	AddForce(mGravity);
	PhysicsUpdate(deltaTime);
	
	// Fix collision on all blocks
	const auto& blocks = mOwner->GetGame()->GetBlocks();
	for (auto& block : blocks)
	{
		// Done falling
		if (FixCollision(mOwner->GetCollision(), block->GetCollision()) ==
			CollSide::Top)
		{
			mVelocity.z = 0.0f;
			Mix_PlayChannel(-1,
							mOwner->GetGame()->GetSound("Assets/Sounds/Land.wav"),
							0);
			ChangeState(MoveState::OnGround);
		}
	}
}


// ============================================================================
// ============================================================================
void PlayerMove::UpdateWallClimb(float deltaTime)
{
	mWallClimbTimer += deltaTime;
	AddForce(mGravity);
	PhysicsUpdate(deltaTime);
	
	if (mWallClimbTimer < 0.4f)
	{
		AddForce(mWallClimbForce);
	}
	else
	{
		ChangeState(MoveState::Falling);
		return;
	}
	
	// Fix collision on all blocks
	bool climbing = false;
	const auto& blocks = mOwner->GetGame()->GetBlocks();
	for (auto& block : blocks)
	{
		CollSide side = FixCollision(mOwner->GetCollision(),
									 block->GetCollision());
		
		// If you don’t collide with the side of any blocks or
		// mVelocity.z <= 0.0f, set mVelocity.z to 0.0f and change
		// state to Falling
		if (side == CollSide::SideX1 || side == CollSide::SideX2 ||
			side == CollSide::SideY1 || side == CollSide::SideY2)
		{
			climbing = true;
		}
		if (mVelocity.z <= 0.0f)
		{
			climbing = true;
		}
	}
	if (!climbing)
	{
		mVelocity.z = 0.0f;
		ChangeState(MoveState::Falling);
	}
}


// ============================================================================
// ============================================================================
void PlayerMove::UpdateWallRun(float deltaTime)
{
	mWallRunTimer += deltaTime;
	AddForce(mGravity);
	PhysicsUpdate(deltaTime);
	if (mWallRunTimer < 0.4f)
	{
		AddForce(mWallRunForce);
	}
	else
	{
		ChangeState(MoveState::Falling);
		return;
	}
	
	// Fix collision on all blocks
	bool doneRunning = false;
	auto blocks = mOwner->GetGame()->GetBlocks();
	for (auto& block : blocks)
	{
		FixCollision(mOwner->GetCollision(), block->GetCollision());

		if (mVelocity.z <= 0.0f)
		{
			doneRunning = true;
		}
	}
	if (doneRunning)
	{
		mVelocity.z = 0.0f;
		ChangeState(MoveState::Falling);
	}
}


// ============================================================================
// ============================================================================
PlayerMove::CollSide PlayerMove::FixCollision(CollisionComponent* self,
											  CollisionComponent* block)
{
	Vector3 pos = mOwner->GetPosition();
	if (!self->Intersect(block))
	{
		return CollSide::None;
	}
	
	// Get player min/max and block min/max
	const Vector3 playerMin = mOwner->GetCollision()->GetMin();
	const Vector3 playerMax = mOwner->GetCollision()->GetMax();
	const Vector3 blockMin = block->GetMin();
	const Vector3 blockMax = block->GetMax();
	
	// Figure out which side we are closest to
	float dx1 = blockMin.x - playerMax.x;
	float dx2 = blockMax.x - playerMin.x;
	float dy1 = blockMin.y - playerMax.y;
	float dy2 = blockMax.y - playerMin.y;
	float dz1 = blockMin.z - playerMax.z;
	float dz2 = blockMax.z - playerMin.z;
	CollSide wallSideX;
	CollSide wallSideY;
	Vector3 force = Vector3::Zero;
	
	float dx = 0.0f;
	if (Math::Abs(dx1) < Math::Abs(dx2))
	{
		dx = dx1;
		wallSideX = CollSide::SideX1;
		force.x = -700.0f;
	}
	else
	{
		dx = dx2;
		wallSideX = CollSide::SideX2;
		force.x = 700.0f;
	}
	
	float dy = 0.0f;
	if (Math::Abs(dy1) < Math::Abs(dy2))
	{
		dy = dy1;
		wallSideY = CollSide::SideY1;
		force.y = -700.0f;
	}
	else
	{
		dy = dy2;
		wallSideY = CollSide::SideY2;
		force.y = 700.0f;
	}
	
	float dz = 0.0f;
	if (Math::Abs(dz1) < Math::Abs(dz2))
	{
		dz = dz1;
	}
	else
	{
		dz = dz2;
	}
	
	// Fix position
	if (Math::Abs(dy) < Math::Abs(dx) && Math::Abs(dy) < Math::Abs(dz))
	{
		pos.y += dy;
	}
	else if (Math::Abs(dx) < Math::Abs(dy) && Math::Abs(dx) < Math::Abs(dz))
	{
		pos.x += dx;
	}
	else if (Math::Abs(dz) < Math::Abs(dy) && Math::Abs(dz) < Math::Abs(dx))
	{
		pos.z += dz;
	}
	mOwner->SetPosition(pos);
	
	// Return the proper side
	if (Math::Abs(dz) > Math::Abs(dy) && Math::Abs(dz) > Math::Abs(dx) &&
		Math::Abs(pos.z) >= Math::Abs(blockMax.z))
	{
		return CollSide::Bottom;
	}
	else if (Math::Abs(dz) < Math::Abs(dy) && Math::Abs(dz) < Math::Abs(dx))
	{
		return CollSide::Top;
	}
	else if (Math::Abs(dx) < Math::Abs(dy))
	{
		AddForce(force);
		return wallSideX;
	}
	else if (Math::Abs(dy) < Math::Abs(dx))
	{
		AddForce(force);
		return wallSideY;
	}
	return CollSide::None;
}


// ============================================================================
// ============================================================================
void PlayerMove::PhysicsUpdate(float deltaTime)
{
	mAcceleration = mPendingForces * (1.0f / mMass);
	mVelocity += mAcceleration * deltaTime;
	FixXYVelocity();
	mOwner->SetPosition(mOwner->GetPosition() + mVelocity * deltaTime);
	
	float rot = mOwner->GetRotation();
	rot += mOwner->GetMove()->GetAngularSpeed() * deltaTime;
	mOwner->SetRotation(rot);
	
	mPendingForces = Vector3::Zero;
}


// ============================================================================
// ============================================================================
void PlayerMove::FixXYVelocity()
{
	Vector2 tempVelocity = Vector2(mVelocity.x, mVelocity.y);
	
	// Fix the max speed
	if (tempVelocity.Length() > 400.0f)
	{
		tempVelocity.Normalize();
		tempVelocity *= 400.0f;
	}
	
	// Apply the velocity braking code if the acceleration component has the
	// opposite sign of the xyVelocity component
	if (mCurrentState == MoveState::OnGround ||
		mCurrentState == MoveState::WallClimb)
	{
		if ((Math::NearZero(mAcceleration.x)) ||
			(mAcceleration.x < 0 && tempVelocity.x > 0) ||
			(mAcceleration.x > 0 && tempVelocity.x < 0))
		{
			tempVelocity.x *= 0.9f;
		}
		if ((Math::NearZero(mAcceleration.y)) ||
			(mAcceleration.y < 0 && tempVelocity.y > 0) ||
			(mAcceleration.y > 0 && tempVelocity.y < 0))
		{
			tempVelocity.y *= 0.9f;
		}
	}
	
	// Update mVelocity
	mVelocity.x = tempVelocity.x;
	mVelocity.y = tempVelocity.y;
}


// ============================================================================
// ============================================================================
bool PlayerMove::CanWallClimb(CollSide side)
{
	Vector3 playerForward = mOwner->GetForward();
	Vector3 blockNormal;
	if (side == PlayerMove::SideX1)
	{
		blockNormal = Vector3::NegUnitX;
	}
	else if (side == PlayerMove::SideX2)
	{
		blockNormal = Vector3::UnitX;
	}
	else if (side == PlayerMove::SideY1)
	{
		blockNormal = Vector3::NegUnitY;
	}
	else if (side == PlayerMove::SideY2)
	{
		blockNormal = Vector3::UnitY;
	}
	
	// Check if the player is facing side
	const float playerBlockDot = Vector3::Dot(playerForward, blockNormal);
	if (Math::Abs(playerBlockDot) < 1.10f &&
		Math::Abs(playerBlockDot) > 0.90f)
	{
		Vector3 velocityNorm = mVelocity;
		velocityNorm.Normalize();
		const float velocityBlockDot = Vector3::Dot(velocityNorm, blockNormal);
		if (Math::Abs(velocityBlockDot) < 1.10f &&
			Math::Abs(velocityBlockDot) > 0.90f)
		{
			if (mVelocity.Length() >= 350.0f)
			{
				return true;
			}
		}
	}
	return false;
}


// ============================================================================
// ============================================================================
bool PlayerMove::CanWallRun(CollSide side)
{
	Vector3 playerForward = mOwner->GetForward();
	Vector3 blockNormal;
	if (side == PlayerMove::SideX1)
	{
		blockNormal = Vector3::NegUnitX;
	}
	else if (side == PlayerMove::SideX2)
	{
		blockNormal = Vector3::UnitX;
	}
	else if (side == PlayerMove::SideY1)
	{
		blockNormal = Vector3::NegUnitY;
	}
	else if (side == PlayerMove::SideY2)
	{
		blockNormal = Vector3::UnitY;
	}
	
	// Check if the player is facing side
	const float playerBlockDot = Vector3::Dot(playerForward, blockNormal);
	if (Math::Abs(playerBlockDot) < 0.9f &&
		Math::Abs(playerBlockDot) > 0.1f)
	{
		Vector3 velocityNorm = mVelocity;
		velocityNorm.Normalize();
		const float velocityBlockDot = Vector3::Dot(velocityNorm, blockNormal);
		if (Math::Abs(velocityBlockDot) < 0.9f &&
			Math::Abs(velocityBlockDot) > 0.1f)
		{
			if (mVelocity.Length() >= 350.0f)
			{
				return true;
			}
		}
	}
	return false;
}
