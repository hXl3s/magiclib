#pragma once

#include "memory/Utils.hpp"
#include "optimisation/discrete/GeneticAlgorithm.hpp"
#include "ruleextraction/RuleSolution.hpp"

#include <vector>

namespace hx {
namespace ruleextraction {

class RuleSolutionFactory {
public:
    RuleSolutionFactory(std::size_t &dim,
                        std::size_t &stride,
                        hx::memory::Storage *ruleStorage,
                        hx::memory::Storage *controlStorage,
                        std::vector<std::size_t> &ruleClasses,
                        std::vector<std::size_t> &controlClasses,
                        std::default_random_engine &randomness)
        : _dim(dim)
        , _stride(stride)
        , _ruleStorage(ruleStorage)
        , _controlStorage(controlStorage)
        , _ruleClasses(ruleClasses)
        , _controlClasses(controlClasses)
        , _randomness(randomness) {}

    hx::ruleextraction::RuleSolution operator()() {
        return hx::ruleextraction::RuleSolution(this->_dim,
                                                this->_stride,
                                                this->_ruleStorage,
                                                this->_controlStorage,
                                                this->_controlClasses,
                                                this->_ruleClasses,
                                                this->_randomness);
    }

private:
    std::size_t &_dim;
    std::size_t &_stride;

    hx::memory::Storage *_ruleStorage;
    hx::memory::Storage *_controlStorage;
    std::vector<std::size_t> &_ruleClasses;
    std::vector<std::size_t> &_controlClasses;
    std::default_random_engine &_randomness;
};

using GeneticAlgorithmBase =
    hx::optimisation::discrete::GeneticAlgorithm<hx::ruleextraction::RuleSolution,
                                                 hx::ruleextraction::RuleSolutionFactory>;

class RuleOptimisator final : public GeneticAlgorithmBase {
public:
    RuleOptimisator(std::size_t population,
                    std::size_t stride,
                    std::size_t size,
                    double crossoverFactor,
                    double mutationFactor,
                    hx::ruleextraction::RuleSolutionFactory factory,
                    std::default_random_engine &randomness)
        : GeneticAlgorithmBase(
              population,
              crossoverFactor,
              mutationFactor,
              std::forward<hx::ruleextraction::RuleSolutionFactory>(factory))
        , _strides(stride)
        , _size(size)
        , _randomness(randomness) {}

    virtual ~RuleOptimisator(){};

private:
    void crossoverSolution(const hx::ruleextraction::RuleSolution &parrent_a,
                           const hx::ruleextraction::RuleSolution &parrent_b,
                           hx::ruleextraction::RuleSolution &child) override {
        std::bernoulli_distribution dp(0.5);

        auto parrentData_a = parrent_a.getData();
        auto parrentData_b = parrent_b.getData();
        auto childData = child.getData();

        for (std::size_t i = 0; i < this->_size * this->_strides * 2 - 1; ++i) {
            childData[i] = dp(this->_randomness) ? parrentData_a[i] : parrentData_b[i];
        }
        child.invalidateScore();
    }

    std::size_t _strides;
    std::size_t _size;

    std::default_random_engine &_randomness;
};

class RuleExtractor {
private:
public:
    RuleExtractor(std::size_t population,
                  std::size_t dims,
                  std::vector<std::size_t> controlPointsClasses,
                  std::vector<std::size_t> rulePointsClasses,
                  double crossoverFactor = 0.33,
                  double mutationFactor = 0.1)
        : _randomness(std::random_device()())
        , _dims(dims)
        , _strides(hx::memory::value_padding<8>(_dims))
        , _controlPointsClasses(std::move(controlPointsClasses))
        , _rulePointsClasses(std::move(rulePointsClasses))
        , _ruleCentres(_strides * _rulePointsClasses.size() * sizeof(float))
        , _controlPoints(_strides * _controlPointsClasses.size() * sizeof(float)) {
        this->_pImpl = std::make_unique<hx::ruleextraction::RuleOptimisator>(
            population,
            this->_strides,
            this->_rulePointsClasses.size(),
            crossoverFactor,
            mutationFactor,
            RuleSolutionFactory(this->_dims,
                                this->_strides,
                                &this->_ruleCentres,
                                &this->_controlPoints,
                                this->_rulePointsClasses,
                                this->_controlPointsClasses,
                                this->_randomness),
            _randomness);
    };

    float *getRulesCentres() { return this->_ruleCentres.get_as<float>(); }
    float *getControlCentres() { return this->_controlPoints.get_as<float>(); }

    std::size_t getStride() const { return this->_strides; }
    std::size_t getDims() const { return this->_dims; }
    std::size_t getControlPointsNumber() const {
        return this->_controlPointsClasses.size();
    }
    std::size_t getRulesNumber() const { return this->_rulePointsClasses.size(); }
    std::size_t fitEpoch() { return _pImpl->fitEpoch(); }
    float *getBest() { return _pImpl->getBest().getData(); }

    std::size_t *ruleClasses() { return _rulePointsClasses.data(); }

private:
    std::default_random_engine _randomness;
    std::size_t _dims;
    std::size_t _strides;

    std::vector<std::size_t> _controlPointsClasses;
    std::vector<std::size_t> _rulePointsClasses;

    hx::memory::Storage _ruleCentres;
    hx::memory::Storage _controlPoints;

    std::unique_ptr<hx::ruleextraction::RuleOptimisator> _pImpl;
};
}// namespace ruleextraction
}// namespace hx