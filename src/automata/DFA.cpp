#include "automata/DFA.hpp"
#include "automata/TransitionFunction.hpp"

namespace hx {

hx::TransitionFunction *DFACompileToDynamicTable(hx::TransitionFunction *src,
                                                 std::size_t numberOfStates,
                                                 std::size_t numberOfActions) {
    if (src->getType() == TransitionType::TABLE) return src;

    std::unique_ptr<hx::TransitionFunctionTable> result =
        std::make_unique<hx::TransitionFunctionTable>(numberOfStates, numberOfActions);

    for (std::size_t i = 0; i < numberOfStates; ++i) {
        for (std::size_t j = 0; j < numberOfActions; ++j) {
            try {
                result->set(i, j, src->get(i, j));
            } catch (std::out_of_range &e) {
                result->set(i, j, std::numeric_limits<std::size_t>::max());
            }
        }
    }

    return result.release();
}

// No action reduction, only states
hx::TransitionFunction *DFACompileToReducedTable(hx::TransitionFunction *src,
                                                 std::size_t numberOfStates,
                                                 std::size_t numberOfActions,
                                                 std::size_t startState) {
    std::vector<char> isAccessible(numberOfStates, 0);
    std::queue<std::size_t> q;

    q.push(startState);
    while (!q.empty()) {
        auto current = q.front();
        q.pop();

        isAccessible[current] = 1;
        for (std::size_t i = 0; i < numberOfActions; ++i) {
            try {
                auto transform = src->get(current, i);
                if (transform != DFA::INVALID_STATE && !isAccessible[transform])
                    q.push(transform);
            } catch (std::out_of_range &) {
            }
        }
    }

    auto accessibleNodes = std::accumulate(isAccessible.begin(), isAccessible.end(), 0ul);
    hx::memory::SmallLookupBimap bimapStates(accessibleNodes);
    hx::memory::SmallLookupBimap bimapActions(numberOfActions);

    for (std::size_t i = 0; i < numberOfActions; ++i)
        bimapActions.addPair(i, i);

    for (std::size_t f = 0, s = 0; f < numberOfStates; f++) {
        if (isAccessible[f]) bimapStates.addPair(f, s++);
    }

    std::unique_ptr<hx::TransitionFunctionTable> newTransitionTable =
        std::make_unique<hx::TransitionFunctionTable>(accessibleNodes, numberOfActions);

    for (std::size_t state = 0; state < accessibleNodes; ++state) {
        auto originalState = bimapStates.from(state);
        for (std::size_t action = 0; action < numberOfActions; ++action)
            newTransitionTable->set(state, action, src->get(originalState, action));
    }

    return new TransitionFunctionTableIndirect(
        std::move(newTransitionTable), bimapStates, bimapActions);
}
}// namespace hx