#pragma once
#include "Actor.h"
class Arrow : public Actor
{
public:
	Arrow(class Game* game);
	void UpdateActor(float deltaTime) override;
	
private:
	
};
