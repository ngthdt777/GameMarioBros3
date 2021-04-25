#include <algorithm>
#include <assert.h>
#include "Utils.h"

#include "Mario.h"
#include "Game.h"
#include "ColorBlock.h"
#include "Goomba.h"
#include "Portal.h"
#include "Platform.h"

CMario::CMario(float x, float y) : CGameObject()
{
	level = MARIO_LEVEL_BIG;
	untouchable = 0;
	SetState(MARIO_STATE_IDLE);
	start_x = x; 
	start_y = y; 
	this->x = x; 
	this->y = y; 
}

void CMario::Update(DWORD dt, vector<LPGAMEOBJECT> *coObjects)
{
	// Calculate dx, dy 
	CGameObject::Update(dt);

	// Simple fall down
	vy += MARIO_GRAVITY*dt;

	vector<LPCOLLISIONEVENT> coEvents;
	vector<LPCOLLISIONEVENT> coEventsResult;

	coEvents.clear();

	// turn off collision when die 
	if (state!=MARIO_STATE_DIE)
		CalcPotentialCollisions(coObjects, coEvents);
	// reset untouchable timer if untouchable time has passed
	if ( GetTickCount() - untouchable_start > MARIO_UNTOUCHABLE_TIME) 
	{
		untouchable_start = 0;
		untouchable = 0;
	}

	// No collision occured, proceed normally
	if (coEvents.size()==0)
	{
		x += dx; 
		y += dy;
	}
	else
	{
		float min_tx, min_ty, nx = 0, ny;
		float rdx = 0; 
		float rdy = 0;

		// TODO: This is a very ugly designed function!!!!
		FilterCollision(coEvents, coEventsResult, min_tx, min_ty, nx, ny, rdx, rdy);

		// how to push back Mario if collides with a moving objects, what if Mario is pushed this way into another object?
		//if (rdx != 0 && rdx!=dx)
		//	x += nx*abs(rdx); 
		
		// block every object first!
		x += min_tx*dx + nx*0.4f;
		y += min_ty*dy + ny*0.4f;

		if (nx!=0) vx = 0;
		if (ny!=0) vy = 0;


		//
		// Collision logic with other objects
		//
		for (UINT i = 0; i < coEventsResult.size(); i++)
		{
			LPCOLLISIONEVENT e = coEventsResult[i];

			if (dynamic_cast<CGoomba *>(e->obj)) // if e->obj is Goomba 
			{
				CGoomba *goomba = dynamic_cast<CGoomba *>(e->obj);

				// jump on top >> kill Goomba and deflect a bit 
				if (e->ny < 0)
				{
					if (goomba->GetState()!= GOOMBA_STATE_DIE)
					{
						goomba->SetState(GOOMBA_STATE_DIE);
						vy = -MARIO_JUMP_DEFLECT_SPEED;
					}
				}
				else if (e->nx != 0)
				{
					if (untouchable==0)
					{
						if (goomba->GetState()!=GOOMBA_STATE_DIE)
						{
							if (level > MARIO_LEVEL_SMALL)
							{
								level = MARIO_LEVEL_SMALL;
								StartUntouchable();
							}
							else 
								SetState(MARIO_STATE_DIE);
						}
					}
				}
			} // if Goomba
			else if (dynamic_cast<CPortal *>(e->obj))
			{
				CPortal *p = dynamic_cast<CPortal *>(e->obj);
				CGame::GetInstance()->SwitchScene(p->GetSceneId());
			}
			else if (dynamic_cast<ColorBlock *>(e->obj))
			{
				if (e->ny > 0)
				{
					y -= 27.5;
				}
			}

		}
	}

	// clean up collision events
	for (UINT i = 0; i < coEvents.size(); i++) delete coEvents[i];
}

