#pragma once

#include <numeric>
#include <queue>
#include <vector>

#include "automata/TransitionFunction.hpp"

namespace hx {

hx::TransitionFunction *DFACompileToDynamicTable(hx::TransitionFunction *,
                                                 std::size_t,
                                                 std::size_t);
hx::TransitionFunction *DFACompileToReducedTable(hx::TransitionFunction *,
                                                 std::size_t,
                                                 std::size_t,
                                                 std::size_t);

class DFA {
public:
    struct flags {
        constexpr static std::uint8_t REDUCE_STATE_TABLE = 1 << 1;
        constexpr static std::uint8_t CREATE_DYNAMIC_TABLE = 1 << 2;
    };

    constexpr static std::size_t INVALID_STATE = std::numeric_limits<std::size_t>::max();

    DFA(const hx::TransitionFunctionMap::ContainerMap &stateTransitionMap,
        std::size_t startingState,
        const std::vector<std::size_t> &finalStates)
        : _startingState(startingState)
        , _currentState(startingState)
        , _numberOfStates(0)
        , _numberOfActions(0) {
        _transitionFunction =
            std::make_unique<hx::TransitionFunctionMap>(stateTransitionMap);
        for (const auto &it : stateTransitionMap) {
            _numberOfStates =
                std::max(it.first.first, std::max(it.second, _numberOfStates));
            _numberOfActions = std::max(it.first.second, _numberOfActions);
        }

        _numberOfStates += 1;
        _numberOfActions += 1;

        _createFinalStates(finalStates.cbegin(), finalStates.cend());
    }

    DFA(std::function<std::size_t(std::size_t, std::size_t)> stateTransitionMap,
        std::size_t _startingState,
        const std::vector<std::size_t> &finalStates,
        std::size_t numberOfStates,
        std::size_t numberOfActions)
        : _startingState(_startingState)
        , _currentState(_startingState)
        , _numberOfStates(numberOfStates)
        , _numberOfActions(numberOfActions) {
        _transitionFunction =
            std::make_unique<TransitionFunctionFunc>(stateTransitionMap);

        _createFinalStates(finalStates.cbegin(), finalStates.cend());
    }

    std::size_t process(std::size_t action) {
        _currentState = this->peek(action);
        return _currentState;
    }

    template <typename InputIt1, typename InputIt2>
    std::size_t process(InputIt1 begin, InputIt2 end) {
        while (begin != end)
            process(*(begin++));
        return _currentState;
    }

    bool isFinal() const { return _finalStates[_currentState]; }
    bool peekFinal(std::size_t action) const { return _finalStates[peek(action)]; }

    std::size_t peek(std::size_t action) const {
        return _transitionFunction->get(_currentState, action);
    }

    void reset() { _currentState = _startingState; }
    void compile(std::uint8_t flags) {
        hx::TransitionFunction *newTransitionFunction = _transitionFunction.get();

        if (flags & DFA::flags::CREATE_DYNAMIC_TABLE)
            newTransitionFunction = DFACompileToDynamicTable(
                newTransitionFunction, _numberOfStates, _numberOfActions);

        if (flags & DFA::flags::REDUCE_STATE_TABLE)
            newTransitionFunction = DFACompileToReducedTable(
                newTransitionFunction, _numberOfStates, _numberOfActions, _startingState);

        if (newTransitionFunction != nullptr
            && newTransitionFunction != _transitionFunction.get()) {
            _transitionFunction.reset(newTransitionFunction);
            reset();
        }
    }

private:
    std::unique_ptr<hx::TransitionFunction> _transitionFunction;
    std::size_t _startingState;
    std::size_t _currentState;
    std::vector<char> _finalStates;
    std::size_t _numberOfStates;
    std::size_t _numberOfActions;

    void _createFinalStates(std::vector<std::size_t>::const_iterator begin,
                            std::vector<std::size_t>::const_iterator end) {
        _finalStates.resize(_numberOfStates);

        std::fill(_finalStates.begin(), _finalStates.end(), 0);
        while (begin != end)
            _finalStates[*begin++] = 1;
    }
};
}// namespace hx