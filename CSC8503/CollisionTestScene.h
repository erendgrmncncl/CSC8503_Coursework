#pragma once
#include "TutorialGame.h"
#include <map>
namespace NCL {
	namespace CSC8503 {
		class CollisionTestScene : TutorialGame {
		public:
			CollisionTestScene();
			~CollisionTestScene();
			
			void UpdateGame(float dt) override;
		protected:
			std::map<VolumeType, CollisionVolume*> volumeTypeVolumeObjMap;
			std::map<VolumeType, Mesh*> volumeTypeMeshMap;

			std::vector<StateGameObject*> stateObjects;

			void InitialiseAssets() override;
			void InitWorld() override;

			void InitCollisionObjMap();
			void HandleInputs();
			void AddCollisionTestObjToWorld(VolumeType volumeType);
		};
	}
}
