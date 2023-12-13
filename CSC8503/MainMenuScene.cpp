#include "MainMenuScene.h"

NCL::CSC8503::MainMenuScene::MainMenuScene() : TutorialGame(false)
{
	isMainMenuScene = true;
	InitialiseAssets();
}

NCL::CSC8503::MainMenuScene::~MainMenuScene(){

}

void NCL::CSC8503::MainMenuScene::UpdateGame(float dt) {
	TutorialGame::UpdateGame(dt);
}

void NCL::CSC8503::MainMenuScene::InitialiseAssets() {
	TutorialGame::InitialiseAssets();
}

void NCL::CSC8503::MainMenuScene::InitWorld(){
	world->ClearAndErase();
	physics->Clear();
	world->GetMainCamera().SetFreeMode(false);
}


void NCL::CSC8503::MainMenuScene::HandleMainMenuControls(){
	const Keyboard& keyboard = *Window::GetKeyboard();
	if (keyboard.KeyPressed(KeyCodes::NUM1) ) {

	}
}
