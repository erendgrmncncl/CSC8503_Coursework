#include "NetworkedGame.h"
#include "NetworkPlayer.h"
#include "NetworkObject.h"
#include "GameServer.h"
#include "GameClient.h"
#include "RenderObject.h"

#define COLLISION_MSG 30
namespace {
	constexpr const int MAX_PLAYER = 4;
}

struct MessagePacket : public GamePacket {
	short playerID;
	short messageID;

	MessagePacket() {
		type = Message;
		size = sizeof(short) * 2;
	}
};

NetworkedGame::NetworkedGame() : Coursework(true) {
	thisServer = nullptr;
	thisClient = nullptr;

	NetworkBase::Initialise();
	timeToNextPacket = 0.0f;
	packetsToSnapshot = 0;

	for (int i = 0; i < MAX_PLAYER; i++){
		playerList.push_back(-1);
	}
}

NetworkedGame::~NetworkedGame() {
	delete thisServer;
	delete thisClient;
}

void NetworkedGame::StartAsServer() {
	thisServer = new GameServer(NetworkBase::GetDefaultPort(), MAX_PLAYER);

	thisServer->RegisterPacketHandler(Received_State, this);
	thisServer->RegisterPacketHandler(String_Message, this);
	thisServer->RegisterPacketHandler(BasicNetworkMessages::ClientPlayerInput, this);
}

void NetworkedGame::StartAsClient(char a, char b, char c, char d) {
	thisClient = new GameClient();
	int peer = thisClient->Connect(a, b, c, d, NetworkBase::GetDefaultPort());

	thisClient->RegisterPacketHandler(Delta_State, this);
	thisClient->RegisterPacketHandler(Full_State, this);
	thisClient->RegisterPacketHandler(Player_Connected, this);
	thisClient->RegisterPacketHandler(Player_Disconnected, this);
	thisClient->RegisterPacketHandler(String_Message, this);
	thisClient->RegisterPacketHandler(Game_State, this);
	thisClient->RegisterPacketHandler(BasicNetworkMessages::SyncPlayers, this);
	thisClient->RegisterPacketHandler(BasicNetworkMessages::AddPlayerScore, this);
}

void NetworkedGame::UpdateGame(float dt) {
	timeToNextPacket -= dt;
	if (timeToNextPacket < 0) {
		if (thisServer) {
			UpdateAsServer(dt);
		}
		else if (thisClient) {
			UpdateAsClient(dt);
		}
		timeToNextPacket += 1.0f / 20.0f; //20hz server/client update

		if (thisServer)
		{
			SyncPlayerList();
		}
	}
	if (isGameStarted) {
		HandleHighScoreMenu();
		TutorialGame::UpdateGame(dt);
		for (auto const& serverPlayer : serverPlayers) {
			if (serverPlayer.second != nullptr) {
				serverPlayer.second->HandlePlayerControls(dt, *world);
			}
		}
	}
	else
	{
		if (thisServer)
		{
			Debug::Print(" Waiting for player to join ...", Vector2(5, 95), Debug::RED);
			if (Window::GetKeyboard()->KeyPressed(KeyCodes::S))
			{
				SetIsGameStarted(true);
			}
		}
		else
		{
			Debug::Print(" Waiting for server to start ...", Vector2(5, 95), Debug::RED);
		}
		renderer->Render();
	}
	if (thisServer)
	{
		thisServer->UpdateServer();
	}
	if (thisClient)
	{
		thisClient->UpdateClient();
	}
}

void NetworkedGame::UpdateAsServer(float dt) {
	packetsToSnapshot--;
	if (packetsToSnapshot < 0) {
		BroadcastSnapshot(false);
		packetsToSnapshot = 5;
	}
	else {
		BroadcastSnapshot(true);
	}
}

void NetworkedGame::UpdateAsClient(float dt) {
	thisClient->UpdateClient();
	ClientPacket newPacket;

	if (Window::GetKeyboard()->KeyPressed(KeyCodes::SPACE)) {
		//fire button pressed!
		newPacket.buttonstates[0] = 1;
		newPacket.lastID = 0; //You'll need to work this out somehow... 
	}
	thisClient->SendPacket(newPacket);
}

void NetworkedGame::BroadcastSnapshot(bool deltaFrame) {
	std::vector<GameObject*>::const_iterator first;
	std::vector<GameObject*>::const_iterator last;

	world->GetObjectIterators(first, last);

	for (auto i = first; i != last; ++i) {
		NetworkObject* o = (*i)->GetNetworkObject();
		if (!o) {
			continue;
		}
		//TODO - you'll need some way of determining
		//when a player has sent the server an acknowledgement
		//and store the lastID somewhere. A map between player
		//and an int could work, or it could be part of a 
		//NetworkPlayer struct. 
		int playerState = o->GetLatestNetworkState().stateID;
		GamePacket* newPacket = nullptr;
		if (o->WritePacket(&newPacket, deltaFrame, playerState)) {
			thisServer->SendGlobalPacket(*newPacket);
			delete newPacket;
		}
	}
}

