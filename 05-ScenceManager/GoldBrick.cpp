#include "GoldBrick.h"

GoldBrick::GoldBrick(float x, float y, float model)
{
	this->model = model;
	if (model == QUESTION_BRICK_MODEL_MANY_COIN)
		this->Health = 9;
	startX = x; startY = y;
	minY = y - QUESTION_BRICK_MIN_Y;
	eType = Type::GOLDBRICK;
}

void GoldBrick::Update(DWORD dt, vector<LPGAMEOBJECT>* coObjects)
{
	CGameObject::Update(dt);
	if(isFinish) return;
	vector<LPCOLLISIONEVENT> coEvents;
	vector<LPCOLLISIONEVENT> coEventsResult;

	coEvents.clear();
	y += dy;

	CMario* mario = ((CPlayScene*)CGame::GetInstance()->GetCurrentScene())->GetPlayer();
	if (mario != NULL && mario->level == MARIO_LEVEL_RACOON && mario->isTurningTail) {
		float mLeft, mTop, mRight, mBottom;
		float oLeft, oTop, oRight, oBottom;
		mario->GetBoundingBox(mLeft, mTop, mRight, mBottom);
		GetBoundingBox(oLeft, oTop, oRight, oBottom);
		if (mario->nx > 0) {
			if (CheckAABB(mLeft, mTop + TAIL_SIZE, mRight, mBottom))
				isFinish = true;
		}
		else {
			if (CheckAABB(mLeft-6, mTop + TAIL_SIZE, mRight, mBottom))
				isFinish = true;
		}
	}

	if (y <= minY)
	{
		vy = QUESTION_BRICK_SPEED_UP;
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
		if (state != GOLD_BRICK_STATE_UNBOX) {
			if (model == GOLD_BRICK_MODEL_MANY_COIN) {
				if (Health != 0.f)
					ani = ani = GOLD_BRICK_ANI_BOX;
				else ani = GOLD_BRICK_ANI_UNBOX;
			}
		}
		else ani = GOLD_BRICK_ANI_UNBOX;
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
