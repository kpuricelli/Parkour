#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <SDL/SDL.h>
#include "Math.h"

class Renderer
{
public:
	Renderer(class Game* game);
	~Renderer();

	bool Initialize(float width, float height);
	void Shutdown();
	void UnloadData();

	void Draw();

	void AddMeshComp(class MeshComponent* mesh);
	void RemoveMeshComp(class MeshComponent* mesh);

	class Texture* GetTexture(const std::string& fileName);
	class Mesh* GetMesh(const std::string& fileName);

	void SetViewMatrix(const Matrix4& view) { mView = view; }
	void SetProjectionMatrix(const Matrix4& proj) { mProjection = proj; }

	float GetScreenWidth() const { return mScreenWidth; }
	float GetScreenHeight() const { return mScreenHeight; }
	
	Vector3 Unproject(const Vector3& screenPoint) const;
	
	class Shader* GetShader() const { return mSpriteShader; }
	
private:
	bool LoadShaders();
	void CreateSpriteVerts();

	// Hash table of textures loaded
	std::unordered_map<std::string, class Texture*> mTextures;
	
	// Hash table of meshes loaded
	std::unordered_map<std::string, class Mesh*> mMeshes;

	// All mesh components drawn
	std::vector<class MeshComponent*> mMeshComps;

	// Game
	class Game* mGame;

	// Sprite shader
	class Shader* mSpriteShader;
	
	// Sprite vertex array
	class VertexArray* mSpriteVerts;

	// Mesh shader
	class Shader* mMeshShader;

	// View/projection for 3D shaders
	Matrix4 mView;
	Matrix4 mProjection;

	// Window
	SDL_Window* mWindow;
	
	// OpenGL context
	SDL_GLContext mContext;

	// Width/height of screen
	float mScreenWidth;
	float mScreenHeight;
};
