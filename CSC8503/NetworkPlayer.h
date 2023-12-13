#pragma once
#include "GameObject.h"
#include "GameClient.h"
#include "Player.h"

namespace NCL {
	namespace CSC8503 {
		class NetworkedGame;

		class NetworkPlayer : public Player {
		public:
			NetworkPlayer(NetworkedGame* game, int num, const Vector3& position);
			~NetworkPlayer();

			void OnCollisionBegin(GameObject* otherObject) override;
			void HandlePlayerControls(float dt, GameWorld& world) override;
			void SetClientInput(const Vector3& input);
			int GetPlayerNum() const {
				return playerNum;
			}

		protected:
			Vector3 clientInput;
			NetworkedGame* game;
			int playerNum;
		};
	}
}

