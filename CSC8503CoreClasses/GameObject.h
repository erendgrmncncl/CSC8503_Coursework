#pragma once
#include "Transform.h"
#include "CollisionVolume.h"
#include "RenderObject.h"

using std::vector;

enum Layer {
	All,
	Pickable
};

enum GameObjectType {
	Static,
	Objective,
	Throwable,
	Player,
	BridgeOpener
};

namespace NCL::CSC8503 {
	class NetworkObject;
	class RenderObject;
	class PhysicsObject;
	class GameWorld;

	class GameObject	{
	public:

		GameObject(const std::string& name = "");
		~GameObject();

		void SetBoundingVolume(CollisionVolume* vol) {
			boundingVolume = vol;
		}

		const CollisionVolume* GetBoundingVolume() const {
			return boundingVolume;
		}

		bool IsActive() const {
			return isActive;
		}

		Transform& GetTransform() {
			return transform;
		}

		RenderObject* GetRenderObject() const {
			return renderObject;
		}

		PhysicsObject* GetPhysicsObject() const {
			return physicsObject;
		}

		NetworkObject* GetNetworkObject() const {
			return networkObject;
		}

		void SetNetworkObject(NetworkObject* object) {
			networkObject = object;
		}

		void SetRenderObject(RenderObject* newObject) {
			renderObject = newObject;
		}

		void SetPhysicsObject(PhysicsObject* newObject) {
			physicsObject = newObject;
		}

		const std::string& GetName() const {
			return name;
		}

		virtual void OnCollisionBegin(GameObject* otherObject) {
			if (this->type == GameObjectType::Throwable && otherObject->type == GameObjectType::BridgeOpener) {
				auto* renderer = otherObject->GetRenderObject();
				renderer->SetColour(Vector4(0, 0, 1, 1));
				if (otherObject->collisionCallback != nullptr) {
					otherObject->collisionCallback();
				}
			}
		}

		virtual void OnCollisionEnd(GameObject* otherObject) {
			//std::cout << "OnCollisionEnd event occured!\n";
		}

		bool GetBroadphaseAABB(Vector3&outsize) const;

		void UpdateBroadphaseAABB();

		void SetWorldID(int newID) {
			worldID = newID;
		}

		int		GetWorldID() const {
			return worldID;
		}

		Layer getLayer() const;
		void setLayer(Layer layerToSet);


		GameObjectType GetGameObjectType() const;
		void SetGameObjectType(GameObjectType type);

		void AttachToAnotherObj(GameObject& obj);

		void SetCollisionCallback(std::function<void()> callback);

		bool GetIsAffectedByGravity() const;
		void SetIsAffectedByGravity(bool isAffectedByGravity);

		bool GetIsInteractable() const;
		void SetIsInteractable(bool val);

		GameObject* getNextObjectInDirection(const GameWorld& world, Vector3 direction);

	protected:
		Layer				layer = Layer::All;
		GameObjectType      type = GameObjectType::Static;
		Transform			transform;

		CollisionVolume*	boundingVolume;
		PhysicsObject*		physicsObject;
		RenderObject*		renderObject;
		NetworkObject*		networkObject;

		std::function<void()> collisionCallback = nullptr;

		bool		isActive;
		bool        isAffectedByGravity = true;
		bool		isAttached = false;
		bool        isInteractable = true;
		int			worldID;
		std::string	name;

		Vector3 broadphaseAABB;
	};
}

