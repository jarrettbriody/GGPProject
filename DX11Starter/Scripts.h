#pragma once
#include "Barrel.h"
#include "FPSController.h"
#include "BloodIcicle.h"
#include "BloodSword.h"
#include "BloodOrb.h"
#include "GameManager.h"
#include "BloodPool.h"
#include "AmbientParticles.h"
#include "EnemyTest.h"
#include "MainMenuManager.h"
#include "TowerEnemy.h"
#include "HornedEnemy.h"
#include "BullEnemy.h"
#include "CyclopsEnemy.h"
#include "Projectile.h"
#include "BodyPart.h"

namespace Scripts {
	enum class SCRIPT_NAMES {
		BARREL,
		FPSCONTROLLER,
		BLOODICICLE,
		BLOODORB,
		AMBIENTPARTICLES,
		BLOODSWORD,
		GAMEMANAGER,
		BLOODPOOL,
		ENEMYTEST,
		MAINMENUMANAGER,
		TOWERENEMY,
		HORNEDENEMY,
		BULLENEMY,
		CYCLOPSENEMY,
		PROJECTILE,
		BODYPART,
	};

	static map<string, SCRIPT_NAMES> scriptNameMap = {
		{"BARREL", SCRIPT_NAMES::BARREL},
		{"FPSCONTROLLER", SCRIPT_NAMES::FPSCONTROLLER},
		{"BLOODICICLE", SCRIPT_NAMES::BLOODICICLE},
		{"BLOODORB", SCRIPT_NAMES::BLOODORB},
		{"AMBIENTPARTICLES", SCRIPT_NAMES::AMBIENTPARTICLES},
		{"BLOODSWORD", SCRIPT_NAMES::BLOODSWORD},
		{"GAMEMANAGER", SCRIPT_NAMES::GAMEMANAGER},
		{"BLOODPOOL", SCRIPT_NAMES::BLOODPOOL},
		{"ENEMYTEST", SCRIPT_NAMES::ENEMYTEST},
		{"MAINMENUMANAGER", SCRIPT_NAMES::MAINMENUMANAGER},
		{"TOWERENEMY", SCRIPT_NAMES::TOWERENEMY},
		{"HORNEDENEMY", SCRIPT_NAMES::HORNEDENEMY},
		{"BULLENEMY", SCRIPT_NAMES::BULLENEMY},
		{"CYCLOPSENEMY", SCRIPT_NAMES::CYCLOPSENEMY},
		{"PROJECTILE", SCRIPT_NAMES::PROJECTILE},
		{"BODYPART", SCRIPT_NAMES::BODYPART},
	};

	static map<SCRIPT_NAMES, string> scriptStringNameMap = {
		{SCRIPT_NAMES::BARREL, "BARREL"},
		{SCRIPT_NAMES::FPSCONTROLLER, "FPSCONTROLLER"},
		{SCRIPT_NAMES::BLOODICICLE, "BLOODICICLE"},
		{SCRIPT_NAMES::BLOODORB, "BLOODORB"},
		{SCRIPT_NAMES::AMBIENTPARTICLES, "AMBIENTPARTICLES"},
		{SCRIPT_NAMES::BLOODSWORD, "BLOODSWORD"},
		{SCRIPT_NAMES::GAMEMANAGER, "GAMEMANAGER"},
		{SCRIPT_NAMES::BLOODPOOL, "BLOODPOOL"},
		{SCRIPT_NAMES::ENEMYTEST, "ENEMYTEST"},
		{SCRIPT_NAMES::MAINMENUMANAGER, "MAINMENUMANAGER"},
		{SCRIPT_NAMES::TOWERENEMY, "TOWERENEMY"},
		{SCRIPT_NAMES::HORNEDENEMY, "HORNEDENEMY"},
		{SCRIPT_NAMES::BULLENEMY, "BULLENEMY"},
		{SCRIPT_NAMES::CYCLOPSENEMY, "CYCLOPSENEMY"},
		{SCRIPT_NAMES::PROJECTILE, "PROJECTILE"},
		{SCRIPT_NAMES::BODYPART, "BODYPART"},

	};

	namespace {
		static ScriptManager* Factory(Entity* e, SCRIPT_NAMES n) {
			ScriptManager* script = nullptr;

			switch (n)
			{
			case Scripts::SCRIPT_NAMES::BARREL:
				script = new TestScript();
				break;
			case Scripts::SCRIPT_NAMES::FPSCONTROLLER:
				script = new FPSController();
				break;
			case Scripts::SCRIPT_NAMES::BLOODICICLE:
				script = new BloodIcicle();
				break;
			case Scripts::SCRIPT_NAMES::BLOODORB:
				script = new BloodOrb();
				break;
			case Scripts::SCRIPT_NAMES::AMBIENTPARTICLES:
				script = new AmbientParticles();
				break;
			case Scripts::SCRIPT_NAMES::BLOODSWORD:
				script = new BloodSword();
				break;
			case Scripts::SCRIPT_NAMES::GAMEMANAGER:
				script = new GameManager();
				break;
			case Scripts::SCRIPT_NAMES::BLOODPOOL:
				script = new BloodPool();
				break;
			case Scripts::SCRIPT_NAMES::ENEMYTEST:
				script = new EnemyTest();
				break;
			case Scripts::SCRIPT_NAMES::MAINMENUMANAGER:
				script = new MainMenuManager();
				break;
			case Scripts::SCRIPT_NAMES::TOWERENEMY:
				script = new TowerEnemy();
				break;
			case Scripts::SCRIPT_NAMES::HORNEDENEMY:
				script = new HornedEnemy();
				break;
			case Scripts::SCRIPT_NAMES::BULLENEMY:
				script = new BullEnemy();
				break;
			case Scripts::SCRIPT_NAMES::CYCLOPSENEMY:
				script = new CyclopsEnemy();
				break;
			case Scripts::SCRIPT_NAMES::PROJECTILE:
				script = new Projectile();
				break;
			case Scripts::SCRIPT_NAMES::BODYPART:
				script = new BodyPart();
				break;
			default:
				break;
			}

			if (script != nullptr) script->Setup(e, scriptStringNameMap[n]);

			return script;
		}
	}

	static ScriptManager* CreateScript(Entity* e, string scriptNameStr) {
		SCRIPT_NAMES scriptName = SCRIPT_NAMES::BARREL;
		if (scriptNameMap.count(scriptNameStr)) {
			scriptName = scriptNameMap[scriptNameStr];
			return Factory(e, scriptName);
		}
		else return nullptr;
	}
	static ScriptManager* CreateScript(Entity* e, SCRIPT_NAMES scriptName) {
		return Factory(e, scriptName);
	}
}