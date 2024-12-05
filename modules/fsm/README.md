# FSM
The FSM (Finite State Machine) module provides a simple, efficient, and extensible framework for implementing finite state machines in embedded systems. It is designed with the principle that only one state can be active at any given time, ensuring clarity and predictability in state transitions.

## Key Features
- Lightweight and easy-to-integrate FSM implementation.
- Supports event-driven transitions.
- Modular design with a clear separation of state logic and event handling.

## Design Principles
This FSM module follows the following core principles:

1. Single Active State:
  - At any moment, the FSM can have only one active state.
  - When an event triggers a state transition, the FSM evaluates conditions sequentially in the order of state registration. The first state to satisfy its condition becomes active, and no further states are evaluated during that event.
  - This ensures deterministic behavior and eliminates ambiguity in state activation.
2. Priority by Registration Order:
  - States are evaluated in the order they are registered with the FSM.
  - If multiple states could potentially be satisfied for the same event, the earliest registered state takes precedence.
  - The registration order determines the priority.
3. Event-Driven Transitions:
  - State transitions are triggered by external events passed to the FSM.
  - Each state's transition condition is evaluated dynamically based on the current event.
4. Immutable State List:
  - The list of states is immutable during FSM execution.
  - Handlers cannot add or remove states dynamically while the FSM is processing events. Any modifications to the state list should be done only during initialization.
5. Predictability and Simplicity:
  - The module avoids complex behaviors like simultaneous state activations or nested state dependencies, making it suitable for resource-constrained embedded systems.

## Usage Guidelines
To ensure proper usage of the FSM module:

1. Avoid Overlapping State Conditions:
  - Design state conditions so that only one state can satisfy a given event.
  - If multiple states could logically satisfy the same condition, redesign the states to include explicit priority handling or mutual exclusivity.
2. Understand State Evaluation Order:
  - Be aware that the order of state registration determines priority by default.
  - If priority is important, explicitly register states in the desired order.
3. Immutable State List During Execution:
  - Do not attempt to modify (add or remove) states dynamically within a state handler or while processing an event. This is not supported and may lead to undefined behavior.
  - Modify the state list only during initialization.
4. Debugging State Transitions:
  - Use debugging hooks provided by the FSM module to verify that state transitions are occurring as expected.
  - Ensure that state transition conditions are well-defined and mutually exclusive.

## Example
Refer to test cases or the example below for a simple demonstration of using the FSM module.

- [Test cases](tests/src/fsm/fsm_test.cpp)
- [FSM example](https://github.com/pazzk-labs/OCPP/tree/main/examples)

## FAQ
### Why does only one state handler get called per event?
The FSM module is designed with the principle of a single active state. When a state condition is satisfied, the FSM immediately triggers the corresponding state handler and stops further evaluation. This simplifies state logic and ensures deterministic transitions.

### What happens if multiple states can satisfy the same event?
Only the first registered state with a satisfied condition will be activated. Ensure that your state conditions are mutually exclusive or prioritize states during registration.

### Can I add or remove states dynamically while processing events?
No. The FSM module does not support dynamic modifications to the state list during event processing. This ensures the integrity and predictability of state evaluations. Modify the state list only during initialization.

### What determines state evaluation priority?
The registration order determines the evaluation priority. The first registered state has the highest priority.
