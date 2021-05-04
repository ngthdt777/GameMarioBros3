#include <Windows.h>
#include <stdio.h>
#include <iomanip>
#include <d3d9.h>
#include <d3dx9.h>
#include <iostream>


using namespace std;

//enum Type {
//	MARIO = 0,
//	BRICK = 1,
//	GOOMBA = 2,
//	KOOPAS = 3,
//	PLATFORM = 21,
//
//};


//-----------------------Key Control--------------------------





//-----------------------main
#define WINDOW_CLASS_NAME L"Super Mario Bros 3"
#define MAIN_WINDOW_TITLE L"Super Mario Bros 3"

#define BACKGROUND_COLOR D3DCOLOR_XRGB(255, 255, 200)
#define SCREEN_WIDTH 300  
#define SCREEN_HEIGHT 300

#define MAX_FRAME_RATE 120

//-------------------------------map
#define TILE_WIDTH 16.0f
#define TILE_HEIGHT 16.0f
#define COLOR_BLOCK_GET_THROUGH 100
//----------------------------Game
#define DIRECTINPUT_VERSION 0x0800
#define KEYBOARD_BUFFER_SIZE 1024


#define MAX_GAME_LINE 1024


#define GAME_FILE_SECTION_UNKNOWN -1
#define GAME_FILE_SECTION_SETTINGS 1
#define GAME_FILE_SECTION_SCENES 2
//GameObject
#define ID_TEX_BBOX -100	

//------------------------------PlayScene
#define SCENE_SECTION_UNKNOWN -1
#define SCENE_SECTION_TEXTURES 2
#define SCENE_SECTION_SPRITES 3
#define SCENE_SECTION_ANIMATIONS 4
#define SCENE_SECTION_ANIMATION_SETS	5
#define SCENE_SECTION_OBJECTS	6
#define SCENE_SECTION_DRAWMAP 7
#define OBJECT_TYPE_MARIO	0
#define OBJECT_TYPE_PLATFORM 1
#define OBJECT_TYPE_GOOMBA	2
#define OBJECT_TYPE_KOOPAS	3
#define OBJECT_TYPE_COLOR_BLOCK 4
#define OBJECT_TYPE_FIRE_BALL 5
#define OBJECT_TYPE_BRICK	6
#define OBJECT_TYPE_PORTAL	50


#define MAX_SCENE_LINE 1024
#define SCENE_TEST 1
//-----------------------------Goomba
#define GOOMBA_WALKING_SPEED 0.05f;

#define GOOMBA_BBOX_WIDTH 16
#define GOOMBA_BBOX_HEIGHT 15
#define GOOMBA_BBOX_HEIGHT_DIE 9

#define GOOMBA_STATE_WALKING 100
#define GOOMBA_STATE_DIE 200

#define GOOMBA_ANI_WALKING 0
#define GOOMBA_ANI_DIE 1

//-----------------------------Koopas
#define KOOPAS_DEFEND_TIME			6000
#define KOOPAS_DEFEND_HITBOX		16
#define KOOPAS_WALK_HITBOX_WIDTH	16
#define KOOPAS_WALK_HITBOX_HEIGHT	27


#define KOOPAS_WALKING_SPEED		0.003f
#define KOOPAS_BALL_SPEED			0.02f
#define KOOPAS_BBOX_WIDTH			16
#define KOOPAS_BBOX_HEIGHT			26
#define KOOPAS_BBOX_HEIGHT_DIE		16

#define KOOPAS_STATE_WALKING		100
#define KOOPAS_STATE_DEFEND			200
#define KOOPAS_STATE_BALL			300
#define KOOPAS_STATE_REVIVE			400	//come back up side down
#define KOOPAS_STATE_DIE			500

#define KOOPAS_ANI_WALKING_LEFT		0
#define KOOPAS_ANI_WALKING_RIGHT	1
#define KOOPAS_ANI_DEFEND			2
#define KOOPAS_ANI_BALL				3		
#define KOOPAS_ANI_DIE				4

//--------------------Brick------------------------
#define BRICK_BBOX_WIDTH  16
#define BRICK_BBOX_HEIGHT 16

