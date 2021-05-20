#include "LevelLoader.h"
#include <rapidjson/document.h>
#include "Math.h"
#include <SDL/SDL.h>
#include <fstream>
#include <sstream>
#include "Actor.h"
#include "MeshComponent.h"
#include "Block.h"
#include "Player.h"
#include "Game.h"
#include "Checkpoint.h"
#include "Coin.h"

namespace
{
	// Helper functions to get other types
	bool GetFloatFromJSON(const rapidjson::Value& inObject, const char* inProperty, float& outFloat);
	bool GetIntFromJSON(const rapidjson::Value& inObject, const char* inProperty, int& outInt);
	bool GetStringFromJSON(const rapidjson::Value& inObject, const char* inProperty, std::string& outStr);
	bool GetBoolFromJSON(const rapidjson::Value& inObject, const char* inProperty, bool& outBool);
	bool GetVectorFromJSON(const rapidjson::Value& inObject, const char* inProperty, Vector3& outVector);
	bool GetQuaternionFromJSON(const rapidjson::Value& inObject, const char* inProperty, Quaternion& outQuat);
}

bool LevelLoader::Load(class Game* game, const std::string & fileName)
{
	std::ifstream file(fileName);

	if (!file.is_open())
	{
		SDL_Log("Level file %s not found", fileName.c_str());
		return false;
	}

	std::stringstream fileStream;
	fileStream << file.rdbuf();
	std::string contents = fileStream.str();
	rapidjson::StringStream jsonStr(contents.c_str());
	rapidjson::Document doc;
	doc.ParseStream(jsonStr);

	if (!doc.IsObject())
	{
		SDL_Log("Level file %s is not valid JSON", fileName.c_str());
		return false;
	}

	// Loop through "actors" array
	const rapidjson::Value& actors = doc["actors"];
	if (actors.IsArray())
	{
		for (rapidjson::SizeType i = 0; i < actors.Size(); i++)
		{
			// This index should be an object
			const rapidjson::Value& actorValue = actors[i];
			if (actorValue.IsObject())
			{
				// Lookup actor type
				std::string type = actorValue["type"].GetString();
				Actor* actor = nullptr;

				if (type == "Block")
				{
					Block* block = new Block(game);
					actor = block;
				}
				else if (type == "Player")
				{
					Player* player = new Player(game);
					actor = player;
					Vector3 pos;
					if (GetVectorFromJSON(actorValue, "pos", pos))
					{
						player->SetRespawnPos(pos);
						game->SetPlayer(player);
					}
				}
				else if (type == "Checkpoint")
				{
					Checkpoint* cp = new Checkpoint(game);
					cp->GetMesh()->SetTextureIndex(1);
					actor = cp;
					game->mCheckpoints.push(cp);
					std::string level;
					std::string text;
					if (GetStringFromJSON(actorValue, "level", level))
					{
						cp->SetLevelString(level);
					}
					if (GetStringFromJSON(actorValue, "text", text))
					{
						cp->SetCheckpointString(text);
					}
				}
				else if (type == "Coin")
				{
					Coin* coin = new Coin(game);
					actor = coin;
				}

				// Set properties of actor
				if (actor)
				{
					Vector3 pos;
					if (GetVectorFromJSON(actorValue, "pos", pos))
					{
						actor->SetPosition(pos);
					}

					float scale = 1.0f;
					if (GetFloatFromJSON(actorValue, "scale", scale))
					{
						actor->SetScale(scale);
					}

					float rot = 0.0f;
					if (GetFloatFromJSON(actorValue, "rot", rot))
					{
						actor->SetRotation(rot);
					}

					int textureIdx = 0;
					if (GetIntFromJSON(actorValue, "texture", textureIdx))
					{
						MeshComponent* mesh = actor->GetMesh();
						if (mesh)
						{
							mesh->SetTextureIndex(static_cast<int>(textureIdx));
						}
					}
				}
			}
		}
	}

	return true;
}

namespace
{

	bool GetFloatFromJSON(const rapidjson::Value& inObject, const char* inProperty, float& outFloat)
	{
		auto itr = inObject.FindMember(inProperty);
		if (itr == inObject.MemberEnd())
		{
			return false;
		}

		auto& property = itr->value;
		if (!property.IsDouble())
		{
			return false;
		}

		outFloat = property.GetFloat();
		return true;
	}

	bool GetIntFromJSON(const rapidjson::Value& inObject, const char* inProperty, int& outInt)
	{
		auto itr = inObject.FindMember(inProperty);
		if (itr == inObject.MemberEnd())
		{
			return false;
		}

		auto& property = itr->value;
		if (!property.IsInt())
		{
			return false;
		}

		outInt = property.GetInt();
		return true;
	}

	bool GetStringFromJSON(const rapidjson::Value& inObject, const char* inProperty, std::string& outStr)
	{
		auto itr = inObject.FindMember(inProperty);
		if (itr == inObject.MemberEnd())
		{
			return false;
		}

		auto& property = itr->value;
		if (!property.IsString())
		{
			return false;
		}

		outStr = property.GetString();
		return true;
	}

	bool GetBoolFromJSON(const rapidjson::Value& inObject, const char* inProperty, bool& outBool)
	{
		auto itr = inObject.FindMember(inProperty);
		if (itr == inObject.MemberEnd())
		{
			return false;
		}

		auto& property = itr->value;
		if (!property.IsBool())
		{
			return false;
		}

		outBool = property.GetBool();
		return true;
	}

	bool GetVectorFromJSON(const rapidjson::Value& inObject, const char* inProperty, Vector3& outVector)
	{
		auto itr = inObject.FindMember(inProperty);
		if (itr == inObject.MemberEnd())
		{
			return false;
		}

		auto& property = itr->value;
		if (!property.IsArray() || property.Size() != 3)
		{
			return false;
		}

		for (rapidjson::SizeType i = 0; i < 3; i++)
		{
			if (!property[i].IsDouble())
			{
				return false;
			}
		}

		outVector.x = property[0].GetFloat();
		outVector.y = property[1].GetFloat();
		outVector.z = property[2].GetFloat();

		return true;
	}

	bool GetQuaternionFromJSON(const rapidjson::Value& inObject, const char* inProperty, Quaternion& outQuat)
	{
		auto itr = inObject.FindMember(inProperty);
		if (itr == inObject.MemberEnd())
		{
			return false;
		}

		auto& property = itr->value;

		for (rapidjson::SizeType i = 0; i < 4; i++)
		{
			if (!property[i].IsDouble())
			{
				return false;
			}
		}

		outQuat.x = property[0].GetFloat();
		outQuat.y = property[1].GetFloat();
		outQuat.z = property[2].GetFloat();
		outQuat.w = property[3].GetFloat();

		return true;
	}
}