void CMario::Render()
{
		int ani = -1;
		if (state == MARIO_STATE_DIE)
			ani = MARIO_ANI_DIE;
		else if (level == MARIO_LEVEL_BIG) //MARIO BIG
		{
			if (vx == 0)	//MARIO_STATE_WALKING
			{
				if (nx > 0) ani = MARIO_ANI_BIG_IDLE_RIGHT;
					else ani = MARIO_ANI_BIG_IDLE_LEFT;
			}
			else if (vx >0) 
					ani = MARIO_ANI_BIG_WALK_RIGHT;
				else ani = MARIO_ANI_BIG_WALK_LEFT;

			if (state == MARIO_STATE_TURN)
			{
				if (nx < 0)
					ani = MARIO_ANI_BIG_TURN_LEFT;

				if (nx > 0) 
					ani = MARIO_ANI_BIG_TURN_RIGHT;
			}

			if (state == MARIO_STATE_RUN_RIGHT)
				ani = MARIO_ANI_BIG_RUN_RIGHT;
			if (state == MARIO_STATE_RUN_LEFT)
				ani = MARIO_ANI_BIG_RUN_LEFT;

			if (abs(vx) == MARIO_RUNNING_MAXSPEED)
				if (nx > 0)
					ani = MARIO_ANI_BIG_RUN_RIGHT_MAXSPEED;
				else ani = MARIO_ANI_BIG_RUN_LEFT_MAXSPEED;

			if (state == MARIO_STATE_FLY_RIGHT)
				ani = MARIO_ANI_BIG_FLY_RIGHT;
			if (state == MARIO_STATE_FLY_LEFT)
				ani = MARIO_ANI_BIG_FLY_LEFT;


			if (state == MARIO_STATE_JUMP) {
				if (nx > 0) ani = MARIO_ANI_BIG_JUMP_RIGHT;
				else ani = MARIO_ANI_BIG_JUMP_LEFT;				
			}

			if (state == MARIO_STATE_SIT) {
				if (nx > 0) ani = MARIO_ANI_BIG_SIT_RIGHT;
				else ani = MARIO_ANI_BIG_SIT_LEFT;
			}

		}

		//-----------MARIO SMALL---------------------
		else if (level == MARIO_LEVEL_SMALL)
		{
			if (vx == 0)
			{
				if (nx > 0) ani = MARIO_ANI_SMALL_IDLE_RIGHT;
				else ani = MARIO_ANI_SMALL_IDLE_LEFT;
			}
			else if (vx > 0)
				ani = MARIO_ANI_SMALL_WALK_RIGHT;
			else ani = MARIO_ANI_SMALL_WALK_LEFT;

			if (state == MARIO_STATE_TURN)
			{
				if (nx < 0)
					ani = MARIO_ANI_SMALL_TURN_LEFT;

				if (nx > 0)
					ani = MARIO_ANI_SMALL_TURN_RIGHT;
			}

			if (state == MARIO_STATE_RUN_RIGHT)
				ani = MARIO_ANI_SMALL_RUN_RIGHT;
			if (state == MARIO_STATE_RUN_LEFT)
				ani = MARIO_ANI_SMALL_RUN_LEFT;

			if (state == MARIO_STATE_FLY_RIGHT)
				ani = MARIO_ANI_SMALL_FLY_RIGHT;
			if (state == MARIO_STATE_FLY_LEFT)
				ani = MARIO_ANI_SMALL_FLY_LEFT;


			if (state == MARIO_STATE_JUMP) {
				if (nx > 0) ani = MARIO_ANI_SMALL_JUMP_RIGHT;
				else ani = MARIO_ANI_SMALL_JUMP_LEFT;
			}
		}
		else if (level == MARIO_LEVEL_FIRE) //MARIO BIG
		{
			if (vx == 0)	//MARIO_STATE_WALKING
			{
				if (nx > 0) ani = MARIO_ANI_FIRE_IDLE_RIGHT;
				else ani = MARIO_ANI_FIRE_IDLE_LEFT;
			}
			else if (vx > 0)
				ani = MARIO_ANI_FIRE_WALK_RIGHT;
			else ani = MARIO_ANI_FIRE_WALK_LEFT;

			if (state == MARIO_STATE_TURN)
			{
				if (nx < 0)
					ani = MARIO_ANI_FIRE_TURN_LEFT;

				if (nx > 0)
					ani = MARIO_ANI_FIRE_TURN_RIGHT;
			}

			if (state == MARIO_STATE_RUN_RIGHT)
				ani = MARIO_ANI_FIRE_RUN_RIGHT;
			if (state == MARIO_STATE_RUN_LEFT)
				ani = MARIO_ANI_FIRE_RUN_LEFT;

			if (abs(vx) == MARIO_RUNNING_MAXSPEED)
				if (nx > 0)
					ani = MARIO_ANI_FIRE_RUN_RIGHT_MAXSPEED;
				else ani = MARIO_ANI_FIRE_RUN_LEFT_MAXSPEED;

			if (state == MARIO_STATE_FLY_RIGHT)
				ani = MARIO_ANI_FIRE_FLY_RIGHT;
			if (state == MARIO_STATE_FLY_LEFT)
				ani = MARIO_ANI_FIRE_FLY_LEFT;


			if (state == MARIO_STATE_JUMP) {
				if (nx > 0) ani = MARIO_ANI_FIRE_JUMP_RIGHT;
				else ani = MARIO_ANI_FIRE_JUMP_LEFT;
			}

			if (state == MARIO_STATE_SIT) {
				if (nx > 0) ani = MARIO_ANI_FIRE_SIT_RIGHT;
				else ani = MARIO_ANI_FIRE_SIT_LEFT;
			}

		}

		int alpha = 255;
		if (untouchable) alpha = 128;

		animation_set->at(ani)->Render(x, y, alpha);

		RenderBoundingBox();
}