//----------------------Mario------------------------
#define MARIO_WALKING_SPEED					0.01f 
#define MARIO_RUNNING_SPEED					0.015f
#define MARIO_RUNNING_MAXSPEED				0.03f
#define MARIO_JUMP_SPEED					0.01f
#define MARIO_FLY_SPEED						0.02f
#define MARIO_JUMP_SPEED_PEEK				0.005f
#define MARIO_JUMP_DEFLECT_SPEED			0.2f
#define MARIO_GRAVITY						0.002f
#define MARIO_DIE_DEFLECT_SPEED				0.3f
#define MARIO_SLIDING_SPEED					1.5f

//----------------------Mario------------------------
#define MARIO_STATE_IDLE					0
#define MARIO_STATE_WALK_RIGHT				1000
#define MARIO_STATE_WALK_LEFT				2000
#define MARIO_STATE_RUN_RIGHT				3000
#define MARIO_STATE_RUN_LEFT				4000
#define MARIO_STATE_JUMP					5000
#define MARIO_STATE_FLY_RIGHT				6000
#define MARIO_STATE_FLY_LEFT				7000
#define MARIO_STATE_DIE						8000
#define MARIO_STATE_SIT						9000
#define MARIO_STATE_TURN					10000
#define MARIO_STATE_KICK					11000
#define MARIO_STATE_SHOOT_FIRE				12000

