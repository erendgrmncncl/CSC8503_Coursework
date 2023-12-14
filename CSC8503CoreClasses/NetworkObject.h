#pragma once
#include "GameObject.h"
#include "NetworkBase.h"
#include "NetworkState.h"
#include "Ray.h"

namespace NCL::CSC8503 {
	class GameObject;

	struct FullPacket : public GamePacket {
		int		objectID = -1;
		NetworkState fullState;

		FullPacket() {
			type = Full_State;
			size = sizeof(FullPacket) - sizeof(GamePacket);
		}
	};

	struct DeltaPacket : public GamePacket {
		int		fullID		= -1;
		int		objectID	= -1;
		char	pos[3];
		char	orientation[4];

		DeltaPacket() {
			type = Delta_State;
			size = sizeof(DeltaPacket) - sizeof(GamePacket);
		}
	};

	struct ClientPacket : public GamePacket {
		int		lastID;
		char	buttonstates[8];

		ClientPacket() {
			size = sizeof(ClientPacket);
		}
	};

	struct GameStatePacket : public GamePacket {
		bool isGameStarted = false;
		GameStatePacket(bool val) {
			type = Game_State;
			size = sizeof(GameStatePacket);

			isGameStarted = val;
		}
	};

	struct SyncPlayerListPacket : public GamePacket {
		int playerList[4];
		SyncPlayerListPacket(std::vector<int>& serverPlayers) {
			type = SyncPlayers;
			size = sizeof(SyncPlayerListPacket);

			for (int i = 0; i < 4; i++) {
				playerList[i] = serverPlayers[i];
			}
		}

		void SyncPlayerList(std::vector<int>& clientPlayerList) {
			for (int i = 0; i < 4; ++i) {
				clientPlayerList[i] = playerList[i];
			}
		}
	};

	struct ClientPlayerInputPacket : public GamePacket {
		Vector3 movementVec;
		bool isTriggeredActionButton;
		Vector3 rayPosition;
		Vector3 rayDirection;
		ClientPlayerInputPacket(Vector3 vec, bool isActionKeyPressed, Vector3 rayPos, Vector3 rayDirection) {
			type = ClientPlayerInput;
			size = sizeof(ClientPlayerInputPacket);

			movementVec = vec;
			isTriggeredActionButton = isActionKeyPressed;
			this->rayPosition = rayPos;
			this->rayDirection = rayDirection;
		}
	};

	struct AddPlayerScorePacket : public GamePacket {
		int playerId;
		int score;

		AddPlayerScorePacket(int playerId, int score) {
			type = AddPlayerScore;
			size = sizeof(AddPlayerScorePacket);

			this->playerId = playerId;
			this->score = score;
		}
	};

	class NetworkObject		{
	public:
		NetworkObject(GameObject& o, int id);
		virtual ~NetworkObject();

		//Called by clients
		virtual bool ReadPacket(GamePacket& p);
		//Called by servers
		virtual bool WritePacket(GamePacket** p, bool deltaFrame, int stateID);

		int GetNetworkID() const;

		void UpdateStateHistory(int minID);
		NetworkState& GetLatestNetworkState();

	protected:

		bool GetNetworkState(int frameID, NetworkState& state);

		virtual bool ReadDeltaPacket(DeltaPacket &p);
		virtual bool ReadFullPacket(FullPacket &p);

		virtual bool WriteDeltaPacket(GamePacket**p, int stateID);
		virtual bool WriteFullPacket(GamePacket**p);

		GameObject& object;

		NetworkState lastFullState;

		std::vector<NetworkState> stateHistory;

		int deltaErrors;
		int fullErrors;

		int networkID;
	};
}