#include "Mario.h"
#include "EffectPoint.h"
#include "EffectDisappear.h"
#include "EffectTailHit.h"
#include "EffectBrokenBrick.h"
#include "PlayScence.h"

CMario::CMario(float x, float y) 
{
	level = MARIO_LEVEL_BIG;
	untouchable = 0;
	SetState(MARIO_STATE_IDLE);
	ax = MARIO_ACCELERATION;
	ay = MARIO_GRAVITY;
	nx = 1;
	start_x = x;
	start_y = y;
	this->x = x;
	this->y = y;
	SetMove(false, false, true, false);
	RunningStacks = 0;
	eType = Type::MARIO;
}

void CMario::TimingFlag()
{

	if (this == NULL) return;
	if (GetTickCount() - slow_start > MARIO_SLOW_TIME && slow_start != 0)
	{
		slow_start = 0;
		isReadyToRun = true;
	}
	if (GetTickCount() - running_start > MARIO_RUNNING_STACK_TIME && isRunning && vx != 0 && isReadyToRun)
	{
		running_start = GetTickCount();
		RunningStacks++;

		if (RunningStacks > 7)
		{
			RunningStacks = 7;
			vx = nx * limitRunSpeed;
		}
	}
	if (GetTickCount() - running_stop > MARIO_SLOW_STACK_TIME && !isRunning)
	{
		running_stop = GetTickCount();
		RunningStacks--;
		if (RunningStacks < 0)
		{
			RunningStacks = 0;
			isRunning = false;
			isFlying = false;
		}
	}
	if (GetTickCount() - kicking_start > MARIO_KICKING_TIME && isKicking)	isKicking = false;
	if (GetTickCount() - shooting_start > MARIO_SHOOTING_TIME && isShooting) isShooting = false;
	if (GetTickCount() - turning_state_start > MARIO_TURNING_STATE_TIME && isTurningTail)
	{
		turning_state_start = GetTickCount();
		turning_state++;
	}
	if (GetTickCount() - turning_start > MARIO_TURNING_TAIL_TIME && isTurningTail)
	{
		isTurningTail = false;
		turning_state = 0;
		turning_state_start = 0;
	}
	if (GetTickCount() - untouchable_start > MARIO_UNTOUCHABLE_TIME && untouchable == 1)
	{
		untouchable_start = 0;
		untouchable = 0;
	}
	if (GetTickCount() - flapping_start > MARIO_FLAPPING_TIME && isFlapping)	isFlapping = false;
	if (GetTickCount() - flapping_start <= MARIO_FLAPPING_TIME && isFlapping)
	{
		if (vy >= MARIO_SLOW_FALLING_SPEED)
			vy = MARIO_SLOW_FALLING_SPEED;
	}
	if (GetTickCount() - tailflying_start > MARIO_TAIL_FLYING_TIME && isTailFlying)
	{
		isTailFlying = false;
		isRunning = false;
		isFlying = false;
	}
	if (GetTickCount() - fly_start > MARIO_FLYING_TIME && fly_start != 0 && isFlying)
	{
		fly_start = 0;
		isTailFlying = false;
		isRunning = false;
		isFlying = false;
		StartSlowDown();
	}
}
void CMario::Update(DWORD dt, vector<LPGAMEOBJECT> *coObjects)
{
	CGame *game = CGame::GetInstance();
	CGameObject::Update(dt);

	if (CGame::GetInstance()->GetScene() != WORLDMAP)
	{ 
		float mw = ((CPlayScene*)game->GetCurrentScene())->GetMap()->GetMapWidth();
		float mh = ((CPlayScene*)game->GetCurrentScene())->GetMap()->GetMapHeight();

		//limit X

		if (x >= mw - MARIO_BIG_BBOX_WIDTH)//Right edge
			x = mw - MARIO_BIG_BBOX_WIDTH;
		else if (x <= 0)//Left edge
			x = 0;

		if (game->GetScene() == MAP1_3_1)
		{
			if (x - 25.f <= game->GetCamX())
			{
				SetState(MARIO_STATE_WALK_RIGHT);
				vx = 0.1f;
				nx = 1;
			}
			if (x >= game->GetCamX() + game->GetScreenWidth() - MARIO_BIG_BBOX_WIDTH)
				x = game->GetCamX() + game->GetScreenWidth() - MARIO_BIG_BBOX_WIDTH;
		}


		if (vx * ax < 0 && abs(vx) > MARIO_WALKING_MAXSPEED
			&& (state == MARIO_STATE_WALK_LEFT || state == MARIO_STATE_WALK_RIGHT))
		{
			vx = -nx * MARIO_WALKING_MAXSPEED;
			if (RunningStacks < 0)
				RunningStacks = 0;
		}
		//update speed
		vx += ax * dt + RunningStacks * ax;
		vy += ay * dt;
		//limit the speed of mario 
		if (abs(vx) >= MARIO_WALKING_MAXSPEED)
		{
			if (!isRunning)
				vx = nx * MARIO_WALKING_MAXSPEED;
			else
				if (abs(vx) >= limitRunSpeed && vx * ax > 0)
					if (RunningStacks < 7)
						vx = nx * MARIO_RUNNING_SPEED;
					else
						vx = nx * limitRunSpeed;
		}
		if (vy > MARIO_JUMP_SPEED_MAX)
		{
			vy = MARIO_JUMP_SPEED_MAX;
			ay = MARIO_GRAVITY;
		}
		if (vy <= -limitJumpVelocity)
		{
			vy = -limitJumpVelocity;
			ay = MARIO_GRAVITY;
			isReadyToJump = false;
		}
		if (isFlying)
		{
			vy = -MARIO_FLY_SPEED;
			ay = -MARIO_GRAVITY;
			isReadyToJump = false;
		}
		//timing
		TimingFlag();
		//
		if (state == MARIO_STATE_SIT && vy < 0)
			vy -= MARIO_ACCELERATION_JUMP * dt;
		if (isJumping && isChangeDirection)
			vx = 0;
		if (vy < 0)
			isOnGround = false;
	}
	else SetState(MARIO_STATE_WORLD_MAP);

	vector<LPCOLLISIONEVENT> coEvents;
	vector<LPCOLLISIONEVENT> coEventsResult;

	coEvents.clear();

	if (state != MARIO_STATE_DIE)
		CalcPotentialCollisions(coObjects, coEvents);

	//-----------------------------FIRE BALL-------------------------------

	if (state == MARIO_STATE_SHOOT_FIRE) {
		if (listFire.size() < 2) {
			FireBall *fire = new FireBall(x + 5, y + 5, FIRE_SPEED, FIRE_GRAVITY);
			fire->nx = nx;
			listFire.push_back(fire);
		}
	}
	for (size_t i = 0; i < listFire.size(); i++)
	{
		listFire[i]->Update(dt, coObjects);
		if (!listFire[i]->CheckObjectInCamera() || listFire.at(i)->isFinish) {
			listFire.erase(listFire.begin() + i);
		}
	}
	for (size_t i = 0; i < listEffect.size(); i++)
	{
		listEffect[i]->Update(dt, coObjects);
		if (!listEffect[i]->CheckObjectInCamera() || listEffect.at(i)->isFinish)
			listEffect.erase(listEffect.begin() + i);
	}
	//------------------------------------------------------------------
	//--------------------Check collision
	// No collision occured, proceed normally
	if (coEvents.size() == 0)
	{
		x += dx;
		y += dy;
		if (y - lastStandingY > 1.0f)
			isOnGround = false;
	}
	else
	{
		float min_tx, min_ty, nx = 0, ny;
		float rdx = 0;
		float rdy = 0;

		float x0 = x, y0 = y;
		FilterCollision(coEvents, coEventsResult, min_tx, min_ty, nx, ny, rdx, rdy);
		x =x0+ min_tx * dx + nx * 0.4f;
		y =y0 + min_ty * dy + ny * 0.4f;

		float oLeft, oTop, oRight, oBottom;
		float mLeft, mTop, mRight, mBottom;
		//
		// Collision logic with other objects
		//
		if (CGame::GetInstance()->GetScene() != INTRO) {
			for (UINT i = 0; i < coEventsResult.size(); i++)
			{
				CPlayScene* scene = (CPlayScene*)CGame::GetInstance()->GetCurrentScene();
				LPCOLLISIONEVENT e = coEventsResult[i];
				if (e->ny != 0) {
					isOnGround = true;
					isJumping = false;
					isChangeDirection = false;
					isReadyToSit = true;
					isFlying = false;
				}

				if (e->obj->GetType() == PLATFORM || e->obj->GetType() == PIPE) {
					GetBoundingBox(mLeft, mTop, mRight, mBottom);
					e->obj->GetBoundingBox(oLeft, oTop, oRight, oBottom);
					if (e->ny < 0)
					{
						vy = 0;
						lastStandingY = y;
						if (e->obj->GetType() == PIPE) {
							y = y0;
						}	
					}
					if (e->ny > 0)
					{
						vy = 0;
						ay = MARIO_GRAVITY;
						isReadyToJump = false;
						if (e->obj->GetType() == PIPE) {
							isReadyToPipe = true;
							StartPipeUp();
						}
					}
					if (e->nx != 0)
					{
						if (ceil(mBottom) != oTop)
						{
							vx = 0;
							if (isRunning)
								StopRunning();
							if (x < game->GetCamX())
							{
								SetState(MARIO_STATE_DIE);
								return;
							}
						}
					}
				}
				if (e->obj->GetType() == GOOMBA) // if e->obj is Goomba 
				{
					GetBoundingBox(mLeft, mTop, mRight, mBottom);
					e->obj->GetBoundingBox(oLeft, oTop, oRight, oBottom);
					CGoomba *goomba = dynamic_cast<CGoomba *>(e->obj);
					if (e->ny < 0)
					{
						if (goomba->GetState() != GOOMBA_STATE_DIE)
						{
							if (goomba->Health == 2) {
								goomba->Health = 1;
								goomba->SetState(GOOMBA_STATE_WALKING);
								ShowEffectPoint(goomba, POINT_EFFECT_MODEL_100);
								PlusScore(100);
							}
							else
							{
								goomba->Health--;
								goomba->SetState(GOOMBA_STATE_DIE);
							}
							this->vy = -MARIO_JUMP_DEFLECT_SPEED;
						}
					}
					else
					{
						if (game->IsKeyDown(DIK_A)) {
							if (isTurningTail && level == MARIO_LEVEL_RACOON) {
								EffectTailHit* effectTailHit = new EffectTailHit(goomba->x, goomba->y);
								listEffect.push_back(effectTailHit);
							}
						}
						if (untouchable == 0)
						{
							if (goomba->GetState() != GOOMBA_STATE_DIE)
								BeingAttacked();
						}
						else {
							x = x0 + dx;
						}
						y = y0;
					}
				}
				else if (e->obj->GetType() == KOOPAS)
				{
					GetBoundingBox(mLeft, mTop, mRight, mBottom);
					e->obj->GetBoundingBox(oLeft, oTop, oRight, oBottom);
					CKoopas *koopa = dynamic_cast<CKoopas *>(e->obj);
					if (e->ny < 0)
					{
						PlusScore(100);
						ShowEffectPoint(koopa, POINT_EFFECT_MODEL_100);
						vy = -MARIO_JUMP_DEFLECT_SPEED;
						if (koopa->model == KOOPAS_MODEL_GREEN_WING) {
							koopa->model = KOOPAS_MODEL_GREEN;
							if (koopa->GetState() == KOOPAS_STATE_FLY)
								koopa->SetState(KOOPAS_STATE_WALKING);
						}
						if (koopa->GetState() == KOOPAS_STATE_DEFEND) {
							koopa->nx = this->nx;
							koopa->SetState(KOOPAS_STATE_BALL);
						}
						else if (koopa->GetState() != KOOPAS_STATE_DIE)
						{
							koopa->SetState(KOOPAS_STATE_DEFEND);
						}
					}
					else {
						if (game->IsKeyDown(DIK_A)) {
							if (isTurningTail && level == MARIO_LEVEL_RACOON) {
								if (koopa->GetState() == KOOPAS_STATE_WALKING)
									koopa->SetState(KOOPAS_STATE_DEFEND);
							}
							else if (koopa->state == KOOPAS_STATE_DEFEND || koopa->state == KOOPAS_STATE_UP_SIDE_DOWN) {
								this->isHolding = true;
								koopa->isBeingHeld = true;
							}
						}
						else if (koopa->GetState() == KOOPAS_STATE_DEFEND)
						{
							this->SetState(MARIO_STATE_KICK);
							this->StartKicking();
							koopa->nx = this->nx;
							koopa->SetState(KOOPAS_STATE_BALL);
						}
						else if (untouchable == 0 && (koopa->GetState() == KOOPAS_STATE_BALL || koopa->GetState() == KOOPAS_STATE_WALKING || koopa->GetState() == KOOPAS_STATE_FLY))
						{
							BeingAttacked();
						}
						else {
							x = x0 + dx;
						}
						y = y0;
					}
				}
				else if (e->obj->GetType() == COLORBLOCK)
				{
					GetBoundingBox(mLeft, mTop, mRight, mBottom);
					e->obj->GetBoundingBox(oLeft, oTop, oRight, oBottom);
					if (e->ny > 0) {
						y += dy;
					}
					if (e->ny < 0)
					{
						vy = 0;
						lastStandingY = y;
					}
				}
				else if (e->obj->GetType() == BRICK)
				{
					CBrick* brick = dynamic_cast<CBrick*>(e->obj);
					if (e->ny > 0) {
						if (brick->isUnbox == 0)
							brick->isUnbox = 1;
						y += dy; x -= dx;
						x -= this->nx * 8;// day mario ra xiu
					}
				}
				else if (e->obj->GetType() == FIRE) { // if e->obj is fireball 
					if (untouchable == 0)
					{
						BeingAttacked();
					}
					else { y = y0; }
				}
				else if (e->obj->GetType() == FIREPLANT) {
					FirePlant *plant = dynamic_cast<FirePlant *>(e->obj);
					if (untouchable == 0)
					{
						if (isTurningTail && level == MARIO_LEVEL_RACOON) {
							plant->isFinish = true;
							EffectTailHit* effectTailHit = new EffectTailHit(plant->x, plant->y);
							listEffect.push_back(effectTailHit);
						}
						else BeingAttacked();
					}
					else
					{
						x = x0 + dx;
						if (e->ny < 0) {
							y = y0 + dy;
						}							
					}
				}
				else if (e->obj->GetType() == QUESTIONBRICK) {
					GetBoundingBox(mLeft, mTop, mRight, mBottom);
					e->obj->GetBoundingBox(oLeft, oTop, oRight, oBottom);
					QuestionBrick* qb = dynamic_cast<QuestionBrick*>(e->obj);
					if (e->nx != 0) {
						if (ceil(mBottom) != oTop)
						{
							vx = 0;
						}
					}
					if (e->ny > 0)
					{
						if (qb->Health == 1)
						{
							qb->vy = -QUESTION_BRICK_SPEED_UP;
							qb->Health = 0;
							qb->isUnbox = true;
						}
						ay = MARIO_GRAVITY;
						isReadyToJump = false;
						this->vy = MARIO_JUMP_SPEED_MIN/5;
						y = y0;
					}
					if (e->ny < 0)
					{
						vy = 0;
						lastStandingY = y;
					}
				}
				else if (e->obj->GetType() == GOLDBRICK) {
					GetBoundingBox(mLeft, mTop, mRight, mBottom);
					e->obj->GetBoundingBox(oLeft, oTop, oRight, oBottom);
					GoldBrick* gb = dynamic_cast<GoldBrick*>(e->obj);
					if (e->nx != 0) {
						if (ceil(mBottom) != oTop)
						{
							vx = 0;
						}
					} 
					int model = (int)gb->model;
					if (e->ny > 0) {
						vy = 0;
						{
							switch (model) {
							case GOLD_BRICK_MODEL_COIN:
								if (level > MARIO_LEVEL_SMALL)
									gb->isFinish = true;
								else {
									gb->vy = -QUESTION_BRICK_SPEED_UP;
								}
								break;
							case GOLD_BRICK_MODEL_PSWITCH:
							case GOLD_BRICK_MODEL_MUSHROOM_1_UP:
								gb->SetState(GOLD_BRICK_STATE_UNBOX);
								break;
							case GOLD_BRICK_MODEL_MANY_COIN:
								if (gb->Health != 0) {
									gb->Health--;
									PlusScore(100);
									PlusCoinCollect(1);
									EffectCoin* effectCoin = new EffectCoin(gb->x, gb->y - 10);
									scene->TurnIntoUnit(effectCoin);
									gb->vy = -QUESTION_BRICK_SPEED_UP;
								}
								break;
							}
							ay = MARIO_GRAVITY;
							isReadyToJump = false;
							this->vy = MARIO_JUMP_SPEED_MAX;
						}
					}						
					if (e->ny < 0)
					{
						vy = 0;
						lastStandingY = y;
					}
				}
				else if (e->obj->GetType() == MUSICBRICK) {
					MusicBrick* mb = dynamic_cast<MusicBrick*>(e->obj);
					GetBoundingBox(mLeft, mTop, mRight, mBottom);
					e->obj->GetBoundingBox(oLeft, oTop, oRight, oBottom);
					if (e->ny < 0) //nhay tu tren xuong
					{
						if (mb->GetModel() == MUSIC_BRICK_MODEL_HIDDEN) {
							if (mb->isHidden && ceil(mBottom) != oTop) { y = y0 + dy; }
							else {
								mb->y += MUSIC_BRICK_POS_BOUND;
								mb->vy = -MUSIC_BRICK_GRAVITY;
								if (mb->y >= mb->maxY) {
									if (game->IsKeyDown(DIK_S)) {
										this->vy = -MARIO_CHANGE_SCENE_SPEED *dt; //nhay qua map phu
									}
									else this->vy = -MARIO_DEFLECT_MUSIC_BRICK;
								}									
								lastStandingY = y;								
							}
						}
						else {
							mb->y += MUSIC_BRICK_POS_BOUND;
							mb->vy = -MUSIC_BRICK_GRAVITY;	
							if (mb->y >= mb->maxY) {
								if (game->IsKeyDown(DIK_S)) {
									this->vy = -MARIO_DEFLECT_MUSIC_BRICK;
								}
								this->vy = -MARIO_DEFLECT_MUSIC_BRICK;
							}	
							lastStandingY = y;
						}
					}
					else if (e->ny > 0) {//nhay tu duoi len
						if (mb->GetModel() == MUSIC_BRICK_MODEL_HIDDEN && mb->isHidden) {
							mb->vy = -QUESTION_BRICK_SPEED_UP;
							vy = 0;
							ay = MARIO_GRAVITY;
							isReadyToJump = false;
							this->vy = MARIO_JUMP_SPEED_MAX;
							mb->isHidden = false;
						}							
						else {
							mb->vy = -QUESTION_BRICK_SPEED_UP;
							vy = 0;
							ay = MARIO_GRAVITY;
							isReadyToJump = false;
							this->vy = MARIO_JUMP_SPEED_MAX;
						}
					}
					if (e->nx != 0)
					{
						if (ceil(mBottom) != oTop)
						{
							vx = 0;
						}
					}
				}
				else if (e->obj->GetType() == COIN) {
					e->obj->isFinish = true;
					PlusCoinCollect(1);
					PlusScore(50);
				}
				else if (e->obj->GetType() == PSWITCH)
				{
					PSwitch* pswitch = dynamic_cast<PSwitch*>(e->obj);
					if (e->ny < 0)
					{
						pswitch->SetState(PSWITCH_STATE_USED);
						pswitch->SetPosition(pswitch->x, pswitch->y + PSWITCH_SMALLER);
					}
				}
				else if (e->obj->GetType() == MUSHROOM_POWER || e->obj->GetType() == MUSHROOM_1_UP)
				{
					Mushroom* mushroom = dynamic_cast<Mushroom*>(e->obj);
					mushroom->isFinish = true;
					ShowEffectPoint(this, POINT_EFFECT_MODEL_1K);
					PlusScore(1000);
					if (e->obj->GetType() == MUSHROOM_POWER) {
						this->y -= 20;
						if (this->level < MARIO_LEVEL_BIG)
							this->SetLevel(MARIO_LEVEL_BIG);
					}
				}
				else if (e->obj->GetType() == LEAF)
				{
					Leaf* leaf = dynamic_cast<Leaf*>(e->obj);
					leaf->isFinish = true; this->y -= 5;
					ShowEffectPoint(this, POINT_EFFECT_MODEL_1K);
					PlusScore(1000);
					this->SetLevel(MARIO_LEVEL_RACOON);
				}
				else if (e->obj->GetType() == PORTAL) {
					CPortal* p = dynamic_cast<CPortal*>(e->obj);
					if (CGame::GetInstance()->GetScene() == WORLDMAP) {
						x += dx; y += dy;
					} 
					else {
						if (((CPlayScene*)game->GetCurrentScene())->isEndScene_1 && p->GetSceneId() == WORLDMAP) {
							CGame::GetInstance()->SwitchScene((int)p->GetSceneId());
						}
						else if (p->GetSceneId() != WORLDMAP)
							CGame::GetInstance()->SwitchScene((int)p->GetSceneId());
					}
				}
				else if (e->obj->GetType() == BOX) {
					Box* box = dynamic_cast<Box*>(e->obj);
					this->x = x0 + dx; this->y = y0 + dy;
					switch (box->GetState())
					{
					case BOX_STATE_FLOWER:
						box->stateUnbox = box->GetState();
						box->isUnbox = true;
						box->SetState(BOX_STATE_FLOWER_UP);
						break;
					case BOX_STATE_MUSHROOM:
						box->stateUnbox = box->GetState();
						box->isUnbox = true;
						box->SetState(BOX_STATE_MUSHROOM_UP);
						break;
					case BOX_STATE_STAR:
						box->stateUnbox = box->GetState();
						box->isUnbox = true;
						box->SetState(BOX_STATE_MUSHROOM_UP);
						break;
					}
					if (listCards.size() > 2) listCards.clear();
					this->listCards.push_back(box->stateUnbox);
					((CPlayScene*)game->GetCurrentScene())->isEndScene_1 = true;
					return;
				}
				else if (e->obj->GetType() == BOOMERANGBROTHER) {
					BoomerangBrother *bb = dynamic_cast<BoomerangBrother *>(e->obj);
					if (e->ny < 0)
					{

						if (bb->GetState() != BOOMERANG_BROTHER_STATE_DIE)
						{
							bb->SetState(BOOMERANG_BROTHER_STATE_DIE);
						}
					}
					else if (nx != 0)
					{
						if (untouchable == 0)
							if (bb->GetState() != BOOMERANG_BROTHER_STATE_DIE)
								BeingAttacked();
					}
				}
				else if (e->obj->GetType() == BOOMERANG) {
					CBoomerang* boomerang = dynamic_cast<CBoomerang*>(e->obj);
					if (boomerang->isAppear == true) {
						if (untouchable == 0)
							BeingAttacked();
						else if (e->ny > 0) y += dy;
						y = y0;
					}
					else {
						x += dx;
						if (e->ny != 0)
							y += dy;
						else y = y0;
					}
				}
				else if (e->obj->GetType() == POOP) {
					Poop *poop = dynamic_cast<Poop *>(e->obj);
					poop->isAttach = true;
					y = y0;
				}
				//-----------------WORLD MAP---------------------
				else if (e->obj->GetType() == SCENE) {
					x = e->obj->x;
					if (level == MARIO_LEVEL_SMALL)
						y = e->obj->y;
					else y = e->obj->y - 7.f;
					this->vx = this->vy = 0;
					Scene* tmp = dynamic_cast<Scene*>(e->obj);
					bool cl, cr, cu, cd;
					tmp->GetMove(cl, cu, cr, cd);
					SetMove(cl, cu, cr, cd);
				}
				else if (e->obj->GetType() == CARD) {
					x = e->obj->x;
					if (level == MARIO_LEVEL_SMALL)
						y = e->obj->y;
					else y = e->obj->y - 7.f;
					this->vx = this->vy = 0;
					Card* tmp = dynamic_cast<Card*>(e->obj);
					bool cl, cr, cu, cd;
					tmp->GetMove(cl, cu, cr, cd);
					SetMove(cl, cu, cr, cd);
				}
				else if (e->obj->GetType() == START) {
					x = e->obj->x;
					if (level == MARIO_LEVEL_SMALL)
						y = e->obj->y;
					else y = e->obj->y - 7.f;
					this->vx = this->vy = 0;
					Start* tmp = dynamic_cast<Start*>(e->obj);
					bool cl, cr, cu, cd;
					tmp->GetMove(cl, cu, cr, cd);
					SetMove(cl, cu, cr, cd);
				}
			}
		}
		else {//Collision Intro
			for (UINT i = 0; i < coEventsResult.size(); i++)
			{
				LPCOLLISIONEVENT e = coEventsResult[i];
				if (e->ny != 0) {
					isOnGround = true;
					isJumping = false;
					isChangeDirection = false;
					isReadyToSit = true;
					isFlying = false;
				}
				if (this != NULL && e->obj != NULL) {
					GetBoundingBox(mLeft, mTop, mRight, mBottom);
					e->obj->GetBoundingBox(oLeft, oTop, oRight, oBottom);
				}
				if (e->obj->GetType() == KOOPAS)
				{
					CKoopas *koopa = dynamic_cast<CKoopas *>(e->obj);
					x += dx;
				}
				else if (e->obj->GetType() == PLATFORM) {
					if (e->ny < 0)
					{
						vy = 0;
						lastStandingY = y;
					}
					if (e->ny > 0)
					{
						vy = 0;
						ay = MARIO_GRAVITY;
						isReadyToJump = false;
					}
					if (e->nx != 0)
						x = x0 + dx;
				}
				else if (e->obj->GetType() == MUSHROOM_POWER || e->obj->GetType() == MUSHROOM_1_UP)
				{
					Mushroom* mushroom = dynamic_cast<Mushroom*>(e->obj);
					mushroom->isFinish = true;
					if (e->obj->GetType() == MUSHROOM_POWER) {
						this->y -= 20;
						if (this->level < MARIO_LEVEL_BIG)
							this->SetLevel(MARIO_LEVEL_BIG);
					}
					//this->x += dx; //this->y += dy;
				}
				else if (e->obj->GetType() == LEAF)
				{
					Leaf* leaf = dynamic_cast<Leaf*>(e->obj);
					leaf->isFinish = true; this->y -= 5;
					//this->isTransformToRacoon = true;
					this->SetLevel(MARIO_LEVEL_RACOON);
				}
			}
		}		
	}

	// clean up collision events
	for (UINT i = 0; i < coEvents.size(); i++) delete coEvents[i];
}

