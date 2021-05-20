#include "Actor.h"
#include "Game.h"
#include "Component.h"
#include "MoveComponent.h"
#include "CollisionComponent.h"
#include "MeshComponent.h"
#include "CameraComponent.h"
#include "Coin.h"


// ============================================================================
// ============================================================================
Actor::Actor(Game* game)
	:mGame(game)
	,mState(EActive)
	,mPosition(Vector3::Zero)
	,mScale(1.0f)
	,mRotation(0.0f)
	,mMove(nullptr)
	,mCollision(nullptr)
	,mMesh(nullptr)
	,mCamera(nullptr)
{
	mGame->AddActor(this);
}


// ============================================================================
// ============================================================================
Actor::~Actor()
{
	if (mMove)
		delete mMove;
	if (mCollision)
		delete mCollision;
	if (mMesh)
		delete mMesh;
	if (mCamera)
		delete mCamera;
	
	mGame->RemoveActor(this);
}


// ============================================================================
// ============================================================================
void Actor::Update(float deltaTime)
{
	if (mState == EActive)
	{
		if (mMove)
		{
			mMove->Update(deltaTime);
		}
		if (mCamera)
		{
			mCamera->Update(deltaTime);
		}
		if (mCollision)
		{
			mCollision->Update(deltaTime);
		}
		if (mMesh)
		{
			mMesh->Update(deltaTime);
		}
		UpdateActor(deltaTime);
		
		// Set the world transform:
		// worldMatrix = scaleMatrix * rotationMatrix * positionMatrix
		const Matrix4 scaleMatrix = Matrix4::CreateScale(mScale, mScale, mScale);
		const Matrix4 rotationMatrix = Matrix4::CreateRotationZ(mRotation);
		const Matrix4 positionMatrix = Matrix4::CreateTranslation(mPosition);
		
		mWorldTransform = scaleMatrix * rotationMatrix *
			Matrix4::CreateFromQuaternion(mQuat) * positionMatrix;
	}
}


// ============================================================================
// ============================================================================
void Actor::UpdateActor(float deltaTime)
{
	
}


// ============================================================================
// ============================================================================
void Actor::ProcessInput(const Uint8* keyState)
{
	if (mState == EActive)
	{
		// Process input for move component
		if (mMove)
		{
			mMove->ProcessInput(keyState);
		}
		if (mCamera)
		{
			mCamera->ProcessInput(keyState);
		}
		if (mCollision)
		{
			mCollision->ProcessInput(keyState);
		}
		if (mMesh)
		{
			mMesh->ProcessInput(keyState);
		}
		
		// Process actor input
		ActorInput(keyState);
	}
}


// ============================================================================
// ============================================================================
void Actor::ActorInput(const Uint8* keyState)
{
	
}
