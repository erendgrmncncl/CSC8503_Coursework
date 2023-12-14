#pragma once
#include "TutorialGame.h"

namespace NCL {
	namespace CSC8503 {

		class Player;
		class NavigationGrid;
		class Enemy;

		class Coursework : public TutorialGame {
		public:
			Coursework(bool isNetwork = false);
			~Coursework();

			void UpdateGame(float dt) override;
			virtual void CollectObjective(GameObject* objective);
			
			void SetIsGameEnded(bool state);

			void RestartGame();

		protected:
			int score = 0;
			
			bool isCameraAttachedToPlayer = true;
			bool isNetworkGame = false;
			bool isGameEnded = false;

			Player* player;
			NavigationGrid* worldGrid;
			Enemy* mazeGuard;
			Enemy* bridgeGuard;

			std::vector<Vector3> testNodes;
			std::vector<StateGameObject*> traps;

			bool CheckIsPlayerInStartingArea();
			void InitialiseAssets() override;
			void InitCamera() override;
			void UpdateKeys() override;
			void InitWorld() override;
			void HandleCameraLock() override;

			virtual void InitPlayer();
			void InitMazeGuard(const Vector3& position);
			void InitBridgeGuard(const Vector3& position);
			void InitBridge(const Vector3& position);
			void InitWorldGrid();
			void InitBridgeOpener(const Vector3& position);
			void SpawnThrowable(const Vector3& position);
			virtual void InitObjective(const Vector3& position);

			StateGameObject* SpawnTrap(const Vector3& position);

			void ToggleCameraMode();
			void HandleCameraModeControls();
			void TestPathFinding();
			void DisplayPathFinding();

			void CreateBrickWall(Vector3& position, Vector3& brickDimensions, bool isHorizontal, int width, int height);
		};
	}
}