void CMario::BasicRenderLogicsForAllLevel(int& ani,
	int ani_idle_right, int ani_idle_left, int ani_jump_down_right, int ani_jump_down_left,
	int ani_braking_left, int ani_braking_right, int ani_walking_right, int ani_walking_left, int ani_kicking_right, int ani_kicking_left) {

	if (vx == 0) {
		if (isOnGround) {
			if (nx > 0) ani = ani_idle_right;
			else ani = ani_idle_left;
			if(isKicking)
				if (nx > 0) ani = ani_kicking_right;
				else ani = ani_kicking_left;
		}
		else {
			if (nx > 0) ani = ani_jump_down_right;
			else ani = ani_jump_down_left;
		}
		if (isShooting)
		{
			if (nx > 0)
				ani = MARIO_ANI_FIRE_SHOOT_FIRE_RIGHT;
			else if (nx < 0)
				ani = MARIO_ANI_FIRE_SHOOT_FIRE_LEFT;
		}
	}
	else {
		if (vx > 0 && nx < 0) {
			ani = ani_braking_right;
		}
		else if (vx < 0 && nx >0) {
			ani = ani_braking_left;
		}
		else if (vx > 0 && nx > 0) {
			ani = ani_walking_right;
		}
		else if (vx < 0 && nx < 0) {
			ani = ani_walking_left;
		}
		if (isKicking)
		{
			if (vx > 0)	ani = ani_kicking_right;
			else if (vx < 0)	ani = ani_kicking_left;
		}
		if (vx != 0 && vy > 0 && !isOnGround) {
			if (nx > 0) ani = ani_jump_down_right;
			else ani = ani_jump_down_left;
		}
		if (isShooting)
		{
			if (nx > 0)
				ani = MARIO_ANI_FIRE_SHOOT_FIRE_RIGHT;
			else if (nx < 0)
				ani = MARIO_ANI_FIRE_SHOOT_FIRE_LEFT;
		}
	}
}