////----------------------Mario SMALL------------------------
//#define MARIO_ANI_SMALL_IDLE_RIGHT			100
//#define MARIO_ANI_SMALL_IDLE_LEFT			101
//#define MARIO_ANI_SMALL_WALK_RIGHT			102
//#define MARIO_ANI_SMALL_WALK_LEFT			103
//#define MARIO_ANI_SMALL_RUN_RIGHT			104
//#define MARIO_ANI_SMALL_RUN_LEFT			105
//#define MARIO_ANI_SMALL_TURN_RIGHT			106
//#define MARIO_ANI_SMALL_TURN_LEFT			107
//#define MARIO_ANI_SMALL_JUMP_RIGHT			108
//#define MARIO_ANI_SMALL_JUMP_LEFT			109
//#define MARIO_ANI_SMALL_FLY_RIGHT			110
//#define MARIO_ANI_SMALL_FLY_LEFT			111
////----------------------Mario BIG------------------------
//
//
//#define MARIO_ANI_BIG_IDLE_RIGHT			200
//#define MARIO_ANI_BIG_IDLE_LEFT				201
//#define MARIO_ANI_BIG_WALK_RIGHT			202
//#define MARIO_ANI_BIG_WALK_LEFT				203
//#define MARIO_ANI_BIG_RUN_RIGHT				204
//#define MARIO_ANI_BIG_RUN_LEFT				205
//#define MARIO_ANI_BIG_JUMP_RIGHT			206
//#define MARIO_ANI_BIG_JUMP_LEFT				207
//#define MARIO_ANI_BIG_SIT_RIGHT				208
//#define MARIO_ANI_BIG_SIT_LEFT				209
//#define MARIO_ANI_BIG_TURN_RIGHT			210
//#define MARIO_ANI_BIG_TURN_LEFT				211
//#define MARIO_ANI_BIG_RUN_RIGHT_MAXSPEED	212
//#define MARIO_ANI_BIG_RUN_LEFT_MAXSPEED		213
//#define MARIO_ANI_BIG_FLY_RIGHT				214
//#define MARIO_ANI_BIG_FLY_LEFT				215
////----------------------Mario FIRE------------------------
//
//#define MARIO_ANI_FIRE_IDLE_RIGHT			300
//#define MARIO_ANI_FIRE_IDLE_LEFT			301
//#define MARIO_ANI_FIRE_WALK_RIGHT			302
//#define MARIO_ANI_FIRE_WALK_LEFT			303
//#define MARIO_ANI_FIRE_RUN_RIGHT			304
//#define MARIO_ANI_FIRE_RUN_LEFT				305
//#define MARIO_ANI_FIRE_JUMP_RIGHT			306
//#define MARIO_ANI_FIRE_JUMP_LEFT			307
//#define MARIO_ANI_FIRE_SIT_RIGHT			308
//#define MARIO_ANI_FIRE_SIT_LEFT				309
//#define MARIO_ANI_FIRE_TURN_RIGHT			310
//#define MARIO_ANI_FIRE_TURN_LEFT			311
//#define MARIO_ANI_FIRE_RUN_RIGHT_MAXSPEED	312
//#define MARIO_ANI_FIRE_RUN_LEFT_MAXSPEED	313
//#define MARIO_ANI_FIRE_FLY_RIGHT			314
//#define MARIO_ANI_FIRE_FLY_LEFT				315
////----------------------Mario RACOON------------------------
//
//#define MARIO_ANI_RACOON_IDLE_RIGHT			400
//#define MARIO_ANI_RACOON_IDLE_LEFT			401
//#define MARIO_ANI_RACOON_WALK_RIGHT			402
//#define MARIO_ANI_RACOON_WALK_LEFT			403
//#define MARIO_ANI_RACOON_RUN_RIGHT			404
//#define MARIO_ANI_RACOON_RUN_LEFT			405
//#define MARIO_ANI_RACOON_JUMP_RIGHT			406
//#define MARIO_ANI_RACOON_JUMP_LEFT			407
//#define MARIO_ANI_RACOON_SIT_RIGHT			408
//#define MARIO_ANI_RACOON_SIT_LEFT			409
//#define MARIO_ANI_RACOON_TURN_RIGHT			410
//#define MARIO_ANI_RACOON_TURN_LEFT			411
//#define MARIO_ANI_RACOON_RUN_RIGHT_MAXSPEED	412
//#define MARIO_ANI_RACOON_RUN_LEFT_MAXSPEED	413
//#define MARIO_ANI_RACOON_FLY_RIGHT			414
//#define MARIO_ANI_RACOON_FLY_LEFT			415
//
//#define MARIO_ANI_DIE						999
//----------------------Mario SMALL------------------------
#define MARIO_ANI_SMALL_IDLE_RIGHT			0
#define MARIO_ANI_SMALL_IDLE_LEFT			1
#define MARIO_ANI_SMALL_WALK_RIGHT			2
#define MARIO_ANI_SMALL_WALK_LEFT			3
#define MARIO_ANI_SMALL_RUN_RIGHT			4
#define MARIO_ANI_SMALL_RUN_LEFT			5
#define MARIO_ANI_SMALL_TURN_RIGHT			6
#define MARIO_ANI_SMALL_TURN_LEFT			7
#define MARIO_ANI_SMALL_JUMP_RIGHT			8
#define MARIO_ANI_SMALL_JUMP_LEFT			9
#define MARIO_ANI_SMALL_FLY_RIGHT			10
#define MARIO_ANI_SMALL_FLY_LEFT			11
#define MARIO_ANI_SMALL_KICK_RIGHT			12
#define MARIO_ANI_SMALL_KICK_LEFT			13
//----------------------Mario BIG------------------------


#define MARIO_ANI_BIG_IDLE_RIGHT			14
#define MARIO_ANI_BIG_IDLE_LEFT				15
#define MARIO_ANI_BIG_WALK_RIGHT			16
#define MARIO_ANI_BIG_WALK_LEFT				17
#define MARIO_ANI_BIG_RUN_RIGHT				18
#define MARIO_ANI_BIG_RUN_LEFT				19
#define MARIO_ANI_BIG_JUMP_RIGHT			20
#define MARIO_ANI_BIG_JUMP_LEFT				21
#define MARIO_ANI_BIG_SIT_RIGHT				22
#define MARIO_ANI_BIG_SIT_LEFT				23
#define MARIO_ANI_BIG_TURN_RIGHT			24
#define MARIO_ANI_BIG_TURN_LEFT				25
#define MARIO_ANI_BIG_RUN_RIGHT_MAXSPEED	26
#define MARIO_ANI_BIG_RUN_LEFT_MAXSPEED		27
#define MARIO_ANI_BIG_FLY_RIGHT				28
#define MARIO_ANI_BIG_FLY_LEFT				29
#define MARIO_ANI_BIG_KICK_RIGHT			30
#define MARIO_ANI_BIG_KICK_LEFT				31
//----------------------Mario FIRE------------------------

