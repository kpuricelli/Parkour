#include "Checkpoint.h"
#include "Player.h"
#include "PlayerMove.h"
#include "Game.h"
#include "CollisionComponent.h"
#include "MeshComponent.h"
#include "Renderer.h"
#include "HUD.h"


// ============================================================================
// ============================================================================
Checkpoint::Checkpoint(class Game* game)
	:Actor(game)
	,mLevelString("")
	,mCheckpointString("")
	,mTextTime(0.0f)
{
	mCollision = new CollisionComponent(this);
	mCollision->SetSize(25.0f, 25.0f, 25.0f);
	mMesh = new MeshComponent(this);
	mMesh->SetMesh(mGame->GetRenderer()->GetMesh("Assets/Checkpoint.gpmesh"));
}


// ============================================================================
// ============================================================================
void Checkpoint::UpdateActor(float deltaTime)
{
	if (mCollision)
	{
		mCollision->Update(deltaTime);
		auto& queue = GetGame()->mCheckpoints;
		if (queue.empty())
		{
			return;
		}
		
		// Active checkpoint
		Checkpoint* cp = queue.front();
		
		// Update this' mTextTime and if the current checkpoint string has been
		// shown for more than 5 seconds, erase it from the screen
		if (this == cp)
		{
			cp->mTextTime += deltaTime;
			
			if (cp->mTextTime > 5.0f)
			{
				GetGame()->GetHUD()->UpdateCheckpointText(" ");
			}
		}
		
		// If the timer for the last checkpoint has been set
		if (GetGame()->GetLastCheckpointTimer())
		{
			GetGame()->AddToLastCheckpointTimer(deltaTime / queue.size());
			
			// Destroy last tutorial checkpoint text
			if (GetGame()->GetLastCheckpointTimer() > 5.0f)
			{
				GetGame()->GetHUD()->UpdateCheckpointText(" ");
				GetGame()->ResetLastCheckpointTimer();
			}
		}
		
		// If we collide with the active checkpoint
		if (cp->GetCollision()->Intersect(GetGame()->GetPlayer()->GetCollision()))
		{
			// Update respawn position to be this checkpoint, remove from queue
			GetGame()->GetPlayer()->SetRespawnPos(cp->GetPosition());
			GetGame()->mDeadCheckpoints.push_back(cp);
			queue.pop();
			
			// Set the new active checkpoint to blue
			if (!queue.empty())
			{
				queue.front()->mMesh->SetTextureIndex(0);
				GetGame()->GetHUD()->UpdateCheckpointText(cp->mCheckpointString);
			}
			else
			{
				GetGame()->GetHUD()->UpdateCheckpointText(cp->mCheckpointString);
				GetGame()->AddToLastCheckpointTimer(deltaTime);
			}
			
			// Play sound
			Mix_PlayChannel(-1,
							mGame->GetSound("Assets/Sounds/Checkpoint.wav"),
							0);
			
			// If checkpoint has a level string, set the next level
			if (cp->mLevelString != "")
			{
				GetGame()->SetNextLevel(cp->mLevelString);
			}
		}
	}
	if (mMesh)
	{
		mMesh->Update(deltaTime);
	}
}
