#pragma once

namespace NCL {
	namespace CSC8503 {
		class StateGameObject;
		typedef std::function<void(float, StateGameObject* )> StateUpdateFunction;

		class  State		{
		public:
			State() {}
			State(StateUpdateFunction someFunc) {
				func		= someFunc;
			}
			void Update(float dt, StateGameObject* object = nullptr)  {
				if (func != nullptr) {
					func(dt, object);
				}
			}
		protected:
			StateUpdateFunction func;
		};
	}
}