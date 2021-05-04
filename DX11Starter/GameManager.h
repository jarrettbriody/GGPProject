#pragma once
#include "ScriptManager.h"

enum class GameState
{
	Intro, MainMenu, Gameplay, GameOver, Victory
};

class GameManager : public ScriptManager
{
	map<string, Entity*>* eMap;

	GameState gs;

	const float MAX_GAME_TIME = 480.0f; // 8 minutes
	float gameTimer = MAX_GAME_TIME;

	int enemiesAlive;

	void Init();

	void Update();

public:
	// AI
	Grid grid1;
	Grid grid2;
	Grid grid3;
	Grid grid4;
	Grid grid5;
	Grid grid6;
	Grid grid7;
	Grid grid8;

	void DecrementEnemiesAlive();
};

