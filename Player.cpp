#include "Player.h"
#include "PlayerMove.h"
#include "Game.h"
#include "CollisionComponent.h"
#include "CameraComponent.h"


// ============================================================================
// ============================================================================
Player::Player(class Game* game)
:Actor(game)
{
	mMove = new PlayerMove(this);
	mCollision = new CollisionComponent(this);
	mCollision->SetSize(50.0f, 175.0f, 50.0f);
	mCamera = new CameraComponent(this);
}
