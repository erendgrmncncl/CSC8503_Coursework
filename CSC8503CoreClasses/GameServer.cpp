#include "GameServer.h"
#include "GameWorld.h"
#include "./enet/enet.h"
using namespace NCL;
using namespace CSC8503;

GameServer::GameServer(int onPort, int maxClients) {
	port = onPort;
	clientMax = maxClients;
	clientCount = 0;
	netHandle = nullptr;
	peers = new int[clientMax];
	for (int i = 0; i < clientMax; ++i){
		peers[i] = -1;
	}
	Initialise();
}

GameServer::~GameServer() {
	Shutdown();
}

void GameServer::Shutdown() {
	SendGlobalPacket(BasicNetworkMessages::Shutdown);
	enet_host_destroy(netHandle);
	netHandle = nullptr;
}

bool GameServer::Initialise() {
	ENetAddress address;
	address.host = ENET_HOST_ANY;
	address.port = port;

	netHandle = enet_host_create(&address, clientMax, 1, 0, 0);
	if (!netHandle) {
		std::cout << __FUNCTION__ << " failed to create network handle!" << std::endl;
		return false;
	}

	return true;
}

void NCL::CSC8503::GameServer::AddPeer(int peerNumber) {
	int emptyIndex = clientMax;
	for (int i = 0; i < clientMax; i++) {
		if (peers[i] == peerNumber){
			return;
		}
		if (peers[i] == -1) {
			emptyIndex = std::min(i, emptyIndex);
		}
	}
	if (emptyIndex < clientMax){
		peers[emptyIndex] = peerNumber;
	}
}

bool NCL::CSC8503::GameServer::GetPeer(int peerNumber, int& peerId){
	if (peerNumber >= clientMax)
		return false;
	if (peers[peerNumber] == -1) {
		return false;
	}
	peerId = peers[peerNumber];
	return true;
}

bool GameServer::SendGlobalPacket(int msgID) {
	GamePacket packet;
	packet.type = msgID;
	return SendGlobalPacket(packet);
}

bool GameServer::SendGlobalPacket(GamePacket& packet) {
	ENetPacket* dataPacket = enet_packet_create(&packet, packet.GetTotalSize(), 0);
	enet_host_broadcast(netHandle, 0, dataPacket);
	return true;
}

void GameServer::UpdateServer() {
	if (!netHandle) {
		return;
	}
	ENetEvent event;
	while (enet_host_service(netHandle, &event, 0) > 0) {
		int type = event.type;
		ENetPeer* p = event.peer;
		int peer = p->incomingPeerID;

		if (type == ENetEventType::ENET_EVENT_TYPE_CONNECT) {
			std::cout << "Server: A client has been connected";
			AddPeer(peer + 1);
		}
		else if (type == ENetEventType::ENET_EVENT_TYPE_DISCONNECT) {
			std::cout << "Server: A client has disconnected" << std::endl;
			for (int i = 0; i < 3; ++i){
				if (peers[i] == peer+1) {
					peers[i] = -1;
				}
			}
		}
		else if (type == ENetEventType::ENET_EVENT_TYPE_RECEIVE) {
			GamePacket* packet = (GamePacket*)event.packet->data;
			ProcessPacket(packet, peer);
		}
		enet_packet_destroy(event.packet);
	}
}

void GameServer::SetGameWorld(GameWorld& g) {
	gameWorld = &g;
}