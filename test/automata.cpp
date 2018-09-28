#include <gtest/gtest.h>
#include <stdexcept>

#include <bitset>
#include <unordered_map>

#include "automata/DFA.hpp"
#include "automata/TransitionFunction.hpp"

template <typename T, typename BaseType>
void performAutomataTest(
    hx::DFA &dfa, T *data_pointer, bool *final_pointer, std::size_t size, BaseType base) {
    for (std::size_t i = 0; i < size; ++i) {
        dfa.reset();

        for (auto c : data_pointer[i])
            dfa.process(c - base);
        ASSERT_EQ(dfa.isFinal(), final_pointer[i]);
    }
}

TEST(AutomataTest, ExemplaryAutomata_Ending00) {
    hx::TransitionFunctionMap::ContainerMap map = {
        {{0, 0}, 1}, {{0, 1}, 0}, {{1, 0}, 2}, {{1, 1}, 0}, {{2, 0}, 2}, {{2, 1}, 0}};
    hx::DFA dfa(map, 0, {2});

    std::string testStrings[] = {"11100101001110101110101001010100",
                                 "10110101010100000010000010010101",
                                 "1",
                                 "0",
                                 "00",
                                 "11111111111111111111111111111100"};
    bool correct[] = {true, false, false, false, true, true};

    performAutomataTest(dfa, testStrings, correct, 6, '0');

    dfa.compile(hx::DFA::flags::CREATE_DYNAMIC_TABLE);
    performAutomataTest(dfa, testStrings, correct, 6, '0');

    dfa.compile(hx::DFA::flags::REDUCE_STATE_TABLE);
    performAutomataTest(dfa, testStrings, correct, 6, '0');
}

TEST(AutomataTest, ExemplaryAutomata_Substr011) {
    hx::TransitionFunctionMap::ContainerMap map = {{{0, 0}, 1},
                                                   {{0, 1}, 0},
                                                   {{1, 0}, 0},
                                                   {{1, 1}, 2},
                                                   {{2, 0}, 1},
                                                   {{2, 1}, 3},
                                                   {{3, 0}, 3},
                                                   {{3, 1}, 3}};
    hx::DFA dfa(map, 0, {3});

    std::string testStrings[] = {"11100101001110101110101001010100",
                                 "10110101010100000010000010010101",
                                 "1",
                                 "0",
                                 "001",
                                 "011",
                                 "11111111111111111111111111111100"};
    bool correct[] = {true, true, false, false, false, true, false};

    performAutomataTest(dfa, testStrings, correct, 7, '0');

    dfa.compile(hx::DFA::flags::CREATE_DYNAMIC_TABLE);
    performAutomataTest(dfa, testStrings, correct, 7, '0');

    dfa.compile(hx::DFA::flags::REDUCE_STATE_TABLE);
    performAutomataTest(dfa, testStrings, correct, 7, '0');
}

TEST(AutomataTest, ExemplaryAutomata_Beg1BinDiv5) {
    hx::TransitionFunctionMap::ContainerMap map = {{{0, 0}, 6},
                                                   {{0, 1}, 1},
                                                   {{1, 0}, 2},
                                                   {{1, 1}, 3},
                                                   {{2, 0}, 4},
                                                   {{2, 1}, 5},
                                                   {{3, 0}, 1},
                                                   {{3, 1}, 2},
                                                   {{4, 0}, 3},
                                                   {{4, 1}, 4},
                                                   {{5, 0}, 5},
                                                   {{5, 1}, 1},
                                                   {{6, 0}, 6},
                                                   {{6, 1}, 6}};
    hx::DFA dfa(map, 0, {5});
    std::string testStrings[6] = {
        "1101001000",
        "01101001000",
        "11011001",
        "011011001",
        "1000100100010000100001111010011000000001000111011011011010010",
        "01000100100010000100001111010011000000001000111011011011010010"};
    bool correct[6] = {true, false, false, false, true, false};

    performAutomataTest(dfa, testStrings, correct, 6, '0');

    dfa.compile(hx::DFA::flags::CREATE_DYNAMIC_TABLE);
    performAutomataTest(dfa, testStrings, correct, 6, '0');

    dfa.compile(hx::DFA::flags::REDUCE_STATE_TABLE);
    performAutomataTest(dfa, testStrings, correct, 6, '0');
}