#include <iostream>
#include <fstream>
#include "ColorBlock.h"
#include "PlayScence.h"
#include "Utils.h"
#include "Textures.h"
#include "Sprites.h"
#include "Portal.h"
#include "Map.h"
#include "Platform.h"

using namespace std;

CPlayScene::CPlayScene(int id, LPCWSTR filePath):
	CScene(id, filePath)
{
	key_handler = new CPlayScenceKeyHandler(this);
}

/*
	Load scene resources from scene file (textures, sprites, animations and objects)
	See scene1.txt, scene2.txt for detail format specification
*/

void CPlayScene::_ParseSection_TEXTURES(string line)
{
	vector<string> tokens = split(line);

	if (tokens.size() < 5) return; // skip invalid lines

	int texID = atoi(tokens[0].c_str());
	wstring path = ToWSTR(tokens[1]);
	int R = atoi(tokens[2].c_str());
	int G = atoi(tokens[3].c_str());
	int B = atoi(tokens[4].c_str());

	CTextures::GetInstance()->Add(texID, path.c_str(), D3DCOLOR_XRGB(R, G, B));
}

void CPlayScene::_ParseSection_SPRITES(string line)
{
	vector<string> tokens = split(line);

	if (tokens.size() < 6) return; // skip invalid lines

	int ID = atoi(tokens[0].c_str());
	int l = atoi(tokens[1].c_str());
	int t = atoi(tokens[2].c_str());
	int r = atoi(tokens[3].c_str());
	int b = atoi(tokens[4].c_str());
	int texID = atoi(tokens[5].c_str());

	LPDIRECT3DTEXTURE9 tex = CTextures::GetInstance()->Get(texID);
	if (tex == NULL)
	{
		DebugOut(L"[ERROR] Texture ID %d not found!\n", texID);
		return; 
	}

	CSprites::GetInstance()->Add(ID, l, t, r, b, tex);
}

void CPlayScene::_ParseSection_ANIMATIONS(string line)
{
	vector<string> tokens = split(line);

	if (tokens.size() < 3) return; // skip invalid lines - an animation must at least has 1 frame and 1 frame time

	//DebugOut(L"--> %s\n",ToWSTR(line).c_str());

	LPANIMATION ani = new CAnimation();

	int ani_id = atoi(tokens[0].c_str());
	for (int i = 1; i < tokens.size(); i += 2)	// why i+=2 ?  sprite_id | frame_time  
	{
		int sprite_id = atoi(tokens[i].c_str());
		int frame_time = atoi(tokens[i+1].c_str());
		ani->Add(sprite_id, frame_time);
	}

	CAnimations::GetInstance()->Add(ani_id, ani);
}

void CPlayScene::_ParseSection_ANIMATION_SETS(string line)
{
	vector<string> tokens = split(line);

	if (tokens.size() < 2) return; // skip invalid lines - an animation set must at least id and one animation id

	int ani_set_id = atoi(tokens[0].c_str());

	LPANIMATION_SET s = new CAnimationSet();

	CAnimations *animations = CAnimations::GetInstance();

	for (int i = 1; i < tokens.size(); i++)
	{
		int ani_id = atoi(tokens[i].c_str());
		
		LPANIMATION ani = animations->Get(ani_id);
		s->push_back(ani);
	}

	CAnimationSets::GetInstance()->Add(ani_set_id, s);
}

/*
	Parse a line in section [OBJECTS] 
*/
void CPlayScene::_ParseSection_OBJECTS(string line)
{
	vector<string> tokens = split(line);

	//DebugOut(L"--> %s\n",ToWSTR(line).c_str());

	if (tokens.size() < 3) return; // skip invalid lines - an object set must have at least id, x, y

	int object_type = atoi(tokens[0].c_str());
	float x = atof(tokens[1].c_str());
	float y = atof(tokens[2].c_str());
	int ani_set_id = atoi(tokens[3].c_str());

	CAnimationSets * animation_sets = CAnimationSets::GetInstance();

	CGameObject *obj = NULL;

	switch (object_type)
	{
	case OBJECT_TYPE_MARIO:
		if (player!=NULL) 
		{
			DebugOut(L"[ERROR] MARIO object was created before!\n");
			return;
		}
		obj = new CMario(x,y); 
		player = (CMario*)obj;  

		DebugOut(L"[INFO] Player object created!\n");
		break;
	case OBJECT_TYPE_GOOMBA: {
		float model = atof(tokens[4].c_str());
		int direction = atof(tokens[5].c_str());
		obj = new CGoomba(model,direction); break;
	} 
	case OBJECT_TYPE_BRICK: obj = new CBrick(); break;
	case OBJECT_TYPE_KOOPAS: {
		float model = atof(tokens[4].c_str());
		int direction = atof(tokens[5].c_str());
		obj = new CKoopas(model, direction); break;
	} 
	case OBJECT_TYPE_PLATFORM: {
		float w = atof(tokens[4].c_str());
		float h = atof(tokens[5].c_str());
		obj = new Platform(w, h);
		break;
	}
	case OBJECT_TYPE_COLORBLOCK: {
		float w = atof(tokens[4].c_str());
		float h = atof(tokens[5].c_str());
		obj = new ColorBlock(w, h);
		break;
	}
	case OBJECT_TYPE_PORTAL:
		{	
			float r = atof(tokens[4].c_str());
			float b = atof(tokens[5].c_str());
			int scene_id = atoi(tokens[6].c_str());
			obj = new CPortal(x, y, r, b, scene_id);
		}
		break;
	default:
		DebugOut(L"[ERR] Invalid object type: %d\n", object_type);
		return;
	}
	LPANIMATION_SET ani_set = animation_sets->Get(ani_set_id);
	// General object setup
	obj->SetPosition(x, y);

	obj->SetAnimationSet(ani_set);
	objects.push_back(obj);
}

