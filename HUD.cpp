#include "HUD.h"
#include "Texture.h"
#include "Shader.h"
#include "Game.h"
#include "Renderer.h"
#include "Font.h"
#include <sstream>
#include <iomanip>
#include <math.h>


// ============================================================================
// ============================================================================
HUD::HUD(Game* game)
	:mGame(game)
	,mFont(nullptr)
	,mTimerText(nullptr)
	,mCoinText(nullptr)
	,mCheckpointText(nullptr)
	,mTimer(0.0f)
	,mCoinCount(0)
{
	// Load font
	mFont = new Font();
	mFont->Load("Assets/Inconsolata-Regular.ttf");
	
	// Initalize mTimerText and mCoinText
	mTimerText = mFont->RenderText("00:00.00");
	mCoinText = mFont->RenderText("0/55");
	mCheckpointText = mFont->RenderText(" ");
}


// ============================================================================
// ============================================================================
HUD::~HUD()
{
	// Get rid of font
	if (mFont)
	{
		mFont->Unload();
		delete mFont;
	}
}


// ============================================================================
// ============================================================================
void HUD::Update(float deltaTime)
{
	mTimer += deltaTime;
	
	// Unload and reload the timer text
	mTimerText->Unload();
	delete mTimerText;
	
	// For the new mTimerText
	std::string time;
	std::stringstream ss;
	
	// Split the lhs and rhs of mTimer
	float flhs = 0.0f;
	float frhs = 0.0f;
	frhs = modf(mTimer, &flhs);
	
	// Move the 2 most significant millisecond digits to the rhs of the
	// decimal, then assigning an int to that number will ignore everything
	// after the decimal point
	frhs *= 100.0f;
	const int ms = frhs;
	int sec = flhs;
	
	// Need to save mTimer as an int to use %
	const int timer = mTimer;
	const int min = (timer % 3600) / 60;
	
	// Reset sec to 0 when it gets to 60
	if (sec >= 60)
	{
		sec %= 60;
	}
	
	// If we need to add leading 0-s
	const bool minZeros = (min < 10) ? true : false;
	const bool secZeros = (sec < 10) ? true : false;
	const bool msZeros = (ms < 10) ? true : false;
	
	if (minZeros)
	{
		ss << "0";
	}
	ss << min << ":";
	
	if (secZeros)
	{
		ss << "0";
	}
	ss << sec << ".";
	
	if (msZeros)
	{
		ss << "0";
	}
	ss << ms;
	time = ss.str();
	mTimerText = mFont->RenderText(time);
}


// ============================================================================
// ============================================================================
void HUD::UpdateCoinCount()
{
	// Unload the old text
	mCoinText->Unload();
	delete mCoinText;
	
	// Setup the new coin text
	++mCoinCount;
	std::stringstream ss;
	ss << mCoinCount << "/55";
	std::string text;
	text = ss.str();
	mCoinText = mFont->RenderText(text);
}


// ============================================================================
// ============================================================================
void HUD::UpdateCheckpointText(const std::string& text)
{
	mCheckpointText->Unload();
	delete mCheckpointText;
	mCheckpointText = mFont->RenderText(text);
}


// ============================================================================
// ============================================================================
void HUD::Draw(Shader* shader)
{
	DrawTexture(shader, mTimerText, Vector2(-420.0f, -315.0f));
	DrawTexture(shader, mCoinText, Vector2(-449.0f, -285.0f));
	DrawTexture(shader, mCheckpointText, Vector2::Zero);
}


// ============================================================================
// ============================================================================
void HUD::DrawTexture(class Shader* shader, class Texture* texture,
					  const Vector2& offset, float scale)
{
	// Scale the quad by the width/height of texture
	Matrix4 scaleMat = Matrix4::CreateScale(
		static_cast<float>(texture->GetWidth()) * scale,
		static_cast<float>(texture->GetHeight()) * scale,
		1.0f);
	
	// Translate to position on screen
	Matrix4 transMat =
		Matrix4::CreateTranslation(Vector3(offset.x, offset.y, 0.0f));
	
	// Set world transform
	Matrix4 world = scaleMat * transMat;
	shader->SetMatrixUniform("uWorldTransform", world);
	
	// Set current texture
	texture->SetActive();
	
	// Draw quad
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
}
