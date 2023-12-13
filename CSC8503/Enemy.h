#pragma once

#include "GameObject.h"
#include "Player.h"
#include "BehaviourNode.h"

class BehaviourSequence;
namespace NCL {
	namespace Maths {
		class Vector3;
	}
	namespace CSC8503 {

		enum EnemyType {
			MazeGuard,
			BridgeGuard
		};

		class Enemy : public GameObject {
		public:
			Enemy(const NCL::Maths::Vector3& position, float scale, EnemyType type, Player& player, float inverseMass = 10.f);
			~Enemy();

			void SetNodesToPlayer(std::vector<Vector3> nodes);
			void HandleEnemy(float dt, GameWorld& world);
		protected:
			
			EnemyType enemyType = EnemyType::MazeGuard;
			
			bool isChasingPlayer = false;
			bool isStunned = false;

			int currentNodeToGoIndex = 0;

			float stunTimer = 0.f;
			float behaviourTimer = 0.f;
			float distanceToTarget = 0.f;
			float moveSpeed = 5.f;
			
			BehaviourState currentState = BehaviourState::Ongoing;
			BehaviourSequence* rootSequence = nullptr;
			Player* player = nullptr;


			std::vector<Vector3> nodesToPlayer;

			void InitBehaviourTree();
			void InitMazeGuardSequnces();
			void ChasePlayer(float dt);
			void OnCollisionBegin(GameObject* otherObject) override;
		};
	}
}