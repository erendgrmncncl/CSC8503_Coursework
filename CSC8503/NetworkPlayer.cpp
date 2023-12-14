#include "NetworkPlayer.h"
#include "NetworkObject.h"
#include "NetworkedGame.h"
#include "SceneManager.h"

using namespace NCL;
using namespace CSC8503;

NetworkPlayer::NetworkPlayer(NetworkedGame* game, int num, const Vector3& position) : Player(position) {
	this->game = game;
	playerNum = num;
	clientInput = Vector3(0, 0, 0);
	rayDirection = Vector3(0, 0, 0);
	rayPosition = Vector3(0, 0, 0);
	clientActionInput = false;
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

		bool isActionButtonPressed = false;
		Vector3 newPos = Vector3(0, 0, 0);
		Vector3 rayPos = Vector3(0, 0, 0);
		Vector3 rayDirection = Vector3(0, 0, 0);
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
		if (keyboard.KeyPressed(NCL::KeyCodes::E)) {
			isActionButtonPressed = true;
		}

		if (isActionButtonPressed) {
			Ray ray = CollisionDetection::BuidRayFromCenterOfTheCamera(world.GetMainCamera());
			rayPos = ray.GetPosition();
			rayDirection = ray.GetDirection();
		}

		if (newPos != Vector3(0, 0, 0) || isActionButtonPressed) {
			ClientPlayerInputPacket packet(newPos, isActionButtonPressed, rayPos, rayDirection);
			game->GetClient()->SendPacket(packet);
		}
	}
	else if (game->GetServer()) {
		if (playerNum == 0) { //Server Player
			Player::HandlePlayerControls(dt, world);
		}
		else {
			if (clientInput != Vector3(0, 0, 0)) {
				Vector3 newPos = transform.GetPosition();
				newPos += clientInput * dt * speed;
				transform.SetPosition(newPos);
				clientInput = Vector3(0, 0, 0);
			}
			if (clientActionInput && rayDirection != Vector3(0, 0, 0) && rayPosition != Vector3(0, 0, 0)) {
				PickUpObject(world, rayPosition, rayDirection);
				clientActionInput = false;
				rayPosition = Vector3(0, 0, 0);
				rayDirection = Vector3(0, 0, 0);
			}
		}
	}
}

void NCL::CSC8503::NetworkPlayer::SetClientInput(const Vector3& input, bool actionInput, Vector3 rayPos, Vector3 rayDir) {
	if (input != Vector3(0, 0, 0)) {
		clientInput = input;
	}
	clientActionInput = actionInput;
	rayPosition = rayPos;
	rayDirection = rayDir;
}

int NCL::CSC8503::NetworkPlayer::GetScore() const
{
	return playerScore;
}

void NCL::CSC8503::NetworkPlayer::SetScore(int score){
	this->playerScore = score;
}

void NCL::CSC8503::NetworkPlayer::PickUpObject(GameWorld& world, Vector3 rayPosition, Vector3 rayDirection) {
	Ray ray(rayPosition, rayDirection);
	RayCollision closestCollision;

	if (world.Raycast(ray, closestCollision, true, this, Layer::Pickable)) {
		//TODO(eren.degirmenci): check distance between player and object
		auto* collidedObj = (GameObject*)closestCollision.node;
		if (collidedObj != nullptr)
		{
			if (collidedObj->GetGameObjectType() == GameObjectType::Objective) {
				auto* gameScene = (NetworkedGame*)SceneManager::GetSceneManager()->GetCurrentScene();
				if (gameScene != nullptr) {
					gameScene->CollectObjective(collidedObj, playerNum);

				}
			}
		}
		else
		{
			return;
		}

	}
}