void CMario::RenderJumping(int& ani, int ani_jump_up_right, int ani_jump_up_left, int ani_jump_down_right, int ani_jump_down_left)
{
	if (nx > 0 && vy <= 0)
		ani = ani_jump_up_right;
	else if (nx < 0 && vy <= 0)
		ani = ani_jump_up_left;
	else if (nx > 0 && vy >= 0)
		ani = ani_jump_down_right;
	else if (nx < 0 && vy >= 0)
		ani = ani_jump_down_left;
}

void CMario::Render()
{
	int ani = -1;
	int alpha = 255;

	if (state == MARIO_STATE_DIE)	ani = MARIO_ANI_DIE;
	else if (level == MARIO_LEVEL_SMALL)
	{
		if (isJumping)
		{
			if (!isHolding)
			{
				if (isFlying)
				{
					RenderJumping(ani, MARIO_ANI_SMALL_FLY_RIGHT, MARIO_ANI_SMALL_FLY_LEFT, MARIO_ANI_SMALL_FLY_RIGHT, MARIO_ANI_SMALL_FLY_LEFT);
				}
				else
					RenderJumping(ani, MARIO_ANI_SMALL_JUMP_RIGHT, MARIO_ANI_SMALL_JUMP_LEFT, MARIO_ANI_SMALL_JUMP_RIGHT, MARIO_ANI_SMALL_JUMP_LEFT);
			}
			else
				RenderJumping(ani, MARIO_ANI_SMALL_HOLD_JUMP_RIGHT, MARIO_ANI_SMALL_HOLD_JUMP_LEFT, MARIO_ANI_SMALL_HOLD_JUMP_RIGHT, MARIO_ANI_SMALL_HOLD_JUMP_LEFT);
		}
		else
		{
			if (!isHolding)
			{
				int ani_go_right = MARIO_ANI_SMALL_WALK_RIGHT;
				int ani_go_left = MARIO_ANI_SMALL_WALK_LEFT;
				if (isRunning)
					if (abs(vx) >= MARIO_RUNNING_MAXSPEED)
					{
						ani_go_right = MARIO_ANI_SMALL_RUN_RIGHT;
						ani_go_left = MARIO_ANI_SMALL_RUN_LEFT;
					}
				if (RunningStacks == 7)
				{
					ani_go_right = MARIO_ANI_SMALL_RUN_RIGHT_MAXSPEED;
					ani_go_left = MARIO_ANI_SMALL_RUN_LEFT_MAXSPEED;
				}
				BasicRenderLogicsForAllLevel(ani,
					MARIO_ANI_SMALL_IDLE_RIGHT, MARIO_ANI_SMALL_IDLE_LEFT,
					MARIO_ANI_SMALL_JUMP_RIGHT, MARIO_ANI_SMALL_JUMP_LEFT,
					MARIO_ANI_SMALL_TURN_RIGHT, MARIO_ANI_SMALL_TURN_LEFT,
					ani_go_right, ani_go_left, MARIO_ANI_SMALL_KICK_RIGHT, MARIO_ANI_SMALL_KICK_LEFT);
			}
			else
				BasicRenderLogicsForAllLevel(ani,
					MARIO_ANI_SMALL_HOLD_IDLE_RIGHT, MARIO_ANI_SMALL_HOLD_IDLE_LEFT,
					MARIO_ANI_SMALL_JUMP_RIGHT, MARIO_ANI_SMALL_JUMP_LEFT,
					MARIO_ANI_SMALL_TURN_RIGHT, MARIO_ANI_SMALL_TURN_LEFT,
					MARIO_ANI_SMALL_HOLD_WALK_RIGHT, MARIO_ANI_SMALL_HOLD_WALK_LEFT, MARIO_ANI_SMALL_KICK_RIGHT, MARIO_ANI_SMALL_KICK_LEFT);
		}
		if (state == MARIO_STATE_WORLD_MAP)
			ani = MARIO_ANI_SMALL_WORLD_MAP;

		for (size_t i = 0; i < listFire.size(); i++)
			listFire[i]->Render();
		for (size_t i = 0; i < listEffect.size(); i++)
			listEffect[i]->Render();
	}
	else if (level == MARIO_LEVEL_BIG)
	{
		if (state == MARIO_STATE_SIT)
		{
			if (nx > 0)	ani = MARIO_ANI_BIG_SIT_RIGHT;
			else	ani = MARIO_ANI_BIG_SIT_LEFT;
		}
		else if (isJumping)
		{
			if (!isHolding)
			{
				if (isFlying)
				{
					RenderJumping(ani,MARIO_ANI_BIG_FLY_RIGHT,MARIO_ANI_BIG_FLY_LEFT,MARIO_ANI_BIG_FLY_RIGHT,MARIO_ANI_BIG_FLY_LEFT);
				}
				else
					RenderJumping(ani,MARIO_ANI_BIG_JUMP_RIGHT,MARIO_ANI_BIG_JUMP_LEFT,MARIO_ANI_BIG_JUMP_RIGHT_DOWN,MARIO_ANI_BIG_JUMP_LEFT_DOWN);
			}
			else
				RenderJumping(ani,MARIO_ANI_BIG_HOLD_JUMP_RIGHT,MARIO_ANI_BIG_HOLD_JUMP_LEFT,MARIO_ANI_BIG_HOLD_JUMP_RIGHT,MARIO_ANI_BIG_HOLD_JUMP_LEFT);
		}
		else
		{
			if (!isHolding)
			{
				int ani_go_right = MARIO_ANI_BIG_WALK_RIGHT;
				int ani_go_left = MARIO_ANI_BIG_WALK_LEFT;
				if (isRunning)
					if (abs(vx) >= MARIO_RUNNING_SPEED)
					{
						ani_go_right = MARIO_ANI_BIG_RUN_RIGHT;
						ani_go_left = MARIO_ANI_BIG_RUN_LEFT;
					}
				if (abs(vx) >= MARIO_RUNNING_MAXSPEED)
				{
					ani_go_right = MARIO_ANI_BIG_RUN_RIGHT_MAXSPEED;
					ani_go_left = MARIO_ANI_BIG_RUN_LEFT_MAXSPEED;
				}
				BasicRenderLogicsForAllLevel(ani,
					MARIO_ANI_BIG_IDLE_RIGHT, MARIO_ANI_BIG_IDLE_LEFT,
					MARIO_ANI_BIG_JUMP_RIGHT_DOWN, MARIO_ANI_BIG_JUMP_LEFT_DOWN,			
					MARIO_ANI_BIG_TURN_RIGHT, MARIO_ANI_BIG_TURN_LEFT,
					ani_go_right, ani_go_left, MARIO_ANI_BIG_KICK_RIGHT, MARIO_ANI_BIG_KICK_LEFT);
			}
			else
				BasicRenderLogicsForAllLevel(ani,
					MARIO_ANI_BIG_HOLD_IDLE_RIGHT, MARIO_ANI_BIG_HOLD_IDLE_LEFT,
					MARIO_ANI_BIG_HOLD_JUMP_RIGHT, MARIO_ANI_BIG_HOLD_JUMP_LEFT,
					MARIO_ANI_BIG_TURN_RIGHT, MARIO_ANI_BIG_TURN_LEFT,
					MARIO_ANI_BIG_HOLD_WALK_RIGHT, MARIO_ANI_BIG_HOLD_WALK_LEFT, MARIO_ANI_BIG_KICK_RIGHT, MARIO_ANI_BIG_KICK_LEFT);
		}
		if (state == MARIO_STATE_WORLD_MAP)
			ani = MARIO_ANI_BIG_WORLD_MAP;		
	}
	else if (level == MARIO_LEVEL_RACOON)
	{
		if (state == MARIO_STATE_SIT)
		{
			if (nx > 0)
				ani = MARIO_ANI_RACOON_SIT_RIGHT;
			else
				ani = MARIO_ANI_RACOON_SIT_LEFT;
		}
		else if (isJumping)
		{
			if (!isHolding)
			{
				if (isFlying)
				{
					RenderJumping(ani,MARIO_ANI_RACOON_FLY_RIGHT,MARIO_ANI_RACOON_FLY_LEFT,MARIO_ANI_RACOON_FLY_RIGHT,MARIO_ANI_RACOON_FLY_LEFT);
				}
				else
					RenderJumping(ani,MARIO_ANI_RACOON_JUMP_RIGHT,MARIO_ANI_RACOON_JUMP_LEFT,MARIO_ANI_RACOON_JUMP_RIGHT_DOWN,MARIO_ANI_RACOON_JUMP_LEFT_DOWN);
			}
			else
				RenderJumping(ani,MARIO_ANI_RACOON_HOLD_JUMP_RIGHT,MARIO_ANI_RACOON_HOLD_JUMP_LEFT,MARIO_ANI_RACOON_HOLD_JUMP_RIGHT,MARIO_ANI_RACOON_HOLD_JUMP_LEFT);
			if (!isFlying)
			{
				if (nx > 0)
					ani = MARIO_ANI_RACOON_FLOAT_RIGHT;
				if (nx < 0)
					ani = MARIO_ANI_RACOON_FLOAT_LEFT;
			}
			else
			{
				if (nx > 0)
					ani = MARIO_ANI_RACOON_FLY_RIGHT;
				if (nx < 0)
					ani = MARIO_ANI_RACOON_FLY_LEFT;
			}
		}
		else
		{
			if (!isHolding)
			{
				int ani_go_right = MARIO_ANI_RACOON_WALK_RIGHT;
				int ani_go_left = MARIO_ANI_RACOON_WALK_LEFT;
				if (isRunning)
					if (abs(vx) >= MARIO_RUNNING_SPEED)
					{
						ani_go_right = MARIO_ANI_RACOON_RUN_RIGHT;
						ani_go_left = MARIO_ANI_RACOON_RUN_LEFT;
					}
				if (abs(vx) >= MARIO_RUNNING_MAXSPEED)
				{
					ani_go_right = MARIO_ANI_RACOON_RUN_RIGHT_MAXSPEED;
					ani_go_left = MARIO_ANI_RACOON_RUN_LEFT_MAXSPEED;
				}
				BasicRenderLogicsForAllLevel(ani,
					MARIO_ANI_RACOON_IDLE_RIGHT, MARIO_ANI_RACOON_IDLE_LEFT,
					MARIO_ANI_RACOON_JUMP_RIGHT_DOWN, MARIO_ANI_RACOON_JUMP_LEFT_DOWN,
					MARIO_ANI_RACOON_TURN_RIGHT, MARIO_ANI_RACOON_TURN_LEFT,
					ani_go_right, ani_go_left, MARIO_ANI_RACOON_KICK_RIGHT, MARIO_ANI_RACOON_KICK_LEFT);
			}
			else
				BasicRenderLogicsForAllLevel(ani,
					MARIO_ANI_RACOON_HOLD_IDLE_RIGHT, MARIO_ANI_RACOON_HOLD_IDLE_LEFT,
					MARIO_ANI_RACOON_HOLD_JUMP_RIGHT, MARIO_ANI_RACOON_HOLD_JUMP_LEFT,
					MARIO_ANI_RACOON_TURN_RIGHT, MARIO_ANI_RACOON_TURN_LEFT,
					MARIO_ANI_RACOON_HOLD_WALK_RIGHT, MARIO_ANI_RACOON_HOLD_WALK_LEFT, MARIO_ANI_RACOON_KICK_RIGHT, MARIO_ANI_RACOON_KICK_LEFT);
		}
		if (isTurningTail)
		{
			if (nx > 0)
				ani = MARIO_ANI_RACOON_SPIN_RIGHT;
			else if (nx < 0)
				ani = MARIO_ANI_RACOON_SPIN_LEFT;
		}
		if (state == MARIO_STATE_WORLD_MAP)
			ani = MARIO_ANI_RACOON_WORLD_MAP;
	}
	else if (level == MARIO_LEVEL_FIRE)
	{
		if (state == MARIO_STATE_SIT)
		{
			if (nx > 0)
				ani = MARIO_ANI_FIRE_SIT_RIGHT;
			else
				ani = MARIO_ANI_FIRE_SIT_LEFT;
		}
		else if (isJumping)
		{
			if (!isHolding)
			{
				if (isFlying)
				{
					RenderJumping(ani,MARIO_ANI_FIRE_FLY_RIGHT,MARIO_ANI_FIRE_FLY_LEFT,MARIO_ANI_FIRE_FLY_RIGHT,MARIO_ANI_FIRE_FLY_LEFT);
				}
				else
					RenderJumping(ani,MARIO_ANI_FIRE_JUMP_RIGHT,MARIO_ANI_FIRE_JUMP_LEFT,MARIO_ANI_FIRE_JUMP_RIGHT_DOWN,MARIO_ANI_FIRE_JUMP_LEFT_DOWN);
			}
			else
				RenderJumping(ani,MARIO_ANI_FIRE_HOLD_JUMP_RIGHT,MARIO_ANI_FIRE_HOLD_JUMP_LEFT,MARIO_ANI_FIRE_HOLD_JUMP_RIGHT,MARIO_ANI_FIRE_HOLD_JUMP_LEFT);
		}
		else
		{
			if (!isHolding)
			{
				int ani_go_right = MARIO_ANI_FIRE_WALK_RIGHT;
				int ani_go_left = MARIO_ANI_FIRE_WALK_LEFT;
				if (isRunning)
					if (abs(vx) >= MARIO_RUNNING_SPEED)
					{
						ani_go_right = MARIO_ANI_FIRE_RUN_RIGHT;
						ani_go_left = MARIO_ANI_FIRE_RUN_LEFT;
					}
				if (abs(vx) >= MARIO_RUNNING_MAXSPEED)
				{
					ani_go_right = MARIO_ANI_FIRE_RUN_RIGHT_MAXSPEED;
					ani_go_left = MARIO_ANI_FIRE_RUN_LEFT_MAXSPEED;
				}
				BasicRenderLogicsForAllLevel(ani,
					MARIO_ANI_FIRE_IDLE_RIGHT, MARIO_ANI_FIRE_IDLE_LEFT,
					MARIO_ANI_FIRE_JUMP_RIGHT_DOWN, MARIO_ANI_FIRE_JUMP_LEFT_DOWN,
					MARIO_ANI_FIRE_TURN_RIGHT, MARIO_ANI_FIRE_TURN_LEFT,
					ani_go_right, ani_go_left, MARIO_ANI_FIRE_KICK_RIGHT, MARIO_ANI_FIRE_KICK_LEFT);
			}
			else
				BasicRenderLogicsForAllLevel(ani,
					MARIO_ANI_FIRE_HOLD_IDLE_RIGHT, MARIO_ANI_FIRE_HOLD_IDLE_LEFT,
					MARIO_ANI_FIRE_HOLD_JUMP_RIGHT, MARIO_ANI_FIRE_HOLD_JUMP_LEFT,
					MARIO_ANI_FIRE_TURN_RIGHT, MARIO_ANI_FIRE_TURN_LEFT,
					MARIO_ANI_FIRE_HOLD_WALK_RIGHT, MARIO_ANI_FIRE_HOLD_WALK_LEFT, MARIO_ANI_FIRE_KICK_RIGHT, MARIO_ANI_FIRE_KICK_LEFT);
		}
		if (state == MARIO_STATE_WORLD_MAP)
			ani = MARIO_ANI_FIRE_WORLD_MAP;
	}

	if (untouchable == 1) alpha = 120;
	animation_set->at(ani)->Render(x, y, alpha);
	for (size_t i = 0; i < listFire.size(); i++)
	{
		listFire[i]->Render();
	}
	for (size_t i = 0; i < listEffect.size(); i++)
	{
		listEffect[i]->Render();
	}
	//RenderBoundingBox();
}
void CMario::SetState(int state)
{
	switch (state)
	{
	case MARIO_STATE_IDLE:
		if (vx > 0)
			ax = -MARIO_ACCELERATION;
		if (vx < 0)
			ax = MARIO_ACCELERATION;
		ay = MARIO_GRAVITY;
		if (abs(vx) <= MARIO_WALKING_SPEED)
		{
			vx = 0;
			ax = 0;
		}
		if (isRunning)
			StopRunning();
		break;
	case MARIO_STATE_WALK_RIGHT:
		if (ax < 0 && vy > 0)
		{
			isChangeDirection = true;
		}
		if (ax < 0 && RunningStacks != 0)
		{
			StopRunning();
			StartSlowDown();
		}
		ax = MARIO_ACCELERATION;
		nx = 1;
		break;
	case MARIO_STATE_WALK_LEFT:
		if (ax > 0 && vy > 0)
			isChangeDirection = true;
		if (ax > 0 && RunningStacks != 0)
		{
			StopRunning();
			StartSlowDown();
		}
		ax = -MARIO_ACCELERATION;
		nx = -1;
		break;
	case MARIO_STATE_JUMP:
		if (vy > -MARIO_JUMP_SPEED_MIN)
			vy = -MARIO_JUMP_SPEED_MIN;
		if (RunningStacks == 7 && vx == MARIO_RUNNING_MAXSPEED)
		{
			isFlying = true;
			StartFlying();
		}
		ay = -MARIO_ACCELERATION_JUMP;

		break;
	case MARIO_STATE_DIE:
		vy = -MARIO_DIE_DEFLECT_SPEED;
		break;
	case MARIO_STATE_SIT:
		if (level != MARIO_LEVEL_SMALL)
		{
			if (abs(vx) <= MARIO_WALKING_SPEED) {
				vx = 0;
			}
			ay = MARIO_GRAVITY;
			ax = -nx * MARIO_ACCELERATION;
			isSitting = true;
		}
		break;
	case MARIO_STATE_TURN:
	case MARIO_STATE_RUN_RIGHT:
	case MARIO_STATE_RUN_LEFT:
	case MARIO_STATE_FLY_RIGHT: 
	case MARIO_STATE_FLY_LEFT:
	case MARIO_STATE_KICK:
	case MARIO_STATE_SHOOT_FIRE:
	case MARIO_STATE_SPIN:
	case MARIO_STATE_HOLD:
	case MARIO_STATE_RUN_MAXSPEED:
	case MARIO_STATE_WORLD_MAP:
		break;
	}
	if (state == MARIO_STATE_SIT && this->state != MARIO_STATE_SIT)
		y += MARIO_SIT_PUSH_BACK;
	if (state != MARIO_STATE_SIT && this->state == MARIO_STATE_SIT)
		y -= MARIO_SIT_PUSH_BACK;
	CGameObject::SetState(state);
}

