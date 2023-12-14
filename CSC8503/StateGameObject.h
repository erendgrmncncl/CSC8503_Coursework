#pragma once
#include "GameObject.h"

namespace NCL {
    namespace CSC8503 {
        class StateMachine;
        class State;
        class StateTransition;
        class StateGameObject : public GameObject  {
        public:
            StateGameObject(bool addStates = true);
            StateGameObject(std::vector<State*>& states, std::vector<StateTransition*>& stateTransitions);
            ~StateGameObject();

            virtual void Update(float dt);
            void AddStates(std::vector<State*>& states);
            void AddStateTransitions(std::vector<StateTransition*>& stateTransitions);

        protected:
            void MoveLeft(float dt);
            void MoveRight(float dt);

            StateMachine* stateMachine;
            float counter;
        };
    }
}