void CPlayScene::_ParseSection_TILEMAP(string line) {	//doc map tu file txt

	int ID, rowMap, columnMap, rowTile, columnTile, totalTile;
	LPCWSTR path = ToLPCWSTR(line);
	ifstream f(path, ios::in);
	f >> ID >> rowMap >> columnMap >> rowTile >> columnTile >> totalTile;
	int** tileMapData = new int *[rowMap];
	for (int i = 0; i < rowMap; i++) {
		tileMapData[i] = new int[columnMap];
		for (int j = 0; j < columnMap; j++) {
			f >> tileMapData[i][j];
		}
	}
	f.close();
	map = new Map(ID, rowMap, columnMap, rowTile, columnTile, totalTile);
	map->GetSpriteTile();
	map->SetMap(tileMapData);
}



void CPlayScene::Load()
{
	DebugOut(L"[INFO] Start loading scene resources from : %s \n", sceneFilePath);

	ifstream f;
	f.open(sceneFilePath);

	// current resource section flag
	int section = SCENE_SECTION_UNKNOWN;					

	char str[MAX_SCENE_LINE];
	while (f.getline(str, MAX_SCENE_LINE))
	{
		string line(str);

		if (line[0] == '#') continue;	// skip comment lines	

		if (line == "[TEXTURES]") { section = SCENE_SECTION_TEXTURES; continue; }
		if (line == "[SPRITES]") { 
			section = SCENE_SECTION_SPRITES; continue; }
		if (line == "[ANIMATIONS]") { 
			section = SCENE_SECTION_ANIMATIONS; continue; }
		if (line == "[ANIMATION_SETS]") { 
			section = SCENE_SECTION_ANIMATION_SETS; continue; }
		if (line == "[OBJECTS]") { 
			section = SCENE_SECTION_OBJECTS; continue; }
		if (line == "[TILEMAP]") {
			section = SCENE_SECTION_DRAWMAP; continue;
		}
		if (line[0] == '[') { section = SCENE_SECTION_UNKNOWN; continue; }	
		
		//
		// data section
		//
		switch (section)
		{ 
			case SCENE_SECTION_TEXTURES: _ParseSection_TEXTURES(line); break;
			case SCENE_SECTION_SPRITES: _ParseSection_SPRITES(line); break;
			case SCENE_SECTION_ANIMATIONS: _ParseSection_ANIMATIONS(line); break;
			case SCENE_SECTION_ANIMATION_SETS: _ParseSection_ANIMATION_SETS(line); break;
			case SCENE_SECTION_OBJECTS: _ParseSection_OBJECTS(line); break;
			case SCENE_SECTION_DRAWMAP: _ParseSection_TILEMAP(line); break;
		}
	}

	f.close();

	CTextures::GetInstance()->Add(ID_TEX_BBOX, L"textures\\bbox.png", D3DCOLOR_XRGB(255, 255, 255));

	DebugOut(L"[INFO] Done loading scene resources %s\n", sceneFilePath);
}

void CPlayScene::Update(DWORD dt)
{
	// We know that Mario is the first object in the list hence we won't add him into the colliable object list
	// TO-DO: This is a "dirty" way, need a more organized way 
	CGameObject* obj = NULL;

	vector<LPGAMEOBJECT> coObjects;

	for (size_t i = 1; i < objects.size(); i++)
	{
		coObjects.push_back(objects[i]);
	}

	for (size_t i = 0; i < objects.size(); i++)
	{
		objects[i]->Update(dt, &coObjects);
	}

	 //skip the rest if scene was already unloaded (Mario::Update might trigger PlayScene::Unload)
	if (player == NULL) return; 
	
	if (CGame::GetInstance()->GetScene() == SCENE_TEST) {
		
		if (player == NULL) return;

		// Update camera to follow mario
		float cx, cy, __cx,__cy, sw, sh, mw, mh, mx, my, px, py;
		player->GetPosition(px, py);

		CGame *game = CGame::GetInstance();
		sw = game->GetScreenWidth();
		sh = game->GetScreenHeight();
		mw = map->GetMapWidth();
		mh = map->GetMapHeight();
		cx = 0; cy = 250;
		__cx = px-20; __cy = cy; //Cam theo Mario
		if (__cx < cx)
			__cx = cx;//khong cho qua ben trai dau map
		CGame::GetInstance()->SetCamPos((int) __cx,(int) __cy);
		map->SetCamPos((int)__cx, (int)__cy);
	}
	

}

