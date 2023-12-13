#include "NetworkPlayer.h"
#include "NetworkObject.h"
#include "NetworkedGame.h"

using namespace NCL;
using namespace CSC8503;

NetworkPlayer::NetworkPlayer(NetworkedGame* game, int num, const Vector3& position) : Player(position) {
	this->game = game;
	playerNum = num;
	clientInput = Vector3(0, 0, 0);
}

NetworkPlayer::~NetworkPlayer() {

}

void NetworkPlayer::OnCollisionBegin(GameObject* otherObject) {
	if (game) {
		if (dynamic_cast<NetworkPlayer*>(otherObject))
		{
			game->OnPlayerCollision(this, (NetworkPlayer*)otherObject);
		}
	}
}

void NCL::CSC8503::NetworkPlayer::HandlePlayerControls(float dt, GameWorld& world) {
	if (game->GetClient()) {
		const Keyboard& keyboard = *Window::GetWindow()->GetKeyboard();
		const Mouse& mouse = *Window::GetMouse();

		Vector3 newPos = Vector3(0, 0, 0);
		if (keyboard.KeyDown(NCL::KeyCodes::A)) {
			newPos.x -= 1;
		}
		if (keyboard.KeyDown(NCL::KeyCodes::D)) {
			newPos.x += 1;
		}
		if (keyboard.KeyDown(NCL::KeyCodes::W)) {
			newPos.z -= 1;
		}
		if (keyboard.KeyDown(NCL::KeyCodes::S)) {
			newPos.z += 1;
		}

		if (newPos != Vector3(0,0,0)) {
			ClientPlayerInputPacket packet(newPos);
			game->GetClient()->SendPacket(packet);
		}

	}
	else if (game->GetServer()) {
		if (playerNum == 0) { //Server Player
			Player::HandlePlayerControls(dt, world);
		}
		else if (clientInput != Vector3(0, 0, 0)) {
			Vector3 newPos = transform.GetPosition();
			newPos += clientInput * dt * speed;
			transform.SetPosition(newPos);
			clientInput = Vector3(0, 0, 0);
		}
	}
}

void NCL::CSC8503::NetworkPlayer::SetClientInput(const Vector3& input) {
	if (input != Vector3(0,0,0)) {
		clientInput = input;
	}
}
