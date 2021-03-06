#pragma once

#include <Windows.h>
#include <d3dx9.h>
#include <vector>
#include "define.h"
#include "Sprites.h"
#include "Animations.h"
#include "Textures.h"
#include "Utils.h"

using namespace std;

	// special texture to draw object bounding box

class CGameObject; 
typedef CGameObject * LPGAMEOBJECT;

struct CCollisionEvent;
typedef CCollisionEvent * LPCOLLISIONEVENT;
struct CCollisionEvent
{
	LPGAMEOBJECT obj;
	float t, nx, ny;
	
	float dx, dy;		// *RELATIVE* movement distance between this object and obj

	CCollisionEvent(float t, float nx, float ny, float dx = 0, float dy = 0, LPGAMEOBJECT obj = NULL) 
	{ 
		this->t = t; 
		this->nx = nx; 
		this->ny = ny;
		this->dx = dx;
		this->dy = dy;
		this->obj = obj; 
	}

	static bool compare(const LPCOLLISIONEVENT &a, LPCOLLISIONEVENT &b)
	{
		return a->t < b->t;
	}
};


class CGameObject
{
protected:
	Type eType;
	ObjectType objType;
public:
	float cgLeft, cgRight, cgUp, cgDown;
	float x; 
	float y;

	float ax, ay; //acceleration

	float dx;	// dx = vx*dt
	float dy;	// dy = vy*dt

	float vx;
	float vy;
	bool isAppear = false;
	float nx;	 
	float width;
	float height;
	int state;
	int level;
	float Health;
	bool isFinish = false;
	int untouchable;
	DWORD timeUntouchable;
	float model;
	DWORD dt; 
	DWORD untouchable_start;
	LPANIMATION_SET animation_set;

public: 
	void SetPosition(float x, float y) { this->x = x, this->y = y; }
	void SetSpeed(float vx, float vy) { this->vx = vx, this->vy = vy; }
	void GetPosition(float &x, float &y) { x = this->x; y = this->y; }
	void GetSpeed(float &vx, float &vy) { vx = this->vx; vy = this->vy; }
	void SetHealth(float Health) { this->Health = Health; }
	void GetHealth(float &Health) {Health = this->Health;}
	void StartUntouchable(int timeUntouchable) {
		this->timeUntouchable = timeUntouchable;
		untouchable = 1; 
		untouchable_start = GetTickCount();
	}
	bool CheckObjectInCamera();
	int GetState() { return this->state; }

	void RenderBoundingBox();

	void SetAnimationSet(LPANIMATION_SET ani_set) 
	{ 
		animation_set = ani_set; 
	}

	LPCOLLISIONEVENT SweptAABBEx(LPGAMEOBJECT coO);
	bool CheckAABB(float mainLeft, float mainTop, float mainRight, float mainBottom);
	void CalcPotentialCollisions(vector<LPGAMEOBJECT> *coObjects, vector<LPCOLLISIONEVENT> &coEvents);
	void FilterCollision(
		vector<LPCOLLISIONEVENT> &coEvents, 
		vector<LPCOLLISIONEVENT> &coEventsResult, 
		float &min_tx, 
		float &min_ty, 
		float &nx, 
		float &ny, 
		float &rdx, 
		float &rdy);

	CGameObject();

	virtual void GetBoundingBox(float &left, float &top, float &right, float &bottom) = 0;
	virtual void Update(DWORD dt, vector<LPGAMEOBJECT> *coObjects = NULL);
	virtual void Render() = 0;
	virtual void SetState(int state) { this->state = state; }

	Type GetType() { return this->eType; };
	ObjectType GetObjectType() { return this->objType; };
	void SetMove(bool cLeft, bool cUp, bool cRight, bool cDown)
	{
		cgLeft = cLeft;
		cgRight = cRight;
		cgUp = cUp;
		cgDown = cDown;
	};
	void GetMove(bool& cLeft, bool& cUp, bool& cRight, bool& cDown)
	{
		cLeft = cgLeft;
		cRight = cgRight;
		cUp = cgUp;
		cDown = cgDown;
	};
	~CGameObject();
};

