#include "Enemy.h"
#include "BehaviourAction.h"
#include "BehaviourSequence.h"
#include "BehaviourSelector.h"
#include "CollisionDetection.h"
#include "PhysicsObject.h"
#include "Debug.h"
#include "Window.h"
#include "SceneManager.h"
#include "Coursework.h"
#include <iostream>

namespace {
	constexpr const float ENEMY_MESH_SIZE = 1.f;
	constexpr const float STUN_TIME = 5.f;
}
NCL::CSC8503::Enemy::Enemy(const NCL::Maths::Vector3& position, float scale, EnemyType type, Player& player, float inverseMass) {

	enemyType = type;
	this->player = &player;

	SphereVolume* volume = new SphereVolume(scale);
	boundingVolume = (CollisionVolume*)volume;

	transform.SetScale(Vector3(scale, scale, scale))
		.SetPosition(position);

	physicsObject = new PhysicsObject(&transform, boundingVolume);

	physicsObject->SetInverseMass(inverseMass);
	physicsObject->InitSphereInertia();

	if (enemyType == EnemyType::MazeGuard) {
		InitMazeGuardSequnces();
	}
	else if (enemyType == EnemyType::BridgeGuard) {

	}
}
NCL::CSC8503::Enemy::~Enemy() {

}
void NCL::CSC8503::Enemy::InitBehaviourTree() {
	BehaviourAction* findKey = new BehaviourAction("Find Key", [&](float dt, BehaviourState state)->BehaviourState {
		if (state == BehaviourState::Initialise) {
			std::cout << "Looking for a key !" << std::endl;
			behaviourTimer = rand() % 100;
			state = BehaviourState::Ongoing;
		}
		else if (state == BehaviourState::Ongoing) {
			behaviourTimer -= dt;
			if (behaviourTimer <= 0.f) {
				std::cout << "Found a key !" << std::endl;
				return BehaviourState::Success;
			}
		}

		return state; // will be ongoing until success.
		});

	BehaviourAction* goToRoom = new BehaviourAction("Go To Room", [&](float dt, BehaviourState state)->BehaviourState {
		if (state == BehaviourState::Initialise) {
			std::cout << "Going to the loot room !" << std::endl;
			state = BehaviourState::Ongoing;
		}
		else if (state == BehaviourState::Ongoing) {
			distanceToTarget -= dt;
			if (distanceToTarget <= 0.0f) {
				std::cout << "Reached to the room !" << std::endl;
				return BehaviourState::Success;
			}
		}

		return state; // will be ongoing until success.
		});

	BehaviourAction* openDoor = new BehaviourAction("Open Door", [&](float dt, BehaviourState state)-> BehaviourState {

		if (state == BehaviourState::Ongoing)
		{
			std::cout << "Opening Door!" << std::endl;
			return BehaviourState::Success;
		}
		return state;
		});

	BehaviourAction* lookForTreasure = new BehaviourAction("Look For Treasure", [&](float dt, BehaviourState state)->BehaviourState {
		if (state == BehaviourState::Initialise) {
			std::cout << "Looking for treasure!" << std::endl;
			return BehaviourState::Ongoing;
		}
		else if (state == BehaviourState::Ongoing) {
			bool found = rand() % 2;
			if (found) {
				std::cout << "I found some treasure !\n";
				return BehaviourState::Success;
			}
			std::cout << "No treasure here..." << std::endl;
			return BehaviourState::Failure;
		}
		return state;
		});

	BehaviourAction* lookForItems = new BehaviourAction("Look For Items", [&](float dt, BehaviourState state)->BehaviourState {
		if (state == BehaviourState::Initialise) {
			std::cout << "Looking for items!" << std::endl;
			return BehaviourState::Ongoing;
		}
		else if (state == BehaviourState::Ongoing)
		{
			bool found = rand() % 2;
			if (found) {
				std::cout << "I found some items!" << std::endl;
				return BehaviourState::Success;
			}

			std::cout << "No items in here..." << std::endl;
			return BehaviourState::Failure;
		}
		return state;
		});


	BehaviourSequence* sequence = new BehaviourSequence("Room Sequence");
	sequence->AddChild(findKey);
	sequence->AddChild(goToRoom);
	sequence->AddChild(openDoor);

	BehaviourSelector* selection = new BehaviourSelector("Loot Selection");
	selection->AddChild(lookForTreasure);
	selection->AddChild(lookForItems);

	BehaviourSequence* rootSequence = new BehaviourSequence("Root Sequence");
	rootSequence->AddChild(sequence);
	rootSequence->AddChild(selection);

	//Test

	for (int i = 0; i < 5; i++) {
		rootSequence->Reset();
		behaviourTimer = 0.0f;
		distanceToTarget = rand() % 250;

		BehaviourState state = BehaviourState::Ongoing;
		std::cout << "We're going on an adventure! " << std::endl;
		while (state == BehaviourState::Ongoing) {
			state = rootSequence->Execute(1.0f); //fake dt.
		}
		if (state == BehaviourState::Success) {
			std::cout << "What a successful adventure!" << std::endl;
		}
		else if (state == BehaviourState::Failure) {
			std::cout << "What a waste of time! " << std::endl;
		}
	}

	std::cout << "All done!" << std::endl;
}

