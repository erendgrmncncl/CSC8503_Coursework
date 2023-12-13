#pragma once
#include "Coursework.h"
#include "NetworkBase.h"

namespace NCL {
	namespace CSC8503 {
		class GameServer;
		class GameClient;
		class NetworkPlayer;
		class Player;
		
		struct FullPacket;
		struct ClientPlayerInputPacket;

		class NetworkedGame : public Coursework, public PacketReceiver {
		public:
			NetworkedGame();
			~NetworkedGame();

			void StartAsServer();
			void StartAsClient(char a, char b, char c, char d);

			void UpdateGame(float dt) override;

			void StartLevel();

			void ReceivePacket(int type, GamePacket* payload, int source) override;

			void OnPlayerCollision(NetworkPlayer* a, NetworkPlayer* b);

			bool GetIsGameStarted() const;
			void SetIsGameStarted(bool isGameStarted);

			GameClient* GetClient();
			GameServer* GetServer();
		protected:

			bool isClientConnectedToServer = false;
			bool isGameStarted = false;

			void UpdateAsServer(float dt);
			void UpdateAsClient(float dt);

			void BroadcastSnapshot(bool deltaFrame);
			void UpdateMinimumState();
			int GetPlayerPeerID(int peerId = -2);

			void SendGameStatusPacket();
			void InitWorld() override;

			void HandleClientPlayerInput(ClientPlayerInputPacket* playerMovementPacket, int playerPeerID);

			void SpawnPlayers();
			NetworkPlayer* AddPlayerObject(const Vector3& position, int playerNum);

			void HandleFullPacket(FullPacket* fullPacket );

			void SyncPlayerList();

			std::map<int, int> stateIDs;

			GameServer* thisServer;
			GameClient* thisClient;
			float timeToNextPacket;
			int packetsToSnapshot;

			std::vector<NetworkObject*> networkObjects;

			std::vector<int> playerList;
			std::map<int, NetworkPlayer*> serverPlayers;
			GameObject* localPlayer;
		};
	}
}

