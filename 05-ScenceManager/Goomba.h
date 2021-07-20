#pragma once
#include "GameObject.h"
#include "define.h"
#include "Poop.h"

//-----------------------------Goomba---------------------------
#define GOOMBA_WALKING_SPEED			0.035f
#define GOOMBA_GRAVITY					0.002f
#define GOOMBA_DIE_DEFLECT_SPEED		0.35f
#define GOOMBA_JUMP_SPEED				0.125f
#define GOOMBA_HIGHJUMP_SPEED			0.25f

#define GOOMBA_RED_TIME_WALKING			800
#define GOOMBA_RED_TIME_HIGHJUMPING		500
#define GOOMBA_RED_TIME_JUMPING			150
#define GOOMBA_RED_JUMPING_STACKS		3

#define GOOMBA_STATE_DIE				100
#define GOOMBA_STATE_WALKING			200
#define GOOMBA_STATE_DIE_BY_TAIL		300
#define GOOMBA_STATE_RED_JUMPING		400
#define GOOMBA_STATE_RED_HIGHJUMPING	450
#define GOOMBA_STATE_RED_WINGSWALKING	500

#define GOOMBA_BBOX_WIDTH						16
#define GOOMBA_WING_BBOX_WIDTH					20
#define GOOMBA_BBOX_HEIGHT						15
#define GOOMBA_WING_BBOX_HEIGHT					24
#define GOOMBA_WING_BBOX_WALK_HEIGHT			19
#define GOOMBA_BBOX_HEIGHT_DIE					9

#define GOOMBA_MODEL_NORMAL						1
#define GOOMBA_MODEL_WING						2
#define GOOMBA_MODEL_WING_BROWN					3



#define GOOMBA_ANI_DIE							0
#define GOOMBA_ANI_WING_DIE						1
#define GOOMBA_ANI_WALKING						2
#define GOOMBA_ANI_WING_WALKING_WITH_OUT_WING	3
#define GOOMBA_ANI_WING_WALKING					4
#define GOOMBA_ANI_WING_JUMPING					5
#define GOOMBA_ANI_WING_FALLING					6
#define GOOMBA_ANI_WING_BROWN_JUMPING			7
#define GOOMBA_ANI_WING_BROWN_FALLING			8

#define GOOMBA_JUMPING_SPEED					0.25f
#define TIME_TO_DIE								300
#define POOP_DELAY_DROP							1000
class CGoomba : public CGameObject
{
	DWORD timeDropDelay;
	DWORD timeToDie;
	DWORD walking_start = 0;
	bool isWalking = false;
	bool isJumping = false;
	bool isHighJumping = false;
	int jumping_stacks = 0;
public: 	
	CMario* mario;
	float model;
	float x0, y0;
	vector<LPGAMEOBJECT> listPoop;
	int Health;
	CGoomba(CMario* mario,float model, float direction);
	virtual void SetState(int state);
	void TurnAround();
	float GetModel() { return model; }
	virtual void GetBoundingBox(float &left, float &top, float &right, float &bottom);
	virtual void Update(DWORD dt, vector<LPGAMEOBJECT> *coObjects);
	virtual void Render();
	void StartWalking() { walking_start = GetTickCount(); isWalking = true; }
};