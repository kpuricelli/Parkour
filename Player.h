#pragma once
#include "Actor.h"
class Player : public Actor
{
public:
	Player(class Game* game);
	void SetRespawnPos(const Vector3& pos) { mRespawnPos = pos; }
	Vector3 GetRespawnPos() const { return mRespawnPos; }
	
private:
	Vector3 mRespawnPos;
};
