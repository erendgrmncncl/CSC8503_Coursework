#include "Coursework.h"
#include "Player.h"
#include "RenderObject.h"
#include "PhysicsObject.h"
#include "NavigationGrid.h"
#include "PositionConstraint.h"
#include "OrientationConstraint.h"
#include "Enemy.h"

namespace {
	constexpr const char OBJECTIVE_TYPE_NODE = 'o';
	constexpr const char OBSTACLE_TYPE_NODE = 'x';
	constexpr const int FINISH_SCORE = 1;
}

NCL::CSC8503::Coursework::Coursework(bool isNetwork) : TutorialGame(false) {
	InitialiseAssets();
	world->GetMainCamera().SetFreeMode(false);
	isNetworkGame = isNetwork;
	if (!isNetwork) {
		InitCamera();
		InitWorld();
	}
}

NCL::CSC8503::Coursework::~Coursework() {

}

void NCL::CSC8503::Coursework::UpdateGame(float dt)
{
	TutorialGame::UpdateGame(dt);
	if (mazeGuard != nullptr && player != nullptr){
		TestPathFinding();

	}
	if (mazeGuard != nullptr) {
		mazeGuard->SetNodesToPlayer(testNodes);
	}
	if (player != nullptr){
		player->HandlePlayerControls(dt, *world);
	}
	if (mazeGuard != nullptr){
		mazeGuard->HandleEnemy(dt, *world);
	}

	HandleCameraModeControls();
	DisplayPathFinding();
}

void NCL::CSC8503::Coursework::InitWorldGrid() {
	float startPosX = 0;
	float startPosZ = 0;
	worldGrid = new NavigationGrid("TestGrid1.txt");
	auto* allNodes = worldGrid->GetAllNodes();
	
	int gridWidth = worldGrid->GetNavGridWidth();
	int gridHeight = worldGrid->GetNavGridHeight();
	int gridSize = worldGrid->GetNavGridSize();

	float cubeDimensions = gridSize / 2;
	
	for (int y = 0; y < gridHeight; ++y) {
		for (int x = 0; x < gridWidth; ++x) {
			GridNode& n = allNodes[(gridWidth * y) + x];
			if ((char)n.type == OBSTACLE_TYPE_NODE)
			{
				AddCubeToWorld(Vector3(startPosX + (x * gridSize), -15, startPosZ + ( y * gridSize)), Vector3(cubeDimensions, cubeDimensions, cubeDimensions), 0.f);
			}
			else if ((char)n.type == OBJECTIVE_TYPE_NODE) {
				InitObjective(Vector3(startPosX + (x * gridSize), -15, startPosZ + (y * gridSize)));
			}
			else if ((char)n.type == 'B') {
				Vector3 startPos = Vector3(startPosX + (x * gridSize), -15, startPosZ + (y * gridSize));
				Vector3 brickDimensions = Vector3(1, 1, 1);
				CreateBrickWall(startPos, brickDimensions,true, 4, 5);
			}
			else if ((char)n.type == 'b')
			{
				Vector3 startPos = Vector3(startPosX + (x * gridSize), -15, startPosZ + (y * gridSize));
				Vector3 brickDimensions = Vector3(1, 1, 1);
				CreateBrickWall(startPos, brickDimensions, false, 4, 5);
			}
			else if ((char)n.type == 'E') {
				Vector3 spawnPos = Vector3(startPosX + (x * gridSize), -15, startPosZ + (y * gridSize));
				InitMazeGuard(spawnPos);
			}
		}
	}
}

void NCL::CSC8503::Coursework::InitBridgeOpener(const Vector3& position) {
	auto* target = AddCubeToWorld(position, Vector3(20, 20, 1), 0);
	target->SetGameObjectType(GameObjectType::BridgeOpener);
	target->SetCollisionCallback([&]() {
		InitBridge(Vector3(2.f, -19.f, -95.f));
	});
}
void NCL::CSC8503::Coursework::InitObjective(const Vector3& position) {
	auto* collectible = AddSphereToWorld(position, 1.f, 0.f);
	collectible->setLayer(Layer::Pickable);
	collectible->SetGameObjectType(GameObjectType::Objective);
}
void NCL::CSC8503::Coursework::TestPathFinding(){
	NavigationPath outPath;

	Vector3 startPos(mazeGuard->GetTransform().GetPosition());
	Vector3 endPos(player->GetTransform().GetPosition());

	bool found = worldGrid->FindPath(startPos, endPos, outPath);

	Vector3 pos;
	testNodes.clear();
	while (outPath.PopWaypoint(pos)) {
		testNodes.push_back(pos);
	}
}

void NCL::CSC8503::Coursework::DisplayPathFinding(){
	for (int i = 1; i < testNodes.size(); ++i) {
		Vector3 a = testNodes[i - 1];
		Vector3 b = testNodes[i];

		Debug::DrawLine(a, b, Vector4(0, 1, 0, 1));
	}
}

void NCL::CSC8503::Coursework::CreateBrickWall(Vector3& position, Vector3& brickDimensions, bool isHorizontal, int width, int height){
	Vector3 startPos = position;
	if (isHorizontal){
		startPos.x = startPos.x - (brickDimensions.x * width);
	}
	else	{
		startPos.z = startPos.z - (brickDimensions.z * width);
	}

	startPos.y = startPos.y + (brickDimensions.y * height);

	for (int i = 0; i < width; i++)
	{
		for (int j = 0; j < height; j++)
		{
			if (isHorizontal) {
				AddCubeToWorld(Vector3(startPos.x + (i * brickDimensions.x), startPos.y - (j * brickDimensions.y), startPos.z), brickDimensions);
			}
			else {
				AddCubeToWorld(Vector3(startPos.x, startPos.y - (j * brickDimensions.y), startPos.z + (i * brickDimensions.z)), brickDimensions);
			}
		}
	}
}