#define MARIO_ANI_FIRE_IDLE_RIGHT			32
#define MARIO_ANI_FIRE_IDLE_LEFT			33
#define MARIO_ANI_FIRE_WALK_RIGHT			34
#define MARIO_ANI_FIRE_WALK_LEFT			35
#define MARIO_ANI_FIRE_RUN_RIGHT			36
#define MARIO_ANI_FIRE_RUN_LEFT				37
#define MARIO_ANI_FIRE_JUMP_RIGHT			38
#define MARIO_ANI_FIRE_JUMP_LEFT			39
#define MARIO_ANI_FIRE_SIT_RIGHT			40
#define MARIO_ANI_FIRE_SIT_LEFT				41
#define MARIO_ANI_FIRE_TURN_RIGHT			42
#define MARIO_ANI_FIRE_TURN_LEFT			43
#define MARIO_ANI_FIRE_RUN_RIGHT_MAXSPEED	44
#define MARIO_ANI_FIRE_RUN_LEFT_MAXSPEED	45
#define MARIO_ANI_FIRE_FLY_RIGHT			46
#define MARIO_ANI_FIRE_FLY_LEFT				47
#define MARIO_ANI_FIRE_KICK_RIGHT			48
#define MARIO_ANI_FIRE_KICK_LEFT			49	
#define MARIO_ANI_FIRE_SHOOT_FIRE_RIGHT		50
#define MARIO_ANI_FIRE_SHOOT_FIRE_LEFT		51	
//----------------------Mario RACOON------------------------

#define MARIO_ANI_RACOON_IDLE_RIGHT			52
#define MARIO_ANI_RACOON_IDLE_LEFT			53
#define MARIO_ANI_RACOON_WALK_RIGHT			54
#define MARIO_ANI_RACOON_WALK_LEFT			55
#define MARIO_ANI_RACOON_RUN_RIGHT			56
#define MARIO_ANI_RACOON_RUN_LEFT			57
#define MARIO_ANI_RACOON_JUMP_RIGHT			58
#define MARIO_ANI_RACOON_JUMP_LEFT			59
#define MARIO_ANI_RACOON_SIT_RIGHT			60
#define MARIO_ANI_RACOON_SIT_LEFT			61
#define MARIO_ANI_RACOON_TURN_RIGHT			62
#define MARIO_ANI_RACOON_TURN_LEFT			63
#define MARIO_ANI_RACOON_RUN_RIGHT_MAXSPEED	64
#define MARIO_ANI_RACOON_RUN_LEFT_MAXSPEED	65
#define MARIO_ANI_RACOON_FLY_RIGHT			66
#define MARIO_ANI_RACOON_FLY_LEFT			67
#define MARIO_ANI_RACOON_KICK_RIGHT			68
#define MARIO_ANI_RACOON_KICK_LEFT			69

#define MARIO_ANI_DIE						70




//--------------------------HITBOX---------------------
#define	MARIO_LEVEL_SMALL					51
#define	MARIO_LEVEL_BIG						52
#define MARIO_LEVEL_FIRE					53
#define MARIO_LEVEL_RACOON					54
#define MARIO_BIG_BBOX_WIDTH				15
#define MARIO_BIG_BBOX_HEIGHT				27
#define MARIO_SMALL_BBOX_WIDTH				13
#define MARIO_SMALL_BBOX_HEIGHT				15
#define MARIO_BIG_BBOX_SIT_WIDTH			15
#define MARIO_BIG_BBOX_SIT_HEIGHT			18
#define MARIO_UNTOUCHABLE_TIME				5000
#define MARIO_DELAY_JUMP_TIME				600


//----------------------FIRE BALL-------------------
#define FIRE_SPEED 0.008f
#define FIRE_GRAVITY 0.0006f
#define FIRE_BOUNCE_SPEED_Y  0.15f
#define FIRE_BBOX_WIDTH 8
#define FIRE_BBOX_HEIGHT 8
#define SHOOT_FIRE_RIGHT 0
#define SHOOT_FIRE_LEFT 1
#define LOAD_FIRE_FROM_FILE 5