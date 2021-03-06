#include "WorldMap.h"

using namespace std;

WorldMap::WorldMap(int id, LPCWSTR filePath) :
	CScene(id, filePath)
{
	key_handler = new WorldMapKeyHandler(this);
}

/*
	Load scene resources from scene file (textures, sprites, animations and objects)
	See scene1.txt, scene2.txt for detail format specification
*/

void WorldMap::_ParseSection_TEXTURES(string line)
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

void WorldMap::_ParseSection_SPRITES(string line)
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

void WorldMap::_ParseSection_ANIMATIONS(string line)
{
	vector<string> tokens = split(line);

	if (tokens.size() < 3) return; // skip invalid lines - an animation must at least has 1 frame and 1 frame time

	DebugOut(L"--> %s\n",ToWSTR(line).c_str());

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

void WorldMap::_ParseSection_ANIMATION_SETS(string line)
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
void WorldMap::_ParseSection_OBJECTS(string line)
{
	LPCWSTR path = ToLPCWSTR(line);
	ifstream f; f.open(path);
	if (!f)
		DebugOut(L"\nFailed to open object file!");

	char str[MAX_SCENE_LINE];
	while (f.getline(str, MAX_SCENE_LINE)) {

		string line(str);
		vector<string> tokens = split(line);

		DebugOut(L"--> %s\n", ToWSTR(line).c_str());

		if (line[0] == '#') continue;
		if (tokens.size() < 3) continue; // skip invalid lines - an object set must have at least id, x, y

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
		case OBJECT_TYPE_PLATFORM: {
			float w = (float)atof(tokens[3].c_str());
			float h = (float)atof(tokens[4].c_str());
			obj = new Platform();
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
		case OBJECT_TYPE_CARD:{
			float cgLeft = (float)atof(tokens[5].c_str());
			float cgUp = (float)atof(tokens[6].c_str());
			float cgRight = (float)atof(tokens[7].c_str());
			float cgDown = (float)atof(tokens[8].c_str());
			obj = new Card(cgLeft, cgUp, cgRight, cgDown);
			break; 
		}
		case OBJECT_TYPE_START: {
			float cgLeft = (float)atof(tokens[5].c_str());
			float cgUp = (float)atof(tokens[6].c_str());
			float cgRight = (float)atof(tokens[7].c_str());
			float cgDown = (float)atof(tokens[8].c_str());
			obj = new Start(cgLeft, cgUp, cgRight, cgDown);
			break;
		}		
		case OBJECT_TYPE_SCENE: {
			float cgLeft = (float)atof(tokens[5].c_str());
			float cgUp = (float)atof(tokens[6].c_str());
			float cgRight = (float)atof(tokens[7].c_str());
			float cgDown = (float)atof(tokens[8].c_str());
			float model = (float)atof(tokens[4].c_str());
			obj = new Scene(model, cgLeft, cgUp, cgRight, cgDown);
			break;
		}			
		default:
			DebugOut(L"[ERR] Invalid object type: %d\n", object_type);
			return;
		}

		LPANIMATION_SET ani_set = animation_sets->Get(ani_set_id);
		obj->SetPosition(x, y);
		obj->SetAnimationSet(ani_set);
		objects.push_back(obj);
	}
	f.close();
}

void WorldMap::_ParseSection_TILEMAP(string line) {	//doc map tu file txt

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



void WorldMap::Load()
{
	DebugOut(L"[INFO] Start loading scene resources from : %s \n", sceneFilePath);

	ifstream f;
	f.open(sceneFilePath);

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

	BackUp* backup = BackUp::GetInstance();
	CTextures::GetInstance()->Add(ID_TEX_BBOX, L"textures\\bbox.png", D3DCOLOR_XRGB(255, 255, 255));
	for (size_t i = 1; i < objects.size(); i++)
		if (objects[i]->GetType() == PORTAL) {
			CPortal* portal = dynamic_cast<CPortal*>(objects[i]);
			if (portal->GetSceneId() == backup->scene && backup->scene != 0)
				if (backup->level == MARIO_LEVEL_SMALL)
					player->SetPosition(objects[i]->x, objects[i]->y);					
				else 
					player->SetPosition(objects[i]->x, objects[i]->y - 7.f);
		}			
	DebugOut(L"[INFO] Done loading scene resources %s\n", sceneFilePath);
}

void WorldMap::Update(DWORD dt)
{
	CGameObject* obj = NULL;

	vector<LPGAMEOBJECT> coObjects;

	playTime -= dt;

	for (size_t i = 0; i < objects.size(); i++)
	{
		coObjects.push_back(objects[i]);
	}

	for (size_t i = 0; i < objects.size(); i++)
	{
		LPGAMEOBJECT e = objects[i];
		objects[i]->Update(dt, &coObjects);
	}
	Camera* camera = new Camera(player, game, map);
	camera->Update(dt);

	//skip the rest if scene was already unloaded (Mario::Update might trigger PlayScene::Unload)
	if (player == NULL) return;
}


void WorldMap::Render()
{
	map->DrawMap();
	for (size_t i = 0; i < objects.size(); i++) {
			objects[i]->Render();
	}
	Board* board = new Board(CGame::GetInstance()->GetCamX(), CGame::GetInstance()->GetCamY() + SCREEN_HEIGHT - DISTANCE_FROM_BOTTOM_CAM_TO_TOP_BOARD);
	board->Render(player, playTime);
}

/*
	Unload current scene
*/
void WorldMap::Unload()
{
	for (size_t i = 0; i < objects.size(); i++)
		delete objects[i];


	objects.clear();
	player = NULL;

	DebugOut(L"[INFO] Scene %s unloaded! \n", sceneFilePath);
}

void WorldMapKeyHandler::OnKeyDown(int KeyCode)
{
	CMario* player = ((WorldMap*)scence)->GetPlayer();
	if (player != NULL)
	{
		switch (KeyCode)
		{
		case DIK_RIGHT:
			if (player->cgRight) {
				player->vx = PLAYER_SPEED;
				player->vy = 0;
			}			
			break;
		case DIK_LEFT:
			if (player->cgLeft) {
				player->vx = -PLAYER_SPEED;
				player->vy = 0;
			}
			break;
		case DIK_UP:
			if (player->cgUp) {
				player->vx = 0;
				player->vy = -PLAYER_SPEED;
			}
			break;
		case DIK_DOWN:
			if (player->cgDown) {
				player->vx = 0;
				player->vy = PLAYER_SPEED;
			}
			break;
		case DIK_R:
			//restart position
			if (player->level == MARIO_LEVEL_SMALL)
				player->SetPosition(32, 48);
			else player->SetPosition(32, 41);
			player->vx = player->vy = 0;
			player->SetMove(false, false, true, false);
			break;
		}
	}
}

void WorldMapKeyHandler::KeyState(BYTE *states)
{

}

void WorldMapKeyHandler::OnKeyUp(int KeyCode) {

}

void WorldMap::LoadBackUp() {
	BackUp* backup = BackUp::GetInstance();
	backup->LoadBackUp(player);
}

void WorldMap::BackUpPlayer() {
	if (player != NULL) {
		BackUp* backup = BackUp::GetInstance();
		backup->BackUpMario(player);
	}
}
