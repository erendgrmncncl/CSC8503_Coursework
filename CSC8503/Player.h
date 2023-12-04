#pragma once

#include "GameObject.h"

namespace NCL {
	namespace CSC8503 {
		class Player : public GameObject {
		public:
			Player(const Vector3& position, float inverseMass = 100.f);

			void HandlePlayerControls(float dt, GameWorld& world);
		protected:
			float speed = 10.f;
			GameObject* heldObj = nullptr;

			void PickUpObject(GameWorld& world);
		private:
		};
	}
}