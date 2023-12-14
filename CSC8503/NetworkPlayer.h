#pragma once
#include "GameObject.h"
#include "GameClient.h"
#include "Player.h"
#include "Ray.h"

namespace NCL {
	namespace CSC8503 {
		class NetworkedGame;

		class NetworkPlayer : public Player {
		public:
			NetworkPlayer(NetworkedGame* game, int num, const Vector3& position);
			~NetworkPlayer();

			void OnCollisionBegin(GameObject* otherObject) override;
			void HandlePlayerControls(float dt, GameWorld& world) override;
			void SetClientInput(const Vector3& input, bool clientActionButtonPressed, Vector3 rayPos, Vector3 rayDir);
			int GetPlayerNum() const {
				return playerNum;
			}

			int GetScore() const;
			void SetScore(int score);

		protected:
			Vector3 clientInput;
			bool clientActionInput = false;
			Vector3 rayPosition;
			Vector3 rayDirection;

			NetworkedGame* game;
			int playerNum;

			int playerScore = 0;
			
			void PickUpObject(GameWorld& world, Vector3 rayPosition, Vector3 rayDirection);
		};
	}
}

