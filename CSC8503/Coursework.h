#pragma once
#include "TutorialGame.h"

namespace NCL {
	namespace CSC8503 {
		class Coursework : public TutorialGame {
		public:
			Coursework();
			~Coursework();
		protected:
			void InitialiseAssets() override;
			void InitCamera() override;
			void UpdateKeys() override;
			void InitWorld() override;
		};
	}
}