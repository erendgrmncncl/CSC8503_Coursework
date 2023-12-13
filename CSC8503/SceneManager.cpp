#include "SceneManager.h"
#include "TutorialGame.h"
#include "PushdownMachine.h"
#include "CourseworkScenes.h"	
#include "MainMenuScene.h"
#include "Coursework.h"
#include "CollisionTestScene.h"
#include "NetworkedGame.h"

SceneManager* NCL::CSC8503::SceneManager::instance = nullptr;

NCL::CSC8503::SceneManager::SceneManager() {
	currentScene = nullptr;
	InitScenes();
	InitPushdownMachine();
}

NCL::CSC8503::SceneManager::~SceneManager()
{
}

void NCL::CSC8503::SceneManager::InitScenes(){
	Coursework* courseworkScene = new Coursework();
	MainMenuScene* mainMenuScene = new MainMenuScene();
	CollisionTestScene* collisionTest = new CollisionTestScene();
	NetworkedGame* networkedGame = new NetworkedGame();
	
	gameScenesMap =
	{
		{Scene::MainMenu, (TutorialGame*)mainMenuScene},
		{Scene::GameScene, (TutorialGame*)courseworkScene},
		{Scene::CollisionTest, (TutorialGame*)collisionTest},
		{Scene::NetworkScene, (TutorialGame*)networkedGame}
	};
}

void NCL::CSC8503::SceneManager::InitPushdownMachine()
{
	pushdownMachine = new PushdownMachine(new MainMenuSceneState());
}

void NCL::CSC8503::SceneManager::SetCurrentScene(Scene scene){
	auto* nextScene = gameScenesMap[scene];
	currentScene = nextScene;
}

bool NCL::CSC8503::SceneManager::GetIsForceQuit()
{
	return isForceQuit;
}

void NCL::CSC8503::SceneManager::SetIsForceQuit(bool isForceQuit)
{
	this->isForceQuit = isForceQuit;
}

NCL::CSC8503::PushdownMachine* NCL::CSC8503::SceneManager::GetScenePushdownMachine()
{
	return pushdownMachine;
}

TutorialGame* NCL::CSC8503::SceneManager::GetCurrentScene() {
	return currentScene;
}

SceneManager* NCL::CSC8503::SceneManager::GetSceneManager()
{
	if (instance == nullptr){
		instance = new SceneManager();
	}
	return instance;
}