void NCL::CSC8503::Enemy::InitMazeGuardSequnces() {	
	BehaviourAction* chasePlayer = new BehaviourAction("Chase Player", [&](float dt, BehaviourState state)->BehaviourState {
		if (state == BehaviourState::Initialise) {
			isChasingPlayer = true;
			state = BehaviourState::Ongoing;
		}
		else if (state == BehaviourState::Ongoing) {
			if (nodesToPlayer.size() != 0 && !isStunned && player != nullptr) {
				ChasePlayer(dt);
				Vector3 targetPos = player->GetTransform().GetPosition();
				Vector3 currentPos = transform.GetPosition();
				if (isChasingPlayer == false) {
					currentNodeToGoIndex = 0;
					return BehaviourState::Success;
				}
			}
		}

		return state;
	});

	BehaviourSequence* sequence = new BehaviourSequence("Maze Guard Sequence");
	sequence->AddChild(chasePlayer);

	rootSequence = new BehaviourSequence("Root Sequence");
	rootSequence->AddChild(sequence);
}

void NCL::CSC8503::Enemy::ChasePlayer(float dt) {
	auto window = Window::GetWindow();
	auto screenSize = window->GetScreenSize();
	Debug::Print("Started Chasing Player!", Vector2(screenSize.x * 0.1f, screenSize.y * .8f), Vector4(1, 1, 1, 1));
	Vector3 position = transform.GetPosition();
	Vector3 targetPos = nodesToPlayer[currentNodeToGoIndex];	
	float yValue = position.y;
	targetPos.y = yValue;
	Vector3 newPos = Vector3::MoveTowards(position, targetPos, moveSpeed * dt);
	if (newPos == targetPos && currentNodeToGoIndex < nodesToPlayer.size() -1) {
		currentNodeToGoIndex++;
	}
	transform.SetPosition(newPos);
}

void NCL::CSC8503::Enemy::OnCollisionBegin(GameObject* otherObject){
	if (otherObject->GetGameObjectType() == GameObjectType::Player && !isStunned ){
		isChasingPlayer = false;
		auto* gameScene = (Coursework*)SceneManager::GetSceneManager()->GetCurrentScene();
		if (gameScene != nullptr){
			gameScene->SetIsGameEnded(true);
		}
	}
	else if (otherObject->GetGameObjectType() == GameObjectType::Throwable) {
		isStunned = true;
		stunTimer = 0.f;
	}
}

void NCL::CSC8503::Enemy::SetNodesToPlayer(std::vector<Vector3> nodes) {
	for (int i = 0; i < nodes.size(); i++) {
		if ( i < nodesToPlayer.size() &&  nodesToPlayer[i] != nodes[i] && i < currentNodeToGoIndex) {
			currentNodeToGoIndex = i;
		}
	}
	this->nodesToPlayer = nodes;
	
}

void NCL::CSC8503::Enemy::HandleEnemy(float dt, GameWorld& world) {
	if (rootSequence != nullptr) {
		currentState = rootSequence->Execute(dt);
	}
	
	if (isStunned) {

		Debug::Print("Enemy is stunned! ", Vector2(65, 20), Vector4(1,0,0,1));
		stunTimer += dt;
		if (stunTimer >= STUN_TIME) {
			isStunned = false;
		}
	}
	else{

		Debug::Print("Enemy is chasing! ", Vector2(65, 20), Vector4(1, 0, 0, 1));
	}
}
