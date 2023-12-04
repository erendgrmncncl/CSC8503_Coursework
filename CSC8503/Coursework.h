#pragma once
#include "TutorialGame.h"

namespace NCL {
	namespace CSC8503 {


		class Player;
		class Coursework : public TutorialGame {
		public:
			Coursework();
			~Coursework();

			void UpdateGame(float dt) override;
		protected:
			bool isCameraAttachedToPlayer = true;
			Player* player;


			void InitialiseAssets() override;
			void InitCamera() override;
			void UpdateKeys() override;
			void InitWorld() override;
			void InitPlayer();
			void ToggleCameraMode();
			void HandleCameraModeControls();
};
	}
}