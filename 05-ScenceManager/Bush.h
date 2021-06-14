#pragma once
#include "GameObject.h"
#include "define.h"

class Bush : public CGameObject
{
public:
	Bush();
	virtual void Render();
	virtual void GetBoundingBox(float& l, float& t, float& r, float& b);
	~Bush();
};
