#pragma once

#include "optimisation/discrete/OptimisationSolution.hpp"

#include "ThreadPool.hpp"

#include <iostream>
#include <random>
#include <set>

namespace hx {
namespace optimisation {
namespace discrete {

template <
    typename T,
    typename FactoryType,
    typename ScoreType = std::size_t,
    typename = std::enable_if_t<
        std::is_base_of_v<hx::optimisation::discrete::OptimisationSolution<ScoreType>,
                          T> && std::is_same_v<std::invoke_result_t<FactoryType>, T>>>
class GeneticAlgorithm {
public:
    GeneticAlgorithm(std::size_t populationSize,
                     double crossoverFactor,
                     double mutationFactor,
                     FactoryType &&factory)
        : _solutionFactory(factory)
        , _populationSize(populationSize)
        , _crossoverFactor(static_cast<std::size_t>(crossoverFactor * populationSize))
        , _mutationFactor(static_cast<std::size_t>(mutationFactor * populationSize))
        , _populationSorted(populationSize)
        , _populationScore(populationSize)
        , _bestSolution(std::move(this->_solutionFactory()))
        , _randomness(std::random_device()()) {
        for (std::size_t i = 0; i < this->_populationSize; ++i) {
            this->_populationSorted[i] = i;
        }

        this->_population.reserve(this->_populationSize);
        for (std::size_t i = 0; i < this->_populationSize; ++i) {
            this->_population.push_back(std::move(this->_solutionFactory()));
        }
    }

    virtual ~GeneticAlgorithm(){};

    std::size_t fitEpoch() {
        this->updateSolutionScore();
        this->updateSolutionBest();

        this->crossoverPopulation();
        this->mutatePopulation();

        return this->_bestSolution.getScore();
    }

    T &getBest() { return this->_bestSolution; }

protected:
    virtual void crossoverSolution(const T &parrent_a, const T &parrent_b, T &child) = 0;
    virtual void mutateSolution(T &solution) { solution.changeSolution(); }

    void updateSolutionScore() {
        _threadPool
            .async_map(
                [this](std::size_t i, T *populationSample) -> ScoreType {
                    this->_populationScore[i] = populationSample->getScore();
                    return this->_populationScore[i];
                },
                _population.begin(),
                _population.end())
            .get();
    }

    void updateSolutionBest() {
        std::sort(_populationSorted.begin(),
                  _populationSorted.end(),
                  [this](std::size_t a, std::size_t b) -> bool {
                      return this->_populationScore[a] < this->_populationScore[b];
                  });

        if (_populationScore[_populationSorted[0]] < _bestSolution.getScore()) {
            this->_bestSolution = _population[_populationSorted[0]];
            this->_bestSolution.invalidateScore();
        }
    }

    void crossoverPopulation() {
        std::set<std::size_t> indexesToRemove;
        std::geometric_distribution selection(0.1);
        for (std::size_t i = 0; i < this->_crossoverFactor; ++i) {
            std::size_t randomSolutionIndex = selection(this->_randomness);
            if (randomSolutionIndex < this->_populationSize)
                indexesToRemove.insert(
                    this->_populationSorted[this->_populationSize - randomSolutionIndex
                                            - 1]);
        }

        while (!indexesToRemove.empty()) {
            std::size_t childIndex = *indexesToRemove.begin();
            std::size_t parrentA, parrentB;

            parrentA = selection(this->_randomness);
            parrentB = selection(this->_randomness);

            if (parrentA >= this->_populationSize || parrentB >= this->_populationSize)
                continue;
            parrentA = this->_populationSorted[parrentA];
            parrentB = this->_populationSorted[parrentB];

            if (parrentA == parrentB || parrentA == childIndex || parrentB == childIndex)
                continue;

            this->crossoverSolution(this->_population[parrentA],
                                    this->_population[parrentB],
                                    this->_population[childIndex]);
            this->_population[childIndex].invalidateScore();
            indexesToRemove.erase(childIndex);
        }
    }

    void mutatePopulation() {
        std::uniform_int_distribution<> selection(0, this->_populationSize - 1);

        for (std::size_t i = 0; i < this->_mutationFactor; ++i) {
            std::size_t index = selection(this->_randomness);
            this->mutateSolution(this->_population[index]);
        }
    }

private:
    hx::ThreadPool<> _threadPool;
    FactoryType _solutionFactory;

    std::size_t _populationSize;
    std::size_t _crossoverFactor;
    std::size_t _mutationFactor;

    std::vector<T> _population;
    std::vector<std::size_t> _populationSorted;
    std::vector<ScoreType> _populationScore;
    T _bestSolution;

    std::default_random_engine _randomness;
};

}// namespace discrete
}// namespace optimisation
}// namespace hx