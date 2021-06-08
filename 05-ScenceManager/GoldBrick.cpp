#include "GoldBrick.h"

GoldBrick::GoldBrick(float x, float y, float model)
{
	this->model = model;
	//this->Health = 1;
	startX = x; startY = y;
	minY = y - QUESTION_BRICK_MIN_Y;
	eType = Type::GOLDBRICK;
}

void GoldBrick::Update(DWORD dt, vector<LPGAMEOBJECT>* coObjects)
{
	CGameObject::Update(dt);
	y += dy;
	//if (Health == 0)
	//{
	//	if (model == GOLD_BRICK_MODEL_COIN)
	//	{
	//		isFinish = true;
	//	}
	//	else SetState(GOLD_BRICK_STATE_UNBOX);
	//}
	if (y <= minY)
	{
		vy = QUESTION_BRICK_SPEED_UP * dt;
	}
	if (y >= startY)
	{
		y = startY;
		vy = 0;
	}
	
	if (state == GOLD_BRICK_STATE_IDLE_COIN)
	{
		timeTransform += dt;
	}
	if (timeTransform > TIME_FROM_COIN_TO_BRICK)
	{
		this->SetState(GOLD_BRICK_STATE_BOX);
	}
	
}
void GoldBrick::GetBoundingBox(float& l, float& t, float& r, float& b)
{
	l = x;
	t = y;
	r = l + QUESTION_BRICK_BBOX_WIDTH;
	b = t + QUESTION_BRICK_BBOX_HEIGHT;
	if (isFinish)
		l = t = r = b = 0;
}
void GoldBrick::Render()
{
	if (isFinish)
		return;
	int ani = -1;
	if (eType == Type::GOLDBRICK)
	{
		ani = GOLD_BRICK_ANI_BOX;
		if (state == GOLD_BRICK_STATE_UNBOX)
			ani = GOLD_BRICK_ANI_UNBOX;
	}
	else if (eType == Type::COIN)
		ani = GOLD_BRICK_ANI_IDLE_COIN;

	animation_set->at(ani)->Render(x, y);
}
void GoldBrick::SetState(int state)
{
	CGameObject::SetState(state);
	switch (state)
	{
	case GOLD_BRICK_STATE_BOX:
		eType = Type::GOLDBRICK;
		break;
	case GOLD_BRICK_STATE_UNBOX:		
		break;
	case GOLD_BRICK_STATE_IDLE_COIN:		
		eType = Type::COIN;
		break;
	}
}
GoldBrick::~GoldBrick()
{
}