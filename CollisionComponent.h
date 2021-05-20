#pragma once
#include "Component.h"
#include "Math.h"

class CollisionComponent : public Component
{
public:
	CollisionComponent(class Actor* owner);
	~CollisionComponent();

	// Set width/height of this box
	void SetSize(float width, float height, float depth)
	{
		mWidth = width;
		mHeight = height;
		mDepth = depth;
	}

	// Returns true if this box intersects with other
	bool Intersect(const CollisionComponent* other);

	// Get min and max points of box
	Vector3 GetMin() const;
	Vector3 GetMax() const;

	// Get width, height, center of box
	const Vector3& GetCenter() const;
	float GetWidth() const { return mWidth; }
	float GetHeight() const { return mHeight; }
	float GetDepth() const { return mDepth; }
	
private:
	float mWidth;
	float mHeight;
	float mDepth;
};

