#pragma once

#include "TutorialGame.h"

namespace NCL {
	namespace CSC8503 {
		class MainMenuScene : TutorialGame {
		public:
			MainMenuScene();
			~MainMenuScene();
			
			void UpdateGame(float dt) override;
		protected:
			void InitialiseAssets() override;
			void InitWorld() override;
			void HandleMainMenuControls();
		};
	}
}