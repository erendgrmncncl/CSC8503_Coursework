#include "StateGameObject.h"
#include "StateTransition.h"
#include "StateMachine.h"
#include "State.h"
#include "PhysicsObject.h"

using namespace NCL;
using namespace CSC8503;

StateGameObject::StateGameObject(bool addStates) {
	counter = 0.f;
	stateMachine = new StateMachine(this);
	if (addStates) {
		State* stateA = new State([&](float dt, StateGameObject* obj)-> void {
			this->MoveLeft(dt);
			});

		State* stateB = new State([&](float dt, StateGameObject* obj)->void {
			this->MoveRight(dt);
			});

		stateMachine->AddState(stateA);
		stateMachine->AddState(stateB);

		StateTransition* stateTransitionAB = new StateTransition(stateA, stateB, [&]()-> bool {
			return this->counter > 3.f;
			});

		StateTransition* stateTransitionBA = new StateTransition(stateB, stateA, [&]()->bool {
			return this->counter < 0.f;
			});

		stateMachine->AddTransition(stateTransitionAB);
		stateMachine->AddTransition(stateTransitionBA);
	}

}

NCL::CSC8503::StateGameObject::StateGameObject(std::vector<State*>& states, std::vector<StateTransition*>& stateTransitions) {

	counter = 0.f;
	stateMachine = new StateMachine();

	for (const auto& state : states) {
		stateMachine->AddState(state);
	}
	for (const auto& stateTransition : stateTransitions) {
		stateMachine->AddTransition(stateTransition);
	}
}

StateGameObject::~StateGameObject() {
	delete stateMachine;
}

void StateGameObject::Update(float dt) {
	stateMachine->Update(dt);
}

void NCL::CSC8503::StateGameObject::AddStates(std::vector<State*>& states) {
	for (const auto& state : states) {
		stateMachine->AddState(state);
	}
}

void NCL::CSC8503::StateGameObject::AddStateTransitions(std::vector<StateTransition*>& stateTransitions) {
	for (const auto& stateTransition : stateTransitions) {
		stateMachine->AddTransition(stateTransition);
	}
}

void StateGameObject::MoveLeft(float dt) {
	GetPhysicsObject()->AddForce({ -1, 0,0 });
	counter += dt;
}

void StateGameObject::MoveRight(float dt) {
	GetPhysicsObject()->AddForce({ 1,0,0 });
	counter -= dt;
}