#include "Game.h"
#include "Block.h"
#include "Actor.h"
#include "Renderer.h"
#include "LevelLoader.h"
#include "MeshComponent.h"
#include "Checkpoint.h"
#include "Arrow.h"
#include "HUD.h"
#include "SDL/SDL_mixer.h"
#include <SDL/SDL_ttf.h>
#include <fstream>
#include <algorithm>

// Game Window Settings
static const float sWindowHeight = 1024.0f;
static const float sWindowWidth = 728.0;
static const float sFovY = 1.22f;
static const float sNearSideDrawDistance = 10.0f;
static const float sFarSideDrawDistance = 10000.0f;
static const float sFrameDelay = 16.0f;
static const float sSmallestDeltaTime = 0.05f;
static const float sDenominator = 1000.0f;

// Sound options
static const int sFrequency = 44100;
static const int sNumChannels = 2;
static const int sChunkSize = 2048;


// ============================================================================
// Basic construction for the game object that uses only an initialization list
// ============================================================================
Game::Game()
	:mPlayer(nullptr)
	,mRenderer(nullptr)
	,mHUD(nullptr)
	,mTicksCount(0)
	,mLastCheckpointTimer(0.0f)
	,mIsRunning(true)
{
}


// ============================================================================
// Basic setup. If we fail at this high of a level, abort.
// ============================================================================
bool Game::Initialize()
{
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0)
	{
		SDL_Log("Unable to initialize SDL: %s", SDL_GetError());
		return false;
	}

	mRenderer = new Renderer(this);
	if (!mRenderer->Initialize(sWindowHeight, sWindowWidth))
	{
		SDL_Log("Unable to initialize Renderer: %s", SDL_GetError());
		return false;
	}

	if (Mix_OpenAudio(sFrequency, MIX_DEFAULT_FORMAT, sNumChannels, sChunkSize))
	{
		SDL_Log("Unable to initialize SDL Audio: %s", SDL_GetError());
		return false;
	}
	
	if (SDL_SetRelativeMouseMode(SDL_TRUE))
	{
		SDL_Log("Unable to set SDL Relative Mouse Mode: %s", SDL_GetError());
		return false;
	}

	if (SDL_GetRelativeMouseState(nullptr, nullptr))
	{
		SDL_Log("Unable to get SDL Relative Mouse State: %s", SDL_GetError());
		return false;
	}

	if (TTF_Init())
	{
		SDL_Log("Unable to initialize the TTF Engine: %s", SDL_GetError());
		return false;
	}

	if (!LoadData())
	{
		SDL_Log("Unable to load the level: %s", SDL_GetError());
		return false;
	}
	mTicksCount = SDL_GetTicks();
	return true;
}


// ============================================================================
// ============================================================================
void Game::RunLoop()
{
	while (mIsRunning)
	{
		// Step 1: Process all received input since the last frame
		ProcessInput();

		// Step 2: Update the internal state of the game, based on the input
		UpdateGame();

		// Step 3: Draw the next frame, fully updated
		GenerateOutput();
		
		if (mNextLevel != "")
		{
			LoadNextLevel();
		}
	}
}


// ============================================================================
// ============================================================================
void Game::ProcessInput()
{
	SDL_Event event;
	while (SDL_PollEvent(&event))
	{
		switch (event.type)
		{
			case SDL_QUIT:
				mIsRunning = false;
				break;
		}
	}
	
	// If the user hits the 'Escape' key, shut down the game
	const Uint8 *state = SDL_GetKeyboardState(NULL);
	if (state[SDL_SCANCODE_ESCAPE])
	{
		mIsRunning = false;
	}

	// For each actor in our game, update their state with the newest input
	for (auto actor : mActors)
	{
		actor->ProcessInput(state);
	}
}


// ============================================================================
// ============================================================================
void Game::UpdateGame()
{
	// Compute delta time
	// Wait until 16ms has elapsed since last frame (hard cap on FPS)
	while (!SDL_TICKS_PASSED(SDL_GetTicks(), mTicksCount + sFrameDelay)) {}

	const float ticks = (SDL_GetTicks() - mTicksCount) / sDenominator;
	const float deltaTime = (ticks > sSmallestDeltaTime) ? sSmallestDeltaTime : ticks;
	mTicksCount = SDL_GetTicks();
	
	// Make copy of actor vector
	// (iterate over this in case any new actors are created)
	std::vector<Actor*> copy = mActors;
	
	// Update all actors
	for (auto actor : copy)
	{
		actor->Update(deltaTime);
	}
	
	// Update the HUD
	mHUD->Update(deltaTime);

	// Add any dead actors to a temp vector
	std::vector<Actor*> deadActors;
	for (auto actor : mActors)
	{
		if (actor->GetState() == Actor::EDead)
		{
			deadActors.emplace_back(actor);
		}
	}

	// Delete any of the dead actors (which will remove them from mActors)
	for (auto actor : deadActors)
	{
		delete actor;
	}
	
	// Mark any checkpoints the player collided with for deletion
	for (auto cp : mDeadCheckpoints)
	{
		cp->SetState(Actor::EDead);
		mDeadCheckpoints.erase(std::find(mDeadCheckpoints.begin(),
										 mDeadCheckpoints.end(), cp));
	}
}


