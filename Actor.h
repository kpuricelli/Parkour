#pragma once
#include <vector>
#include <SDL/SDL_stdinc.h>
#include "Math.h"
class Actor
{
public:
	typedef enum
	{
		EActive,
		EPaused,
		EDead
	} State;
	
	Actor(class Game* game);
	virtual ~Actor();

	// Update function called from Game (not overridable)
	void Update(float deltaTime);
	
	// Any actor-specific update code (overridable)
	virtual void UpdateActor(float deltaTime);
	
	// ProcessInput function called from Game (not overridable)
	void ProcessInput(const Uint8* keyState);
	
	// Any actor-specific update code (overridable)
	virtual void ActorInput(const Uint8* keyState);

	// Getters/setters
	const Vector3& GetPosition() const { return mPosition; }
	void SetPosition(const Vector3& pos) { mPosition = pos; }
	
	float GetScale() const { return mScale; }
	void SetScale(float scale) { mScale = scale; }
	
	float GetRotation() const { return mRotation; }
	void SetRotation(float rotation) { mRotation = rotation; }
	
	Vector3 GetForward() const
		{ return Vector3(Math::Cos(mRotation), Math::Sin(mRotation), 0.0f); }
	
	Vector3 GetRight() const
		{ return Vector3(Math::Cos(mRotation + Math::PiOver2),
						 Math::Sin(mRotation + Math::PiOver2), 0.0f); }
	
	State GetState() const { return mState; }
	void SetState(State state) { mState = state; }

	class Game* GetGame() { return mGame; }
	class MoveComponent* GetMove() { return mMove; }
	class CollisionComponent* GetCollision() { return mCollision; }
	
	const Matrix4& GetWorldTransform() const { return mWorldTransform; }
	
	class MeshComponent* GetMesh() const { return mMesh; }
	
	class CameraComponent* GetCamera() const { return mCamera; }
	
	Quaternion GetQuaternion() const { return mQuat; }
	void SetQuaternion(const Quaternion& quat) { mQuat = quat; }
	
protected:
	class Game* mGame;
	
	// Components
	class MoveComponent* mMove;
	class CollisionComponent* mCollision;
	class MeshComponent* mMesh;
	class CameraComponent* mCamera;
	
	// Transform
	Matrix4 mWorldTransform;
	Vector3 mPosition;
	
	Quaternion mQuat;
	
	// Actor's state
	State mState;
	
	float mScale;
	float mRotation;
};
