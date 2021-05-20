#pragma once
#include "Math.h"
#include <string>

class HUD
{
public:
	HUD(class Game* game);
	~HUD();
	
	// UIScreen subclasses can override these
	virtual void Update(float deltaTime);
	virtual void Draw(class Shader* shader);
	
	// Called from coin when the player collects a new coin
	void UpdateCoinCount();
	
	// Called from checkpoint to display that checkpoint's text
	void UpdateCheckpointText(const std::string& text);
	
protected:
	// Helper to draw a texture
	void DrawTexture(class Shader* shader, class Texture* texture,
					 const Vector2& offset = Vector2::Zero,
					 float scale = 1.0f);
	
	class Game* mGame;
	class Font* mFont;
	class Texture* mTimerText;
	class Texture* mCoinText;
	class Texture* mCheckpointText;
	float mTimer;
	int mCoinCount;
};
