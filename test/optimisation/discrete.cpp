#include <gtest/gtest.h>

#include <iostream>
#include <stdexcept>
#include "optimisation/discrete/GeneticAlgorithm.hpp"
#include "optimisation/discrete/OptimisationSolution.hpp"
#include "optimisation/discrete/SimulatedAnnealing.hpp"

auto testData = std::vector<std::size_t>{
    12, 6, 3, 4, 5, 7, 6, 5, 5, 4, 3, 2, 2, 3, 2, 1, 2, 12, 10, 20, 10, 11};

class MockSolution
    : public hx::optimisation::discrete::OptimisationSolution<std::size_t> {
public:
    MockSolution(std::vector<std::size_t> data) : data(std::move(data)) {}

private:
    std::size_t scoreSolution() const override { return data[pointer]; }
    void neighbouring_() override { pointer++; }
    std::vector<std::size_t> data;

public:
    static std::size_t pointer;
};

std::size_t MockSolution::pointer = 0;

class MockSolutionThrows
    : public hx::optimisation::discrete::OptimisationSolution<std::size_t> {
public:
    MockSolutionThrows(std::vector<std::size_t> data)
        : data(std::move(data)), pointer(0), scored(false) {}

private:
    std::size_t scoreSolution() const override {
        if (scored) throw std::logic_error("");
        scored = true;
        return data[pointer];
    }
    void neighbouring_() override {
        pointer++;
        scored = false;
    }
    std::vector<std::size_t> data;
    std::size_t pointer;
    bool mutable scored;
};

TEST(OptimisationDiscrete, TestOptiSolution) {
    MockSolution m(testData);
    MockSolution::pointer = 0;

    for (std::size_t i = 0; i < testData.size(); ++i) {
        ASSERT_EQ(m.getScore(), testData[i]);
        m.changeSolution();
    }
}

TEST(OptimisationDiscrete, TestOptiSolutionLazy) {
    MockSolutionThrows m(testData);
    MockSolution::pointer = 0;

    for (std::size_t i = 0; i < testData.size(); ++i) {
        ASSERT_EQ(m.getScore(), testData[i]);
        ASSERT_NO_THROW(m.getScore());
        ASSERT_NO_THROW(m.getScore());

        ASSERT_EQ(m.getScore(), testData[i]);

        m.changeSolution();
    }
}

TEST(OptimisationDiscrete, TestSimulatedAnnealing) {
    MockSolution m(testData);
    MockSolution::pointer = 0;

    std::vector<std::size_t> partial(testData.size());
    hx::optimisation::discrete::SimulatedAnnealing<MockSolution> annealing(m, 1000.0);

    std::partial_sum(
        testData.begin(),
        testData.end(),
        partial.begin(),
        [](const auto &a, const auto &b) -> std::size_t { return std::min(a, b); });

    for (std::size_t i = 1; i < testData.size(); ++i) {
        auto a = annealing.fitEpoch();
        ASSERT_EQ(a, partial[i]);
    }
}