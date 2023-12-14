#include "Coursework.h"
#include "Player.h"
#include "RenderObject.h"
#include "PhysicsObject.h"
#include "NavigationGrid.h"
#include "PositionConstraint.h"
#include "OrientationConstraint.h"
#include "Enemy.h"
#include "State.h"
#include "StateTransition.h"

namespace {
	constexpr const char OBJECTIVE_TYPE_NODE = 'o';
	constexpr const char OBSTACLE_TYPE_NODE = 'x';
	constexpr const int FINISH_SCORE = 2;

	constexpr const float PLAYER_START_POS_X = 80;
	constexpr const float PLAYER_START_POS_Z = 78;

	constexpr const float PLAYER_ZONE_THRESHOLD = 3.f;
}

NCL::CSC8503::Coursework::Coursework(bool isNetwork) : TutorialGame(false) {
	InitialiseAssets();
	world->GetMainCamera().SetFreeMode(false);
	isNetworkGame = isNetwork;
	isGameEnded = false;
	if (!isNetwork) {
		InitCamera();
		InitWorld();
	}
}

NCL::CSC8503::Coursework::~Coursework() {

}

void NCL::CSC8503::Coursework::UpdateGame(float dt)
{
	if (Window::GetKeyboard()->KeyPressed(KeyCodes::R)) {
		RestartGame();
	}
	//TODO(eren.deg)
	if (score >= FINISH_SCORE && CheckIsPlayerInStartingArea()) {
		auto* window = Window::GetWindow();
		Vector2i screenSizeVec = window->GetScreenSize();
		Debug::Print("YOU WIN! MISCHIEF MANAGED! ", Vector2(50,50));
		Debug::Print("Press R To restart, ESC to return main menu ", Vector2(50,65));
		renderer->Render();
		return;
	}
	if (isGameEnded) {
		auto* window = Window::GetWindow();
		Vector2i screenSizeVec = window->GetScreenSize();
		Debug::Print("YOU LOST!", Vector2(50,50));
		Debug::Print("Press R To restart, ESC to return main menu ", Vector2(65, 10));
		renderer->Render();
		return;
	}

	TutorialGame::UpdateGame(dt);
	Debug::Print("Score: " + std::to_string(score) + "/" + std::to_string(FINISH_SCORE), Vector2(10, 65));
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
	if (traps.size() != 0 ){
		for (auto* trap : traps) {
			if (trap != nullptr) {
				trap->Update(dt);
			}
		}
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
				Vector3 spawnPos = Vector3(startPosX + (x * gridSize), -15.f, startPosZ + (y * gridSize));
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
void NCL::CSC8503::Coursework::SpawnThrowable(const Vector3& position) {
	auto* throwable = AddSphereToWorld(position, 1.5f);
	throwable->setLayer(Layer::Pickable);
	throwable->SetGameObjectType(GameObjectType::Throwable);
	throwable->GetRenderObject()->SetColour(Vector4(1,1,0,1));
}
void NCL::CSC8503::Coursework::InitObjective(const Vector3& position) {
	auto* collectible = AddSphereToWorld(position, 3.f, 0.f);
	collectible->setLayer(Layer::Pickable);
	collectible->SetGameObjectType(GameObjectType::Objective);
	collectible->GetRenderObject()->SetColour(Vector4(1, 0.5, 1, 1));
}
StateGameObject* NCL::CSC8503::Coursework::SpawnTrap(const Vector3& position) {

	std::vector<State*> states;
	std::vector<StateTransition*> stateTransitions;
	StateGameObject* stateObj = new StateGameObject(false);

	Vector3 dimensions = Vector3(10, 8, 4);
	OBBVolume* volume = new OBBVolume(dimensions);
	stateObj->SetBoundingVolume((CollisionVolume*)volume);

	stateObj->GetTransform()
		.SetScale(dimensions)
		.SetPosition(position);

	stateObj->SetRenderObject(new RenderObject(&stateObj->GetTransform(), cubeMesh, basicTex, basicShader));
	stateObj->SetPhysicsObject(new PhysicsObject(&stateObj->GetTransform(), stateObj->GetBoundingVolume()));

	stateObj->GetPhysicsObject()->SetInverseMass(1.f);
	stateObj->SetIsAffectedByGravity(false);
	stateObj->GetPhysicsObject()->InitSphereInertia();

	world->AddGameObject(stateObj);

	State* stateA = new State([&](float dt, StateGameObject* object)-> void {
		object->GetPhysicsObject()->AddForce({ -10, 0,0 });
	});

	State* stateB = new State([&](float dt, StateGameObject* object)->void {
		object->GetPhysicsObject()->AddForce({ 10, 0,0 });
	});

	states.push_back(stateA);
	states.push_back(stateB);

	StateTransition* stateTransitionAB = new StateTransition(stateA, stateB, [&]()-> bool {
		return stateObj->GetTransform().GetPosition().x >= 50.f;
	});

	StateTransition* stateTransitionBA = new StateTransition(stateB, stateA, [&]()->bool {
		return stateObj->GetTransform().GetPosition().x <= -50.f;
	});

	stateTransitions.push_back(stateTransitionAB);
	stateTransitions.push_back(stateTransitionBA);


	stateObj->AddStates(states);
	stateObj->AddStateTransitions(stateTransitions);

	return stateObj;
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

void NCL::CSC8503::Coursework::RestartGame() {
	score = 0;
	isGameEnded = false;
	InitWorld();
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
}

void NCL::CSC8503::Coursework::SetIsGameEnded(bool state){
	isGameEnded = state;
}

bool NCL::CSC8503::Coursework::CheckIsPlayerInStartingArea() {
	Vector3 playerPos = player->GetTransform().GetPosition();
	bool isPlayerXIsInStartingArea = playerPos.x <= PLAYER_START_POS_X + PLAYER_ZONE_THRESHOLD && playerPos.x >= PLAYER_START_POS_X - PLAYER_ZONE_THRESHOLD;
	bool isPlayerZIsInStartingArea = playerPos.z <= PLAYER_START_POS_Z + PLAYER_ZONE_THRESHOLD && playerPos.z >= PLAYER_START_POS_Z - PLAYER_ZONE_THRESHOLD;
	if (isPlayerXIsInStartingArea && isPlayerZIsInStartingArea ){
		return true;
	}

	return false;
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
	InitDefaultFloor();
	InitPlayer();
	InitGameExamples();

	InitBridgeOpener(Vector3(30.f, 0.f, -201.f));

	//Second Objective
	InitObjective(Vector3(0.f, -15.f, -235.f));

	//auto* trap = SpawnTrap(Vector3(0, -15, -75));
	//traps.push_back(trap);
	
	InitWorldGrid();


	//Throwables
	float startZ = 80.f;
	float startX = 85.f;
	for (int i = 0; i < 10; i++){
		SpawnThrowable(Vector3(startX, -15, startZ));
		startX -= 5.f;
		if (i == 4){
			startX = -10.f;
			startZ = -90.f;
		}
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
	player = new Player(Vector3(PLAYER_START_POS_X, -15, PLAYER_START_POS_Z));
	player->SetGameObjectType(GameObjectType::Player);
	player->SetRenderObject(new RenderObject(&player->GetTransform(), enemyMesh, nullptr, basicShader));
	player->GetRenderObject()->SetVisibility(false);
	world->AddGameObject(player);
	LockCameraToObject(player);
}

void NCL::CSC8503::Coursework::InitMazeGuard(const Vector3& position) {
	mazeGuard = new Enemy(position, 5.f, EnemyType::MazeGuard, *player, 0);
	mazeGuard->SetRenderObject(new RenderObject(&mazeGuard->GetTransform(), charMesh, nullptr, basicShader));
	mazeGuard->GetRenderObject()->SetColour(Vector4(1, 0, 0, 1));
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