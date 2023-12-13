#pragma once

#include "GameObject.h"

namespace NCL {
	namespace CSC8503 {
		class Player : public GameObject {
		public:
			Player(const Vector3& position, float inverseMass = 100.f);

			virtual void HandlePlayerControls(float dt, GameWorld& world);
		protected:
			float speed = 40.f;
			GameObject* heldObj = nullptr;

			void PickUpObject(GameWorld& world);
			void HandleHeldObjObject();
			void ThrowObject(GameWorld& world);
		private:
		};
	}
}