void CMario::SetState(int state)
{
	CGameObject::SetState(state);
	switch (state)
	{
	case MARIO_STATE_IDLE:
		vx = 0;		
		isSitting = false;
		isOnGround = true;
		break;
	case MARIO_STATE_WALK_RIGHT:
		vx = MARIO_WALKING_SPEED*dt;
		nx = 1;
		break;
	case MARIO_STATE_WALK_LEFT:
		vx = -MARIO_WALKING_SPEED*dt;
		nx = -1;
		break;
	//case MARIO_STATE_RUN_RIGHT:
	//	vx = MARIO_RUNNING_SPEED;
	//	nx = 1;
	//	break;
	//case MARIO_STATE_RUN_LEFT:
	//	vx = -MARIO_RUNNING_SPEED;
	//	nx = -1;
	//	break;
	case MARIO_STATE_JUMP:
		vy = -MARIO_JUMP_SPEED *dt;
		break; 
	//case MARIO_STATE_TURN:
	//	break;
	case MARIO_STATE_DIE:
		vy = -MARIO_DIE_DEFLECT_SPEED;
		break;
	case MARIO_STATE_SIT:
		isSitting = true;
		break;
	case MARIO_STATE_TURN:
		if (vx < 0)	nx = 1;
		else nx = -1;
		break;
	case MARIO_STATE_RUN_RIGHT:
		vx = MARIO_RUNNING_SPEED * dt;
		nx = 1;
		break;
	case MARIO_STATE_RUN_LEFT:
		vx = -MARIO_RUNNING_SPEED * dt;
		nx = -1;
		break;
	case MARIO_STATE_FLY_RIGHT:
		vx = MARIO_RUNNING_MAXSPEED * dt;
		vy = -MARIO_JUMP_SPEED * dt;
		nx = 1;
		break;
	case MARIO_STATE_FLY_LEFT:
		vx = -MARIO_RUNNING_MAXSPEED * dt;
		vy = -MARIO_JUMP_SPEED * dt;
		nx = -1;
		break;
	}
	//switch (state)
	//{
	//case MARIO_STATE_WALKING_RIGHT:
	//	vx = MARIO_WALKING_SPEED;
	//	nx = 1;
	//	break;
	//case MARIO_STATE_WALKING_LEFT:
	//	vx = -MARIO_WALKING_SPEED;
	//	nx = -1;
	//	break;
	//case MARIO_STATE_JUMP:
	//	// TODO: need to check if Mario is *current* on a platform before allowing to jump again
	//	vy = -MARIO_JUMP_SPEED_Y;
	//	break;
	//case MARIO_STATE_IDLE:
	//	vx = 0;
	//	break;
	//case MARIO_STATE_DIE:
	//	vy = -MARIO_DIE_DEFLECT_SPEED;
	//	break;
	//case MARIO_STATE_SIT:
	//	break;
	//}
}
#pragma region set chuyen dong

void CMario::GetBoundingBox(float &left, float &top, float &right, float &bottom)
{
	left = x;
	top = y; 

	if (level==MARIO_LEVEL_BIG || level == MARIO_LEVEL_FIRE)
	{
		if (state == MARIO_STATE_SIT) {
			right = x + MARIO_BIG_BBOX_SIT_WIDTH;
			bottom = y + MARIO_BIG_BBOX_SIT_HEIGHT;
		}
		else {
			right = x + MARIO_BIG_BBOX_WIDTH;
			bottom = y + MARIO_BIG_BBOX_HEIGHT;
		}
	}
	else
	{
		right = x + MARIO_SMALL_BBOX_WIDTH;
		bottom = y + MARIO_SMALL_BBOX_HEIGHT;
	}
}

/*
	Reset Mario status to the beginning state of a scene
*/
void CMario::Reset()
{
	SetState(MARIO_STATE_IDLE);
	SetLevel(MARIO_LEVEL_BIG);
	SetPosition(start_x, start_y);
	SetSpeed(0, 0);
}


