#include "StateMachine.h"
#include "State.h"
#include "StateTransition.h"
#include "../CSC8503/StateGameObject.h"
using namespace NCL::CSC8503;

StateMachine::StateMachine(StateGameObject* object)	{
	activeState = nullptr;
	stateObj = object;
}

StateMachine::~StateMachine()	{
	for (auto& i : allStates) {
		delete i;
	}
	for (auto& i : allTransitions) {
		delete i.second;
	}
}

void StateMachine::AddState(State* s) {
	allStates.emplace_back(s);
	if (activeState == nullptr) {
		activeState = s;
	}
}

void StateMachine::AddTransition(StateTransition* t) {
	allTransitions.insert(std::make_pair(t->GetSourceState(), t));
}

void StateMachine::Update(float dt) {
	if (activeState) {
		activeState->Update(dt, stateObj);
	
		//Get the transition set starting from this state node;
		std::pair<TransitionIterator, TransitionIterator> range = allTransitions.equal_range(activeState);

		for (auto& i = range.first; i != range.second; ++i) {
			if (i->second->CanTransition()) {
				State* newState = i->second->GetDestinationState();
				activeState = newState;
			}
		}
	}
}