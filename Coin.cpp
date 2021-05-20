#include "Coin.h"
#include "CollisionComponent.h"
#include "Game.h"
#include "Player.h"
#include "MeshComponent.h"
#include "Renderer.h"
#include "HUD.h"


// ============================================================================
// ============================================================================
Coin::Coin(class Game* game)
	:Actor(game)
	,mRotation(0.0f)
{
	mCollision = new CollisionComponent(this);
	mCollision->SetSize(100.0f, 100.0f, 100.0f);
	mMesh = new MeshComponent(this);
	mMesh->SetMesh(mGame->GetRenderer()->GetMesh("Assets/Coin.gpmesh"));
}


// ============================================================================
// ============================================================================
void Coin::UpdateActor(float deltaTime)
{
	// Do we collide with the player?
	Player* player = mGame->GetPlayer();
	if (GetCollision()->Intersect(player->GetCollision()))
	{
		SetState(State::EDead);
		Mix_PlayChannel(-1, mGame->GetSound("Assets/Sounds/Coin.wav"), 0);
		
		// Update coin text
		GetGame()->GetHUD()->UpdateCoinCount();
	}
	
	// Rotate at pi radians / second
	mRotation += Math::Pi * deltaTime;
	SetRotation(mRotation);
}