void CPlayScene::Render()
{
	map->DrawMap();
	for (int i = 0; i < objects.size(); i++)
		objects[i]->Render();
}

/*
	Unload current scene
*/
void CPlayScene::Unload()
{
	for (int i = 0; i < objects.size(); i++)
		delete objects[i];

	objects.clear();
	player = NULL;

	DebugOut(L"[INFO] Scene %s unloaded! \n", sceneFilePath);
}

void CPlayScenceKeyHandler::OnKeyDown(int KeyCode)
{
	//DebugOut(L"[INFO] KeyDown: %d\n", KeyCode);

	CMario *mario = ((CPlayScene*)scence)->GetPlayer();
	switch (KeyCode)
	{

	case DIK_A:
		mario->Reset();
		break;
	case DIK_F1:
		mario->y -= 20;	//day mario len xiu de ko rot khoi map
		mario->SetLevel(MARIO_LEVEL_BIG);
		break;
	case DIK_F2:
		mario->SetLevel(MARIO_LEVEL_SMALL);
		break;
	case DIK_F3:
		mario->y -= 20;	//day mario len xiu de ko rot khoi map
		mario->SetLevel(MARIO_LEVEL_FIRE);
		break;
	}
}

void CPlayScenceKeyHandler::KeyState(BYTE *states)
{
	CGame *game = CGame::GetInstance();
	CMario *mario = ((CPlayScene*)scence)->GetPlayer();

	// disable control key when Mario die 
	if (mario->GetState() == MARIO_STATE_DIE) return;

	//--------------------RUN/TURN/FLY----------------------------
	if ((game->IsKeyDown(DIK_RIGHT)) && (game->IsKeyDown(DIK_Q))) {
		mario->nx = 1;
		if (mario->vx < 0) {
			mario->SetState(MARIO_STATE_TURN);
		}
		mario->SetState(MARIO_STATE_RUN_RIGHT);
		if (abs(mario->vx) >= MARIO_RUNNING_MAXSPEED && (game->IsKeyDown(DIK_E))) {
			mario->vx = MARIO_RUNNING_MAXSPEED * mario->dt;
			mario->SetState(MARIO_STATE_FLY_RIGHT);
		}
			

	}
	else if ((game->IsKeyDown(DIK_LEFT)) && (game->IsKeyDown(DIK_Q))) {
		mario->nx = -1;
		if (mario->vx > 0) {
			mario->SetState(MARIO_STATE_TURN);
		}
		mario->SetState(MARIO_STATE_RUN_LEFT);
		if (abs(mario->vx) >= MARIO_RUNNING_MAXSPEED && (game->IsKeyDown(DIK_E))) {
			mario->vx = -MARIO_RUNNING_MAXSPEED * mario->dt;
			mario->SetState(MARIO_STATE_FLY_LEFT);
		}
	//-------------------------------------WALK---------------------------
	}else if (game->IsKeyDown(DIK_RIGHT) && !((game->IsKeyDown(DIK_Q) || game->IsKeyDown(DIK_Q)))) {
		mario->nx = 1;
		if (mario->vx < 0) {
			mario->SetState(MARIO_STATE_TURN);
		}
		mario->vx = MARIO_WALKING_SPEED * mario->dt;		
		//mario->SetState(MARIO_STATE_WALK_RIGHT);
	}	
	else if (game->IsKeyDown(DIK_LEFT) && !((game->IsKeyDown(DIK_Q) || game->IsKeyDown(DIK_Q)))) {
		mario->nx = -1;
		if (mario->vx > 0) {
			mario->SetState(MARIO_STATE_TURN);
		}
		mario->vx = -MARIO_WALKING_SPEED * mario->dt;
		//mario->SetState(MARIO_STATE_WALK_LEFT);
	} 
	else mario->SetState(MARIO_STATE_IDLE);

	//-------------------------SIT------------------------
	if (game->IsKeyDown(DIK_DOWN)) {
		if (mario->GetLevel() != MARIO_LEVEL_SMALL)
			mario->SetState(MARIO_STATE_SIT);
	}

	//-------------------------JUMP------------------------
	if (game->IsKeyDown(DIK_SPACE)) {
		mario->SetState(MARIO_STATE_JUMP);
	}
	
	
}

void CPlayScenceKeyHandler::OnKeyUp(int KeyCode) {
	CMario *mario = ((CPlayScene*)scence)->GetPlayer();

	switch (KeyCode)
		{
		case DIK_DOWN:
			mario->y -= 10;
			mario->SetState(MARIO_STATE_IDLE);
			break;
		}
}