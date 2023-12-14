#pragma once

namespace NCL {
	namespace CSC8503 {
		class State;
		class StateTransition;
		class StateGameObject;

		typedef std::multimap<State*, StateTransition*> TransitionContainer;
		typedef TransitionContainer::iterator TransitionIterator;

		class StateMachine	{
		public:
			StateMachine(StateGameObject* stateGameObject = nullptr);
			virtual ~StateMachine(); //made it virtual!

			void AddState(State* s);
			void AddTransition(StateTransition* t);

			virtual void Update(float dt); //made it virtual!

		protected:
			State * activeState;

			StateGameObject* stateObj = nullptr;

			std::vector<State*> allStates;
			TransitionContainer allTransitions;
		};
	}
}