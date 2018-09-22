#pragma once

#include <cmath>
#include <random>
#include <type_traits>

#include "optimisation/discrete/OptimisationSolution.hpp"

namespace hx {
namespace optimisation {
namespace discrete {

template <typename T,
          typename ScoreType = std::size_t,
          typename = std::enable_if_t<std::is_base_of_v<
              hx::optimisation::discrete::OptimisationSolution<ScoreType>,
              T>>>
class SimulatedAnnealing {
public:
    SimulatedAnnealing(T startingSolution,
                       double startingTemp,
                       double coolingFactor = 0.9999,
                       std::size_t stepsOnEpoch = 1)
        : _currentSolution(startingSolution)
        , _bestSolution(startingSolution)
        , _startingTemp(startingTemp)
        , _currentTemp(startingTemp)
        , _coolingFactor(coolingFactor)
        , _stepsOnEpoch(stepsOnEpoch)
        , _randomness(std::random_device()()) {}

    std::size_t fitEpoch() noexcept {
        std::uniform_real_distribution<> probability(0.0, 1.0);
        for (std::size_t i = 0; i < this->_stepsOnEpoch; ++i) {
            T newSolution = this->_currentSolution;

            newSolution.changeSolution();
            auto scoreNew = newSolution.getScore();
            auto scoreCur = this->_currentSolution.getScore();

            if (scoreNew < scoreCur) {
                this->_currentSolution = newSolution;
                if (scoreNew < _bestSolution.getScore()) {
                    _bestSolution = newSolution;
                }
            } else if (std::exp((scoreCur - scoreNew) / this->_currentTemp)
                       > probability(this->_randomness)) {
                this->_currentSolution = newSolution;
            }
        }
        this->_currentTemp *= this->_coolingFactor;
        return this->_bestSolution.getScore();
    }

protected:
    T _currentSolution;
    T _bestSolution;

    double _startingTemp;
    double _currentTemp;
    double _coolingFactor;

    std::size_t _stepsOnEpoch;
    std::default_random_engine _randomness;
};

}// namespace discrete
}// namespace optimisation
}// namespace hx