#include "SceneManager.h"

using namespace std;

SceneManager::SceneManager(int id, LPCWSTR filePath) :
	CScene(id, filePath)
{
	key_handler = new CPlayScenceKeyHandler(this);
}

/*
	Load scene resources from scene file (textures, sprites, animations and objects)
	See scene1.txt, scene2.txt for detail format specification
*/

void SceneManager::_ParseSection_TEXTURES(string line)
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

void SceneManager::_ParseSection_SPRITES(string line)
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

void SceneManager::_ParseSection_ANIMATIONS(string line)
{
	vector<string> tokens = split(line);

	if (tokens.size() < 3) return; // skip invalid lines - an animation must at least has 1 frame and 1 frame time

	//DebugOut(L"--> %s\n",ToWSTR(line).c_str());

	LPANIMATION ani = new CAnimation();

	int ani_id = atoi(tokens[0].c_str());
	for (size_t i = 1; i < tokens.size(); i += 2)	// why i+=2 ?  sprite_id | frame_time  
	{
		int sprite_id = atoi(tokens[i].c_str());
		int frame_time = atoi(tokens[i + 1].c_str());
		ani->Add(sprite_id, frame_time);
	}
	CAnimations::GetInstance()->Add(ani_id, ani);
}

void SceneManager::_ParseSection_ANIMATION_SETS(string line)
{
	vector<string> tokens = split(line);

	if (tokens.size() < 2) return; // skip invalid lines - an animation set must at least id and one animation id

	int ani_set_id = atoi(tokens[0].c_str());

	LPANIMATION_SET s = new CAnimationSet();

	CAnimations *animations = CAnimations::GetInstance();

	for (size_t i = 1; i < tokens.size(); i++)
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
void SceneManager::_ParseSection_OBJECTS(string line)
{
	vector<string> tokens = split(line);

	//DebugOut(L"--> %s\n",ToWSTR(line).c_str());

	if (tokens.size() < 3) return; // skip invalid lines - an object set must have at least id, x, y

	int object_type = atoi(tokens[0].c_str());
	float x = (float)atof(tokens[1].c_str());
	float y = (float)atof(tokens[2].c_str());
	int ani_set_id = atoi(tokens[3].c_str());

	CAnimationSets * animation_sets = CAnimationSets::GetInstance();

	CGameObject *obj = NULL;

	switch (object_type)
	{
	case OBJECT_TYPE_MARIO:
		if (player != NULL)
		{
			DebugOut(L"[ERROR] MARIO object was created before!\n");
			return;
		}
		obj = new CMario(x, y);
		player = (CMario*)obj;

		DebugOut(L"[INFO] Player object created!\n");
		break;

	case OBJECT_TYPE_BRICK: {
		obj = new CBrick();
		break;
	}
	case OBJECT_TYPE_PLATFORM: {
		float w = (float)atof(tokens[4].c_str());
		float h = (float)atof(tokens[5].c_str());
		obj = new Platform(w, h);
		break;
	}

	case OBJECT_TYPE_PORTAL:
	{
		float r = (float)atof(tokens[4].c_str());
		float b = (float)atof(tokens[5].c_str());
		float scene_id = (float)atoi(tokens[6].c_str());
		obj = new CPortal(x, y, r, b, scene_id);
		break;
	}
	case OBJECT_TYPE_BUSH:
		obj = new Bush();
		break;
	case OBJECT_TYPE_CARD:
		obj = new Card();
		break;
	case OBJECT_TYPE_START:
		obj = new Start();
		break;
	case OBJECT_TYPE_SCENE: {
		float model = (float)atof(tokens[4].c_str());
		obj = new Scene(model);
		break;
	}
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

void SceneManager::_ParseSection_TILEMAP(string line) {	//doc map tu file txt

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



void SceneManager::Load()
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
			section = SCENE_SECTION_SPRITES; continue;
		}
		if (line == "[ANIMATIONS]") {
			section = SCENE_SECTION_ANIMATIONS; continue;
		}
		if (line == "[ANIMATION_SETS]") {
			section = SCENE_SECTION_ANIMATION_SETS; continue;
		}
		if (line == "[OBJECTS]") {
			section = SCENE_SECTION_OBJECTS; continue;
		}
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

void SceneManager::Update(DWORD dt)
{
	// We know that Mario is the first object in the list hence we won't add him into the colliable object list
	// TO-DO: This is a "dirty" way, need a more organized way 
	CGameObject* obj = NULL;

	vector<LPGAMEOBJECT> coObjects;

	playTime -= dt;

	for (size_t i = 1; i < objects.size(); i++)
	{
		coObjects.push_back(objects[i]);
	}

	for (size_t i = 0; i < objects.size(); i++)
	{

		LPGAMEOBJECT e = objects[i];
		if (objects[i]->CheckObjectInCamera(objects[i]))
			objects[i]->Update(dt, &coObjects);

	}

	for (size_t i = 0; i < objects.size(); i++) {
		if (objects.at(i)->isFinish)
			objects.erase(objects.begin() + i);
	}


	Camera* camera = new Camera(player, game, map);
	camera->Update(dt);
	//skip the rest if scene was already unloaded (Mario::Update might trigger PlayScene::Unload)
	if (player == NULL) return;

}


void SceneManager::Render()
{
	map->DrawMap();
	for (size_t i = 0; i < objects.size(); i++) {

		if (objects[i]->CheckObjectInCamera(objects[i]))
			objects[i]->Render();

	}
	Board* board = new Board(CGame::GetInstance()->GetCamX(), CGame::GetInstance()->GetCamY() + SCREEN_HEIGHT - DISTANCE_FROM_BOTTOM_CAM_TO_TOP_BOARD);
	board->Render(player, playTime);
}

/*
	Unload current scene
*/
void SceneManager::Unload()
{
	for (size_t i = 0; i < objects.size(); i++)
		delete objects[i];

	objects.clear();
	player = NULL;

	DebugOut(L"[INFO] Scene %s unloaded! \n", sceneFilePath);
}

void SceneManagerKeyHandler::OnKeyDown(int KeyCode)
{
	//DebugOut(L"[INFO] KeyDown: %d\n", KeyCode);
	CGame *game = CGame::GetInstance();
	CMario *mario = ((SceneManager*)scence)->GetPlayer();
	if (CGame::GetInstance()->GetScene() != WORLDMAP) {
		switch (KeyCode)
		{
		case DIK_SPACE:
			mario->StartLimitJump();
			break;
		case DIK_A:
			mario->Reset();
			break;
		case DIK_F1:
			mario->y -= 20;
			mario->SetLevel(MARIO_LEVEL_BIG);
			break;
		case DIK_F2:
			mario->SetLevel(MARIO_LEVEL_SMALL);
			break;
		case DIK_F3:
			mario->y -= 20;
			mario->SetLevel(MARIO_LEVEL_FIRE);
			break;
		case DIK_F4:
			mario->y -= 20;
			mario->SetLevel(MARIO_LEVEL_RACOON);
			break;
		case DIK_Q://-----------------------SHOOT FIRE--------------------------
			if (mario->level == MARIO_LEVEL_FIRE)
				mario->SetState(MARIO_STATE_SHOOT_FIRE);
			break;
		case DIK_E:
			if (game->IsKeyDown(DIK_Q)) {
				mario->StartLimitFly();
			}
			break;
		}
	}
}

void SceneManagerKeyHandler::KeyState(BYTE *states)
{
	CGame *game = CGame::GetInstance();
	CMario *mario = ((SceneManager*)scence)->GetPlayer();

	// disable control key when Mario die 
	if (mario->GetState() == MARIO_STATE_DIE) return;

	//--------------------RUN/TURN/FLY/WALK----------------------------
	if (CGame::GetInstance()->GetScene() != WORLDMAP) {
		if (game->IsKeyDown(DIK_RIGHT)) {
			if (game->IsKeyDown(DIK_Q)) {
				mario->SetState(MARIO_STATE_RUN_RIGHT);
				if (game->IsKeyDown(DIK_E) && mario->state == MARIO_STATE_RUN_MAXSPEED) {
					mario->isFlying = true;
					mario->SetState(MARIO_STATE_FLY_RIGHT);
				}
			}
			else mario->SetState(MARIO_STATE_WALK_RIGHT);
		}
		else if (game->IsKeyDown(DIK_LEFT)) {
			if (game->IsKeyDown(DIK_Q)) {
				mario->SetState(MARIO_STATE_RUN_LEFT);
				if (game->IsKeyDown(DIK_E) && mario->state == MARIO_STATE_RUN_MAXSPEED) {
					mario->isFlying = true;
					mario->SetState(MARIO_STATE_FLY_LEFT);
				}
			}
			else mario->SetState(MARIO_STATE_WALK_LEFT);
		}
		else mario->SetState(MARIO_STATE_IDLE);

		//-------------------------SIT------------------------
		if (game->IsKeyDown(DIK_DOWN) && !(game->IsKeyDown(DIK_RIGHT) || game->IsKeyDown(DIK_LEFT))) {
			if (mario->GetLevel() != MARIO_LEVEL_SMALL)
				mario->SetState(MARIO_STATE_SIT);
		}
		//-------------------------JUMP------------------------
		if (game->IsKeyDown(DIK_SPACE)) {
			if (mario->vy <= 0) {
				mario->SetState(MARIO_STATE_JUMP);
			}
			else if (mario->level == MARIO_LEVEL_RACOON) {
				mario->SetState(MARIO_STATE_JUMP);
			}

		}

		//-----------------------SPIN-------------------------
		if (mario->level == MARIO_LEVEL_RACOON && game->IsKeyDown(DIK_Q)) {
			if (!mario->isHolding)
				if (!(game->IsKeyDown(DIK_RIGHT) || game->IsKeyDown(DIK_LEFT) || game->IsKeyDown(DIK_SPACE)))
					mario->SetState(MARIO_STATE_SPIN);
		}
	}
	else {
		if (game->IsKeyDown(DIK_RIGHT)) {
			mario->vx += MARIO_WALKING_SPEED * mario->dt;
		}
		if (game->IsKeyDown(DIK_LEFT)) {
			mario->vx -= MARIO_WALKING_SPEED * mario->dt;
		}
		if (game->IsKeyDown(DIK_UP)) {
			mario->vy -= MARIO_WALKING_SPEED * mario->dt;
		}
		if (game->IsKeyDown(DIK_DOWN)) {
			mario->vy += MARIO_WALKING_SPEED * mario->dt;
		}
	}
}

void SceneManagerKeyHandler::OnKeyUp(int KeyCode) {
	CMario *mario = ((SceneManager*)scence)->GetPlayer();
	if (CGame::GetInstance()->GetScene() != WORLDMAP) {
		switch (KeyCode)
		{
		case DIK_DOWN:
			mario->y -= 10;
			mario->SetState(MARIO_STATE_IDLE);
			break;
		case DIK_Q:
			mario->isHolding = false;
			mario->SetState(MARIO_STATE_KICK);
		}
	}
	else {
		switch (KeyCode) {
		case DIK_RIGHT:
			mario->vx = 0;
			break;
		case DIK_LEFT:
			mario->vx = 0;
			break;
		case DIK_UP:
			mario->vy = 0;
			break;
		case DIK_DOWN:
			mario->vy = 0;
			break;
		}
	}
}


