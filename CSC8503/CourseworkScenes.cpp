#include "CourseworkScenes.h"
#include "Debug.h"
#include "Window.h"
#include "SceneManager.h"
#include "NetworkedGame.h"
#include "Coursework.h"

void NCL::CSC8503::MainMenuSceneState::OnAwake(){	
	SceneManager::GetSceneManager()->SetCurrentScene(Scene::MainMenu);
}

NCL::CSC8503::PushdownState::PushdownResult NCL::CSC8503::MainMenuSceneState::OnUpdate(float dt, PushdownState** newState)
{
	if (Window::GetKeyboard()->KeyPressed(KeyCodes::NUM1)) {
		*newState = new GameSceneState();
		return PushdownResult::Push;
	}
	if (Window::GetKeyboard()->KeyPressed(KeyCodes::NUM2)){
		*newState = new CollisionTestSceneState();
		return PushdownResult::Push;
	}
	if (Window::GetKeyboard()->KeyPressed(KeyCodes::NUM3)) {
		*newState = new ServerState();
		return PushdownResult::Push;
	}

	if (Window::GetKeyboard()->KeyPressed(KeyCodes::NUM4)) {
		*newState = new ClientState();
		return PushdownResult::Push;
	}

	if (Window::GetKeyboard()->KeyPressed(KeyCodes::ESCAPE)) {
		SceneManager::GetSceneManager()->SetIsForceQuit(true);
		return PushdownResult::Pop;
	}
	return PushdownResult::NoChange;
}

NCL::CSC8503::PushdownState::PushdownResult NCL::CSC8503::CollisionTestSceneState::OnUpdate(float dt, PushdownState** newState)
{
	if (Window::GetKeyboard()->KeyPressed(KeyCodes::ESCAPE)) {
		*newState = new MainMenuSceneState();
		return PushdownResult::Push;
	}
	return PushdownResult::NoChange;
}

void NCL::CSC8503::CollisionTestSceneState::OnAwake(){

	SceneManager::GetSceneManager()->SetCurrentScene(Scene::CollisionTest);
}

NCL::CSC8503::PushdownState::PushdownResult NCL::CSC8503::GameSceneState::OnUpdate(float dt, PushdownState** newState)
{
	if (Window::GetKeyboard()->KeyPressed(KeyCodes::ESCAPE)) {
		*newState = new MainMenuSceneState();
		return PushdownResult::Push;
	}
	return PushdownResult::NoChange;
}

void NCL::CSC8503::GameSceneState::OnAwake() {
	SceneManager::GetSceneManager()->SetCurrentScene(Scene::GameScene);
	auto* gameScene = (Coursework*)(SceneManager::GetSceneManager()->GetCurrentScene());
}

NCL::CSC8503::PushdownState::PushdownResult NCL::CSC8503::ServerState::OnUpdate(float dt, PushdownState** newState) {
	if (Window::GetKeyboard()->KeyPressed(KeyCodes::ESCAPE)) {
		*newState = new MainMenuSceneState();
		return PushdownResult::Push;
	}
	return PushdownResult::NoChange;
}

void NCL::CSC8503::ServerState::OnAwake(){
	SceneManager::GetSceneManager()->SetCurrentScene(Scene::NetworkScene);
	auto* server = (NetworkedGame*)SceneManager::GetSceneManager()->GetCurrentScene();
	server->StartAsServer();
}

NCL::CSC8503::PushdownState::PushdownResult NCL::CSC8503::ClientState::OnUpdate(float dt, PushdownState** newState) {
	if (Window::GetKeyboard()->KeyPressed(KeyCodes::ESCAPE)) {
		*newState = new MainMenuSceneState();
		return PushdownResult::Push;
	}
	return PushdownResult::NoChange;
}

void NCL::CSC8503::ClientState::OnAwake()
{
	SceneManager::GetSceneManager()->SetCurrentScene(Scene::NetworkScene);
	auto* client = (NetworkedGame*)SceneManager::GetSceneManager()->GetCurrentScene();
	client->StartAsClient(127, 0, 0, 1);
}