void NetworkedGame::UpdateMinimumState() {
	//Periodically remove old data from the server
	int minID = INT_MAX;
	int maxID = 0; //we could use this to see if a player is lagging behind?

	for (auto i : stateIDs) {
		minID = std::min(minID, i.second);
		maxID = std::max(maxID, i.second);
	}
	//every client has acknowledged reaching at least state minID
	//so we can get rid of any old states!
	std::vector<GameObject*>::const_iterator first;
	std::vector<GameObject*>::const_iterator last;
	world->GetObjectIterators(first, last);

	for (auto i = first; i != last; ++i) {
		NetworkObject* o = (*i)->GetNetworkObject();
		if (!o) {
			continue;
		}
		o->UpdateStateHistory(minID); //clear out old states so they arent taking up memory...
	}
}

int NCL::CSC8503::NetworkedGame::GetPlayerPeerID(int peerId) {
	if (peerId == -2) {
		peerId = thisClient->GetPeerID();
	}
	for (int i = 1; i < 4; ++i){
		if (playerList[i] == peerId)
		{
			return i;
		}
	}
	return -1;
}

void NCL::CSC8503::NetworkedGame::SendGameStatusPacket() {
	GameStatePacket state(isGameStarted);
	thisServer->SendGlobalPacket(state);
}

void NCL::CSC8503::NetworkedGame::InitWorld() {
	world->ClearAndErase();
	physics->Clear();

	InitDefaultFloor();
	InitWorldGrid();
	for (int i = 0; i < 6; i++){
		InitObjective(Vector3(0 + (i * 3), -15, 10));
	}
	SpawnPlayers();
}

void NCL::CSC8503::NetworkedGame::HandleClientPlayerInput(ClientPlayerInputPacket* packet, int playerPeerID) {
	int playerIndex = GetPlayerPeerID(playerPeerID);
	auto* playerToHandle = serverPlayers[playerIndex];

	playerToHandle->SetClientInput(packet->movementVec, packet->isTriggeredActionButton, packet->rayPosition, packet->rayDirection);
}

void NetworkedGame::SpawnPlayers() {

	for (int i = 0; i < 4; i++)		{
		if (playerList[i] != -1) {
			auto* netPlayer = AddPlayerObject(Vector3(60 - (5 * i), -15, 60), i);
			serverPlayers.emplace(i, netPlayer);
		}
		else
		{
			serverPlayers.emplace(i, nullptr);
		}
	}
	if (thisServer) {
		localPlayer = serverPlayers[0];
	}
	else
	{
		localPlayer = serverPlayers[GetPlayerPeerID()];
	}

	localPlayer->GetRenderObject()->SetVisibility(false);

	player = (Player*)localPlayer;
	LockCameraToObject(localPlayer);
}

NCL::CSC8503::NetworkPlayer* NCL::CSC8503::NetworkedGame::AddPlayerObject(const Vector3& position, int playerNum) {
	auto* netPlayer = new NetworkPlayer(this, playerNum, position);
	netPlayer->SetGameObjectType(GameObjectType::Player);
	netPlayer->SetRenderObject(new RenderObject(&netPlayer->GetTransform(), enemyMesh, nullptr, basicShader));
	netPlayer->SetNetworkObject(new NetworkObject(*netPlayer, playerNum));
	
	networkObjects.push_back(netPlayer->GetNetworkObject());
	world->AddGameObject(netPlayer);
	Vector4 colour;
	switch (playerNum)
	{
	case 0:
		colour = Vector4(1, 0, 0, 1); // RED
		break;
	case 1:
		colour = Vector4(0, 1, 0, 1); //Green
		break;
	case 2:
		colour = Vector4(0, 0, 1, 1); //Blue
	case 3:
		colour = Vector4(1, 1, 0, 1); //Yellow
	default:
		break;
	}

	netPlayer->GetRenderObject()->SetColour(colour);
	return netPlayer;
}

void NCL::CSC8503::NetworkedGame::HandleFullPacket(NCL::CSC8503::FullPacket* fullPacket) {

	for (int i = 0; i < networkObjects.size(); i++) {
		if (networkObjects[i]->GetNetworkID() == fullPacket->objectID) {
			networkObjects[i]->ReadPacket(*fullPacket);
		}


	}
}

