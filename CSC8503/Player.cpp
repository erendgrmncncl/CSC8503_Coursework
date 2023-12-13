#include "Player.h"
#include "SphereVolume.h"
#include "RenderObject.h"
#include "PhysicsObject.h"
#include "Keyboard.h"
#include "Window.h"
#include "Ray.h"
#include "CollisionDetection.h"
#include "GameWorld.h"
#include "SceneManager.h"
#include "Coursework.h"
namespace {
	constexpr const float PLAYER_MESH_SIZE = 5.f;
	constexpr const Vector3 HELD_OBJECT_OFFSET = Vector3(0, 3, -2.5);
}

NCL::CSC8503::Player::Player(const Vector3& position, float inverseMass) {
	float meshSize = 3.0f;

	AABBVolume* volume = new AABBVolume(Vector3(0.3f, 0.9f, 0.3f) * meshSize);
	boundingVolume = (CollisionVolume*)volume;

	transform.SetScale(Vector3(PLAYER_MESH_SIZE, PLAYER_MESH_SIZE, PLAYER_MESH_SIZE))
		.SetPosition(position);

	physicsObject = new PhysicsObject(&transform, boundingVolume);

	physicsObject->SetInverseMass(inverseMass);
	physicsObject->InitSphereInertia();
}

void NCL::CSC8503::Player::HandlePlayerControls(float dt, GameWorld& world) {

	const Keyboard& keyboard = *Window::GetWindow()->GetKeyboard();
	const Mouse& mouse = *Window::GetMouse();
	HandleHeldObjObject();
	////rotate obj

	//auto camera = world.GetMainCamera();
	//auto& controller = camera.GetActiveController();
	//auto newOrientation = transform.GetOrientation();
	////Update the mouse by how much
	//float axisVal = controller.GetNamedAxis("XLook");
	//float xAxisVal = controller.GetNamedAxis("YLook");

	//float rotateValue = axisVal * dt * 100.f;
	//float xRotateValue = xAxisVal * dt * 100.f;

	//float newVal = newOrientation.y - rotateValue;	
	//float xNewVal = newOrientation.x - xRotateValue;
	//newOrientation.y = newVal;

	//transform.SetOrientation(newOrientation);

	//// rotate obj

	Vector3 newPos = transform.GetPosition();
	if (keyboard.KeyDown(NCL::KeyCodes::A)) {
		newPos.x -= 1 * speed * dt;

	}
	if (keyboard.KeyDown(NCL::KeyCodes::D)) {
		newPos.x += 1 * speed * dt;
	}

	if (keyboard.KeyDown(NCL::KeyCodes::W)) {
		newPos.z += -1 * speed * dt;
	}
	if (keyboard.KeyDown(NCL::KeyCodes::S)) {
		newPos.z += 1 * speed * dt;
	}
	transform.SetPosition(newPos);
	if (keyboard.KeyPressed(NCL::KeyCodes::E)) {
		PickUpObject(world);
	}
	if (heldObj != nullptr && mouse.ButtonDown(MouseButtons::Left)) {
		ThrowObject(world);
	}
}

void NCL::CSC8503::Player::PickUpObject(GameWorld& world) {

	Ray ray = CollisionDetection::BuidRayFromCenterOfTheCamera(world.GetMainCamera());

	RayCollision closestCollision;

	if (world.Raycast(ray, closestCollision, true, this, Layer::Pickable)) {
		//TODO(eren.degirmenci): check distance between player and object
		auto* collidedObj = (GameObject*)closestCollision.node;
		if (collidedObj != nullptr)
		{
			Vector3 objPos = collidedObj->GetTransform().GetPosition();
			Vector3 playerPos = transform.GetPosition();

			float distance = NCL::Maths::CalculateDistance(objPos, playerPos);

			if (distance > 10.f)
			{
				return;
			}
			heldObj = collidedObj;
			collidedObj->GetPhysicsObject()->ClearForces();
			collidedObj->SetIsAffectedByGravity(false);
			heldObj->GetRenderObject()->SetColour(Vector4(0, 1, 1, 1));

			if (heldObj->GetGameObjectType() == GameObjectType::Objective) {
				auto* gameScene = (Coursework*)SceneManager::GetSceneManager()->GetCurrentScene();
				if (gameScene != nullptr)
				{
					gameScene->CollectObjective(heldObj);
					heldObj = nullptr;
				}
			}
		}
		else
		{
			return;
		}

	}
}

void NCL::CSC8503::Player::HandleHeldObjObject() {
	if (heldObj != nullptr) {
		heldObj->GetTransform().SetPosition(transform.GetPosition() + HELD_OBJECT_OFFSET);
	}
}

void NCL::CSC8503::Player::ThrowObject(GameWorld& world) {
	Vector3 pos = CollisionDetection::GetCameraVec(world.GetMainCamera());
	float power = 500.f;

	heldObj->GetPhysicsObject()->AddForce(pos * power);
	heldObj->SetIsAffectedByGravity(true);
	heldObj = nullptr;
}
 