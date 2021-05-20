#pragma once
#include "Actor.h"

class Coin : public Actor
{
public:
	Coin(class Game* game);
	void UpdateActor(float deltaTime) override;
	
private:
	float mRotation;
};