void NCL::CSC8503::Coursework::CollectObjective(GameObject* objective){
	score++;
	world->RemoveGameObject(objective);
	if (score == FINISH_SCORE)
	{
		//TODO(eren.degirmenci): end game.
	}
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
}

void NCL::CSC8503::Coursework::InitCamera() {
	world->GetMainCamera().SetNearPlane(0.1f);
	world->GetMainCamera().SetFarPlane(500.0f);
	world->GetMainCamera().SetPitch(-15.0f);
	world->GetMainCamera().SetYaw(315.0f);
	world->GetMainCamera().SetPosition(Vector3(-60, 0, 60));
	lockedObject = nullptr;
}

void NCL::CSC8503::Coursework::UpdateKeys() {
	NCL::CSC8503::TutorialGame::UpdateKeys();
}

void NCL::CSC8503::Coursework::InitWorld()
{
	world->ClearAndErase();
	physics->Clear();
	InitPlayer();
	InitGameExamples();
	InitDefaultFloor();
	InitBridgeOpener(Vector3(30.f, 0.f, -201.f));
	InitObjective(Vector3(0.f, -15.f, -235.f));
	InitWorldGrid();

	//TEST
	float startZ = 80.f;
	float startX = 85.f;
	for (int i = 0; i < 5; i++){

		auto* throwableTest = AddSphereToWorld(Vector3(startX, -15, startZ), 1.f);
		throwableTest->setLayer(Layer::Pickable);
		throwableTest->SetGameObjectType(GameObjectType::Throwable);
		startX -= 5.f;
	}
	//


}

void NCL::CSC8503::Coursework::HandleCameraLock(){
	if (lockedObject != nullptr) {
		Vector3 objPos = lockedObject->GetTransform().GetPosition();
		Vector3 camPos = objPos + lockedOffset;

		Quaternion quaternion = player->GetTransform().GetOrientation();
		float rotateVal = world->GetMainCamera().GetYaw();
		if (rotateVal != quaternion.y)
		{
			quaternion.y = rotateVal;
			//player->GetTransform().SetOrientation(quaternion);
		}

		Matrix4 temp = Matrix4::BuildViewMatrix(camPos, objPos, Vector3(0, 1, 0));

		Matrix4 modelMat = temp.Inverse();

		//TODO(eren.degirmenci): Poss tips for rotating meshes to camera direction.
		Quaternion q(modelMat);
		Vector3 angles = q.ToEuler(); //nearly there now!

		world->GetMainCamera().SetPosition(camPos);
	}
}

void NCL::CSC8503::Coursework::InitPlayer() {
	player = new Player(Vector3(60, -15, 60));
	player->SetGameObjectType(GameObjectType::Player);
	player->SetRenderObject(new RenderObject(&player->GetTransform(), enemyMesh, nullptr, basicShader));
	world->AddGameObject(player);
	LockCameraToObject(player);
}

void NCL::CSC8503::Coursework::InitMazeGuard(const Vector3& position) {
	mazeGuard = new Enemy(position, 5.f, EnemyType::MazeGuard, *player);
	mazeGuard->SetRenderObject(new RenderObject(&mazeGuard->GetTransform(), charMesh, nullptr, basicShader));
	world->AddGameObject(mazeGuard);
}

void NCL::CSC8503::Coursework::InitBridgeGuard(const Vector3& position) {
	bridgeGuard = new Enemy(position, 10.f, EnemyType::BridgeGuard, *player);
	mazeGuard->SetRenderObject(new RenderObject(&mazeGuard->GetTransform(), charMesh, nullptr, basicShader));
	world->AddGameObject(mazeGuard);
}

void NCL::CSC8503::Coursework::InitBridge(const Vector3& startPos)
{
	Vector3 cubeSize = Vector3(5, 1, 5);

	float invCubeMass = 5.f;
	int numLinks = 9;
	float maxDistance = 11.f;
	float cubeDistance = 10.f;

	GameObject* start = AddCubeToWorld(startPos + Vector3(0, 0, 0), cubeSize, 0);
	GameObject* end = AddCubeToWorld(startPos + Vector3(0, 0, -(numLinks + 2) * cubeDistance), cubeSize, 0);

	GameObject* previous = start;

	for (int i = 0; i < numLinks; ++i) {
		GameObject* block = AddCubeToWorld(startPos + Vector3((i + 1) * cubeDistance, 0, 0), cubeSize, invCubeMass);
		PositionConstraint* constraint = new PositionConstraint(previous, block, maxDistance);
		world->AddConstraint(constraint);
		previous = block;
	}

	PositionConstraint* constraint = new PositionConstraint(previous, end, maxDistance);
	world->AddConstraint(constraint);

}

void NCL::CSC8503::Coursework::ToggleCameraMode() {
	isCameraAttachedToPlayer = !isCameraAttachedToPlayer;
	if (!isCameraAttachedToPlayer) {
		LockCameraToObject(nullptr);
		world->GetMainCamera().SetFreeMode(true);
	}
	else {
		LockCameraToObject(player);
		world->GetMainCamera().SetFreeMode(false);
	}
}

void NCL::CSC8503::Coursework::HandleCameraModeControls() {
	const Keyboard& keyboard = *Window::GetKeyboard();

	if (keyboard.KeyPressed(KeyCodes::V)) {
		ToggleCameraMode();
	}
}