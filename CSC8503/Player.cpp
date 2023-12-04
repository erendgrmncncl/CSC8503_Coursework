#include "Player.h"
#include "SphereVolume.h"
#include "RenderObject.h"
#include "PhysicsObject.h"
#include "Keyboard.h"
#include "Window.h"
#include "Ray.h"
#include "CollisionDetection.h"
#include "GameWorld.h"
namespace {
	constexpr const float PLAYER_MESH_SIZE = 5.f;
}


NCL::CSC8503::Player::Player(const Vector3& position, float inverseMass) {
	SphereVolume* volume = new SphereVolume(5.f);

	boundingVolume = (CollisionVolume*)volume;

	transform.SetScale(Vector3(PLAYER_MESH_SIZE, PLAYER_MESH_SIZE, PLAYER_MESH_SIZE))
		     .SetPosition(position);

	physicsObject = new PhysicsObject(&transform, boundingVolume);

	physicsObject->SetInverseMass(inverseMass);
	physicsObject->InitSphereInertia();
}

void NCL::CSC8503::Player::HandlePlayerControls(float dt, GameWorld& world){

	const Keyboard& keyboard = *Window::GetWindow()->GetKeyboard();

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
	if (keyboard.KeyDown(NCL::KeyCodes::A)) {

	}
	if (keyboard.KeyDown(NCL::KeyCodes::D)) {
	}

	if (keyboard.KeyDown(NCL::KeyCodes::W)) {
		Vector3 newPos = transform.GetPosition();
		newPos.z += -1 * speed * dt;
		transform.SetPosition(newPos);
	}
	if (keyboard.KeyDown(NCL::KeyCodes::S)) {

	}
	if (keyboard.KeyPressed(NCL::KeyCodes::E))	{
		PickUpObject(world);
	}
}

void NCL::CSC8503::Player::PickUpObject(GameWorld& world) {

	Ray ray = CollisionDetection::BuidRayFromCenterOfTheCamera(world.GetMainCamera());

	RayCollision closestCollision;

	if (world.Raycast(ray, closestCollision, true, nullptr, Layer::All)) {
		//TODO(eren.degirmenci): check distance between player and object
		
		heldObj = (GameObject*)closestCollision.node;
		heldObj->GetRenderObject()->SetColour(Vector4(0, 1, 1, 1));
		//heldObj->GetTransform().SetPosition(transform.GetPosition());
	}
}
