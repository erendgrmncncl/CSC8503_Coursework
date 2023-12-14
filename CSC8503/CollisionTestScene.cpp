#include "CollisionTestScene.h"
#include "RenderObject.h"
#include "PhysicsObject.h"

NCL::CSC8503::CollisionTestScene::CollisionTestScene() : TutorialGame(false) {
	InitialiseAssets();
	InitCollisionObjMap();
}

NCL::CSC8503::CollisionTestScene::~CollisionTestScene() {

}

void NCL::CSC8503::CollisionTestScene::UpdateGame(float dt){
	TutorialGame::UpdateGame(dt);
	for (auto* stateObject : stateObjects) {
		stateObject->Update(dt);
	}
	HandleInputs();
}

void NCL::CSC8503::CollisionTestScene::InitialiseAssets() {
	cubeMesh = renderer->LoadMesh("cube.msh");
	sphereMesh = renderer->LoadMesh("sphere.msh");
	charMesh = renderer->LoadMesh("goat.msh");
	enemyMesh = renderer->LoadMesh("Keeper.msh");
	bonusMesh = renderer->LoadMesh("apple.msh");
	capsuleMesh = renderer->LoadMesh("capsule.msh");

	basicTex = renderer->LoadTexture("checkerboard.png");
	basicShader = renderer->LoadShader("scene.vert", "scene.frag");

	InitCamera();
	this->InitWorld();
}

void NCL::CSC8503::CollisionTestScene::InitWorld() {
	world->ClearAndErase();
	physics->Clear();

	InitDefaultFloor();
}

void NCL::CSC8503::CollisionTestScene::InitCollisionObjMap()
{
	volumeTypeVolumeObjMap =
	{
		{VolumeType::AABB, (CollisionVolume*)(new AABBVolume(Vector3(1.f, 1.f, 1.f))) },
		{VolumeType::Sphere, (CollisionVolume*)(new SphereVolume(1.f)) },
		{VolumeType::Capsule, (CollisionVolume*)(new CapsuleVolume(1.f, 1.f)) },
		{VolumeType::OBB, (CollisionVolume*)(new OBBVolume(Vector3(1,1,1))) }
	};

	volumeTypeMeshMap =
	{
		{VolumeType::AABB, cubeMesh },
		{VolumeType::Sphere, sphereMesh },
		{VolumeType::Capsule, capsuleMesh },
		{VolumeType::OBB, cubeMesh }
	};
}

void NCL::CSC8503::CollisionTestScene::HandleInputs()
{
	const Keyboard& keyboard = *Window::GetKeyboard();

	if (keyboard.KeyPressed(KeyCodes::NUM1)){
		AddCollisionTestObjToWorld(VolumeType::AABB);
	}
	if (keyboard.KeyPressed(KeyCodes::NUM2)){
		AddCollisionTestObjToWorld(VolumeType::OBB);
	}
	if (keyboard.KeyPressed(KeyCodes::NUM3)) {
		AddCollisionTestObjToWorld(VolumeType::Sphere);
	}
	if (keyboard.KeyPressed(KeyCodes::NUM4)){
		AddCollisionTestObjToWorld(VolumeType::Capsule);
	}
}

void NCL::CSC8503::CollisionTestScene::AddCollisionTestObjToWorld(VolumeType volumeType){
	StateGameObject* stateObj = new StateGameObject();

	CollisionVolume* volume = volumeTypeVolumeObjMap[volumeType];
	stateObj->SetBoundingVolume((CollisionVolume*)volume);

	stateObj->GetTransform()
		.SetScale(Vector3(1, 1, 1))
		.SetPosition(Vector3(0, 0, 0));

	auto* meshToAttach = volumeTypeMeshMap[volumeType];
	stateObj->SetRenderObject(new RenderObject(&stateObj->GetTransform(), meshToAttach, basicTex, basicShader));
	stateObj->SetPhysicsObject(new PhysicsObject(&stateObj->GetTransform(), stateObj->GetBoundingVolume()));

	Vector4 colour;
	switch (volume->type)
	{
	case VolumeType::AABB: {
		colour = Vector4(1, 0, 0, 1);
		break;
	}
	case VolumeType::OBB: {
		colour = Vector4(0, 1, 1, 1);
		break;
	}
	case VolumeType::Capsule:{
		colour = Vector4(0, 0, 1, 1);
		break;
	}
	case VolumeType::Sphere: {
		colour = Vector4(0, 1, 0, 1);
		break;
	}
	default:
		break;
	}

	stateObj->GetRenderObject()->SetColour(colour);

	stateObj->GetPhysicsObject()->SetInverseMass(10.f);
	stateObj->GetPhysicsObject()->InitSphereInertia();

	stateObjects.push_back(stateObj);
	world->AddGameObject(stateObj);
}
