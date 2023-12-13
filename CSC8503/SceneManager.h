#pragma once

namespace NCL {
	namespace CSC8503 {
		
		class TutorialGame;
		class PushdownMachine;
		
		enum Scene {
			MainMenu = 0,
			GameScene = 1,
			CollisionTest = 2,
			NetworkScene = 3
		};


		class SceneManager {
		public:

			void InitScenes();
			void InitPushdownMachine();
			void SetCurrentScene(Scene scene);
			
			bool GetIsForceQuit();
			void SetIsForceQuit(bool isForceQuit);

			PushdownMachine* GetScenePushdownMachine();
			TutorialGame* GetCurrentScene();
			static SceneManager* GetSceneManager();
		protected:
			bool isForceQuit = false;

			SceneManager();
			~SceneManager();

			static SceneManager* instance;
			
			TutorialGame* currentScene = nullptr;
			PushdownMachine* pushdownMachine = nullptr;

			std::map<Scene, TutorialGame*> gameScenesMap;
		};
	}
}