void CMario::GetBoundingBox(float &left, float &top, float &right, float &bottom)
{
	left = x;
	top = y; 

	if (level==MARIO_LEVEL_BIG || level == MARIO_LEVEL_FIRE || level == MARIO_LEVEL_RACOON)
	{
		if (state == MARIO_STATE_SIT) {
			right = x + MARIO_BIG_BBOX_SIT_WIDTH;
			bottom = y + MARIO_BIG_BBOX_SIT_HEIGHT;
		}
		else if (isTurningTail){
			if (nx > 0) right = x + MARIO_RACOON_BBOX_SPIN_WIDTH + 3;
			else {
				left = x - 3;
				right = x + MARIO_RACOON_BBOX_SPIN_WIDTH;
			}
			bottom = y + MARIO_BIG_BBOX_HEIGHT;
		}
		else{
			right = x + MARIO_BIG_BBOX_WIDTH;
			bottom = y + MARIO_BIG_BBOX_HEIGHT;
		}
	}
	else
	{
		right = x + MARIO_SMALL_BBOX_WIDTH;
		bottom = y + MARIO_SMALL_BBOX_HEIGHT;
	}
	if (state == MARIO_STATE_WORLD_MAP) {
		right = x + MARIO_SMALL_BBOX_WIDTH;
		bottom = y + MARIO_SMALL_BBOX_HEIGHT;
	}
}

void CMario::Reset()
{
	SetState(MARIO_STATE_IDLE);
	SetLevel(MARIO_LEVEL_BIG);
	SetPosition(start_x, start_y);
	SetSpeed(0, 0);
}

void CMario::ShowEffectPoint(CGameObject* obj, float model) {
	EffectPoint* effectPoint = new EffectPoint(obj->x, obj->y, model);
	CPlayScene* scene = (CPlayScene*)CGame::GetInstance()->GetCurrentScene();
	listEffect.push_back(effectPoint);
}
void CMario::BeingAttacked() {
	if (level > MARIO_LEVEL_SMALL)
	{
		if (level > MARIO_LEVEL_BIG) {
			level = MARIO_LEVEL_BIG;
			EffectDisappear* effectDisappear = new EffectDisappear(this->x, this->y);
			listEffect.push_back(effectDisappear);
			StartUntouchable();
		}
		else
		{
			level = MARIO_LEVEL_SMALL;
			StartUntouchable();
		}
	}
	else
		SetState(MARIO_STATE_DIE);
}