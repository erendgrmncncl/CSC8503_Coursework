#include "GameObject.h"
#include "CollisionDetection.h"
#include "PhysicsObject.h"
#include "RenderObject.h"
#include "NetworkObject.h"
#include "../CSC8503CoreClasses/GameWorld.h"

using namespace NCL::CSC8503;

GameObject::GameObject(const std::string& objectName)	{
	name			= objectName;
	worldID			= -1;
	isActive		= true;
	boundingVolume	= nullptr;
	physicsObject	= nullptr;
	renderObject	= nullptr;
	networkObject	= nullptr;
	layer = Layer::All;
}

GameObject::~GameObject()	{
	delete boundingVolume;
	delete physicsObject;
	delete renderObject;
	delete networkObject;
}

bool GameObject::GetBroadphaseAABB(Vector3&outSize) const {
	if (!boundingVolume) {
		return false;
	}
	outSize = broadphaseAABB;
	return true;
}

void GameObject::UpdateBroadphaseAABB() {
	if (!boundingVolume) {
		return;
	}
	if (boundingVolume->type == VolumeType::AABB) {
		broadphaseAABB = ((AABBVolume&)*boundingVolume).GetHalfDimensions();
	}
	else if (boundingVolume->type == VolumeType::Sphere) {
		float r = ((SphereVolume&)*boundingVolume).GetRadius();
		broadphaseAABB = Vector3(r, r, r);
	}
	else if (boundingVolume->type == VolumeType::OBB) {
		Matrix3 mat = Matrix3(transform.GetOrientation());
		mat = mat.Absolute();
		Vector3 halfSizes = ((OBBVolume&)*boundingVolume).GetHalfDimensions();
		broadphaseAABB = mat * halfSizes;
	}
}

Layer NCL::CSC8503::GameObject::getLayer() const{
	return layer;
}

void NCL::CSC8503::GameObject::setLayer(Layer layerToSet){
	layer = layerToSet;
}

GameObject* NCL::CSC8503::GameObject::getNextObjectInDirection(const GameWorld& world, Vector3 direction){
	Vector3 rayDir = GetTransform().GetOrientation() * Vector3(0, 0, 1);
	Vector3 rayPos = GetTransform().GetPosition();
	
	RayCollision closestForwardOfSelectedObj;

	Ray closestRay(rayPos, rayDir);

	if (world.Raycast(closestRay, closestForwardOfSelectedObj, true, this, Layer::All)) {
		auto* closestForwardObjOfSelectedObj = (GameObject*)closestForwardOfSelectedObj.node;
		return closestForwardObjOfSelectedObj;
	}
}

GameObjectType NCL::CSC8503::GameObject::GetGameObjectType() const {
	return type;
}

void NCL::CSC8503::GameObject::SetGameObjectType(GameObjectType type) {
	this->type = type;
}

bool GameObject::GetIsAffectedByGravity() const {
	return isAffectedByGravity;
}

void GameObject::SetIsAffectedByGravity(bool isAffectedByGravity) {
	this->isAffectedByGravity = isAffectedByGravity;
}

void GameObject::SetCollisionCallback(std::function<void()> callback) {
	this->collisionCallback = callback;
}

void GameObject::SetIsInteractable(bool val) {
	this->isInteractable = val;
}

bool GameObject::GetIsInteractable() const {
	return isInteractable;
}