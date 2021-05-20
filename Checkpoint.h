#pragma once
#include "Actor.h"
#include <string>
class Checkpoint : public Actor
{
public:
	Checkpoint(class Game* game);
	void UpdateActor(float deltaTime) override;
	void SetLevelString(const std::string& level) { mLevelString = level; }
	void SetCheckpointString(const std::string& string) { mCheckpointString = string; }
	
	float mTextTime;

private:
	std::string mLevelString;
	std::string mCheckpointString;
};

