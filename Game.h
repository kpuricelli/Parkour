#pragma once
#include "SDL/SDL.h"
#include "SDL/SDL_mixer.h"
#include <unordered_map>
#include <string>
#include <vector>
#include <queue>
#include "Math.h"

class Game
{
public:
	Game();
	bool Initialize();
	void RunLoop();
	void Shutdown();

	// Actors
	void AddActor(class Actor* actor);
	void RemoveActor(class Actor* actor);

	// Sound
	Mix_Chunk* GetSound(const std::string& fileName);

	// Rendenrer
	class Renderer* GetRenderer() {	return mRenderer; }
	
	// Blocks
	void AddBlock(class Block *block) { mBlocks.emplace_back(block); }
	void RemoveBlock(class Block* block);
	std::vector<class Block*> GetBlocks() const { return mBlocks; }
	
	// Player
	class Player* GetPlayer() const { return mPlayer; }
	void SetPlayer(class Player* player) { mPlayer = player; }
	
	// Queue of checkpoints
	std::queue<class Checkpoint*> mCheckpoints;
	std::vector<class Checkpoint*> mDeadCheckpoints;
	
	// Level
	void SetNextLevel(const std::string& level) { mNextLevel = level; }
	
	// HUD
	class HUD* GetHUD() const { return mHUD; }
	
	// Checkpoints
	float GetLastCheckpointTimer() { return mLastCheckpointTimer; }
	void AddToLastCheckpointTimer(float time) { mLastCheckpointTimer += time; }
	void ResetLastCheckpointTimer() { mLastCheckpointTimer = 0.0f; }
		
private:
	void ProcessInput();
	void UpdateGame();
	void GenerateOutput();
	bool LoadData();
	void UnloadData();
	bool LoadNextLevel();

	// Hash table of textures / sounds
	std::unordered_map<std::string, SDL_Texture*> mTextures;
	std::unordered_map<std::string, Mix_Chunk*> mSounds;

	// All the actors / blocks in the game
	std::vector<class Actor*> mActors;
	std::vector<class Block*> mBlocks;
	
	std::string mNextLevel;
	class Player* mPlayer;
	class Renderer* mRenderer;
	class HUD* mHUD;
	Uint32 mTicksCount;
	float mLastCheckpointTimer;
	bool mIsRunning;
};