void NCL::CSC8503::NetworkedGame::HandleAddPlayerScorePacket(AddPlayerScorePacket* packet) {
	serverPlayers[GetPlayerPeerID(packet->playerId)]->SetScore(packet->score);
}

void NCL::CSC8503::NetworkedGame::SyncPlayerList(){
	
	int peerId;
	playerList[0] = 0;
	for (int i = 0; i < 3; ++i) {
		if (thisServer->GetPeer(i, peerId)) {
			playerList[i + 1] = peerId;
		}
		else{
			playerList[i + 1] = -1;
		}
	}

	SyncPlayerListPacket packet(playerList);
	thisServer->SendGlobalPacket(packet);
}

void NCL::CSC8503::NetworkedGame::InitObjective(const Vector3& position) {
	auto* collectible = AddSphereToWorld(position, 3.f, 0.f);
	collectible->setLayer(Layer::Pickable);
	collectible->SetGameObjectType(GameObjectType::Objective);
	collectible->GetRenderObject()->SetColour(Vector4(1, 0.5, 1, 1));
	auto* networkObj = new NetworkObject(*collectible, networkObjectCache);
	collectible->SetNetworkObject(networkObj);
	networkObjects.push_back(networkObj);
	networkObjectCache++;
}

void NetworkedGame::StartLevel() {
	InitWorld();
}

void NetworkedGame::ReceivePacket(int type, GamePacket* payload, int source) {
	switch (type)
{
		case BasicNetworkMessages::String_Message: {
			int a = 0;
			break;
		}
		case BasicNetworkMessages::Game_State: {
			GameStatePacket* packet = (GameStatePacket*)payload;
			SetIsGameStarted(packet->isGameStarted);
			break;
		}
		case BasicNetworkMessages::SyncPlayers: {
			SyncPlayerListPacket* packet = (SyncPlayerListPacket*)payload;
			packet->SyncPlayerList(playerList);
			break;
		}
		case BasicNetworkMessages::Full_State: {
			FullPacket* packet = (FullPacket*)payload;
			HandleFullPacket(packet); 
			break;
		}
		case BasicNetworkMessages::ClientPlayerInput: {
			if (thisServer == nullptr){
				return;
			}
			ClientPlayerInputPacket* packet = (ClientPlayerInputPacket*)payload;
			HandleClientPlayerInput(packet, source + 1);
			break;
		}
		case BasicNetworkMessages::AddPlayerScore: {
			AddPlayerScorePacket* packet = (AddPlayerScorePacket*)payload;
			HandleAddPlayerScorePacket(packet);
			break;
		}
	}
}

void NetworkedGame::OnPlayerCollision(NetworkPlayer* a, NetworkPlayer* b) {
	if (thisServer) { //detected a collision between players!
		MessagePacket newPacket;
		newPacket.messageID = COLLISION_MSG;
		newPacket.playerID = a->GetPlayerNum();

		thisServer->SendGlobalPacket(newPacket);

		newPacket.playerID = b->GetPlayerNum();
		thisServer->SendGlobalPacket(newPacket);
	}
}

bool NCL::CSC8503::NetworkedGame::GetIsGameStarted() const {
	return isGameStarted;
}

void NCL::CSC8503::NetworkedGame::SetIsGameStarted(bool isGameStarted) {
	this->isGameStarted = isGameStarted;
	if (thisServer) {
		SendGameStatusPacket();
	}
	if (isGameStarted){
		StartLevel();
	}
}

void NCL::CSC8503::NetworkedGame::HandleHighScoreMenu() {
	if (Window::GetKeyboard()->KeyDown(KeyCodes::TAB)) {
		float startY = 5;
		for (int i = 0; i < serverPlayers.size(); i++) {
			if (serverPlayers[i] == nullptr || serverPlayers[i]->GetPlayerNum() == -1)	{
				return;
			}
			std::string strToWrite = "Player " + std::to_string(i) + ": " + std::to_string(serverPlayers[i]->GetScore());
			Debug::Print(strToWrite, Vector2(10, startY));
			startY += 5;
		}
	}
}

void NCL::CSC8503::NetworkedGame::CollectObjective(GameObject* objective, int playerId){
	int playerScore = serverPlayers[GetPlayerPeerID(playerId)]->GetScore();
	int newScore = playerScore + 1;
	serverPlayers[GetPlayerPeerID(playerId)]->SetScore(newScore);
	world->RemoveGameObject(objective);

	AddPlayerScorePacket packet(playerId, newScore);
	thisServer->SendGlobalPacket(packet);
}

GameClient* NCL::CSC8503::NetworkedGame::GetClient() {
	return thisClient;
}

GameServer* NCL::CSC8503::NetworkedGame::GetServer() {
	return thisServer;
}
