#pragma once
#include "GameObject.h"
#include "define.h"


class CGoomba : public CGameObject
{
	virtual void GetBoundingBox(float &left, float &top, float &right, float &bottom);
	virtual void Update(DWORD dt, vector<LPGAMEOBJECT> *coObjects);
	virtual void Render();

public: 	
	CGoomba(float &model, int &direction);
	virtual void SetState(int state);
};