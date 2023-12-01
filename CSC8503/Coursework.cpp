#include "Coursework.h"

NCL::CSC8503::Coursework::Coursework() : TutorialGame(false) {
	world->GetMainCamera().SetFreeMode(false);
	InitialiseAssets();
}

NCL::CSC8503::Coursework::~Coursework() {
	
}

void NCL::CSC8503::Coursework::InitialiseAssets() {
	cubeMesh = renderer->LoadMesh("cube.msh");
	sphereMesh = renderer->LoadMesh("sphere.msh");
	charMesh = renderer->LoadMesh("goat.msh");
	enemyMesh = renderer->LoadMesh("Keeper.msh");
	bonusMesh = renderer->LoadMesh("apple.msh");
	capsuleMesh = renderer->LoadMesh("capsule.msh");

	basicTex = renderer->LoadTexture("checkerboard.png");
	basicShader = renderer->LoadShader("scene.vert", "scene.frag");

	InitCamera();
	InitWorld();
}

void NCL::CSC8503::Coursework::InitCamera(){
	world->GetMainCamera().SetNearPlane(0.1f);
	world->GetMainCamera().SetFarPlane(500.0f);
	world->GetMainCamera().SetPitch(-15.0f);
	world->GetMainCamera().SetYaw(315.0f);
	world->GetMainCamera().SetPosition(Vector3(-60, 0, 60));
	lockedObject = nullptr;
}

void NCL::CSC8503::Coursework::UpdateKeys(){
	NCL::CSC8503::TutorialGame::UpdateKeys();
}

void NCL::CSC8503::Coursework::InitWorld()
{
	world->ClearAndErase();
	physics->Clear();

	InitMixedGridWorld(1, 1, 3.5f, 3.5f);

	InitGameExamples();
	InitDefaultFloor();
	//BridgeConstraintTest();

	testStateObject = AddStateObjectToWorld(Vector3(0, 10, 0));
}