#pragma once
#include <iostream>
#include <fstream>
#include "Game.h"
#include "Scence.h"
#include "GameObject.h"
#include "define.h"
#include "Platform.h"
#include "Utils.h"
#include "Textures.h"
#include "Sprites.h"
#include "Mario.h"
#include "Koopas.h"
#include "Camera.h"
#include "Goomba.h"
#include "Ground.h"
#include "Curtain.h"
#include "Mushroom.h"
#include "Leaf.h"
#include "Arrow.h"
#include "ShiningThree.h"

#define ONE_PLAYER_POS	160
#define TWO_PLAYER_POS	180

class IntroScene : public CScene
{
private:
	float CamX = 0, CamY = 0;
protected:
	CMario* player;	// A play scene has to have player, right? 

	CGame* game = CGame::GetInstance();
	DWORD introTime = 0;
	vector<LPGAMEOBJECT> objects;
	void _ParseSection_TEXTURES(string line);
	void _ParseSection_SPRITES(string line);
	void _ParseSection_ANIMATIONS(string line);
	void _ParseSection_ANIMATION_SETS(string line);
	void _ParseSection_OBJECTS(string line);

public:
	int section;
	IntroScene(int id, LPCWSTR filePath);
	Arrow* mainArrow;
	virtual void Load();
	virtual void Update(DWORD dt);
	virtual void Render();
	virtual void Unload();
	CMario * GetPlayer() { return player; }

	//friend class CPlayScenceKeyHandler;
};

class IntroSceneKeyHandler : public CScenceKeyHandler
{
public:
	virtual void KeyState(BYTE *states);
	virtual void OnKeyDown(int KeyCode);
	virtual void OnKeyUp(int KeyCode);

	IntroSceneKeyHandler(CScene *s) :CScenceKeyHandler(s) {};
};