// ============================================================================
// ============================================================================
void Game::GenerateOutput()
{
	mRenderer->Draw();
}


// ============================================================================
// Setup the sounds, cameras, checkpoints, HUD
// ============================================================================
bool Game::LoadData()
{
	// Load sounds
	GetSound("Assets/Sounds/Checkpoint.wav");
	GetSound("Assets/Sounds/Coin.wav");
	GetSound("Assets/Sounds/Jump.wav");
	GetSound("Assets/Sounds/Land.wav");
	GetSound("Assets/Sounds/Music.ogg");
	GetSound("Assets/Sounds/Running.wav");
	
	Matrix4 mat4 =
		Matrix4::CreatePerspectiveFOV(sFovY, sWindowHeight, sWindowWidth, 
									  sNearSideDrawDistance, sFarSideDrawDistance);
	mRenderer->SetProjectionMatrix(mat4);
	
	mat4 = Matrix4::CreateLookAt(Vector3(0,0,0), Vector3::UnitX, Vector3::UnitZ);
	mRenderer->SetViewMatrix(mat4);
	
	// Level file
	if (!LevelLoader::Load(this, "Assets/Tutorial.json"))
	{
		SDL_Log("Unable to load level: %s", SDL_GetError());
		return false;
	}
	
	// Set first checkpoint to blue (0)
	mCheckpoints.front()->GetMesh()->SetTextureIndex(0);
	
	// Arrow pointing towards active checkpoint
	Arrow* arrow = new Arrow(this);
	
	// Start the level music, loop forever
	Mix_PlayChannel(-1, GetSound("Assets/Sounds/Music.ogg"), -1);
	
	// HUD
	mHUD = new HUD(this);
	return true;
}


// ============================================================================
// ============================================================================
void Game::UnloadData()
{
	// Delete actors
	// Because ~Actor calls RemoveActor, have to use a different style loop
	while (!mActors.empty())
	{
		delete mActors.back();
	}
	mActors.clear();

	// Destroy textures
	for (auto i : mTextures)
	{
		SDL_DestroyTexture(i.second);
	}
	mTextures.clear();

	// Destroy sounds
	for (auto s : mSounds)
	{
		Mix_FreeChunk(s.second);
	}
	mSounds.clear();
}


// ============================================================================
// ============================================================================
Mix_Chunk* Game::GetSound(const std::string& fileName)
{
	Mix_Chunk* chunk = nullptr;
	auto it = mSounds.find(fileName);
	if (it != mSounds.end())
	{
		chunk = it->second;
	}
	else
	{
		chunk = Mix_LoadWAV(fileName.c_str());
		if (!chunk)
		{
			SDL_Log("Failed to load sound file %s", fileName.c_str());
			return nullptr;
		}
		mSounds.emplace(fileName, chunk);
	}
	return chunk;
}


// ============================================================================
// ============================================================================
void Game::Shutdown()
{
	UnloadData();
	Mix_CloseAudio();
	mRenderer->Shutdown();
	delete mRenderer;
	SDL_Quit();
}


// ============================================================================
// ============================================================================
void Game::AddActor(Actor* actor)
{
	mActors.emplace_back(actor);
}


// ============================================================================
// ============================================================================
void Game::RemoveActor(Actor* actor)
{
	auto it = std::find(mActors.begin(), mActors.end(), actor);
	if (it != mActors.end())
	{
		// Swap to end of vector and pop off (avoid erasing copies)
		auto it2 = mActors.end() - 1;
		std::iter_swap(it, it2);
		mActors.pop_back();
	}
}


// ============================================================================
// ============================================================================
void Game::RemoveBlock(Block* block)
{
	auto it = std::find(mBlocks.begin(), mBlocks.end(), block);
	if (it != mBlocks.end())
	{
		mBlocks.erase(it);
	}
}


// ============================================================================
// ============================================================================
bool Game::LoadNextLevel()
{
	// Delete all the actors in the current level
	while (!mActors.empty())
	{
		delete mActors.back();
	}
	
	// Clear out the checkpoint queue (just in case)
	while (!mCheckpoints.empty())
	{
		mCheckpoints.pop();
	}
	
	// Call LevelLoader::Load on mNextLevel
	if (!LevelLoader::Load(this, mNextLevel))
	{
		SDL_Log("Unable to load next level: %s", SDL_GetError());
		return false;
	}
	
	// Activate the first checkpoint
	mCheckpoints.front()->GetMesh()->SetTextureIndex(0);
	
	// Allocate a new Arrow actor (since the old one got deleted)
	Arrow* arrow = new Arrow(this);
	
	// mNextLevel.clear() (so we don’t try to change the level again on the next frame)
	mNextLevel.clear();
	return true;
}
