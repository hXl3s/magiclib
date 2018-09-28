#pragma once
#include <iostream>
#include "optimisation/discrete/OptimisationSolution.hpp"

#include "memory/Storage.hpp"

#include <boost/align/aligned_allocator.hpp>

#include <random>

namespace hx {
namespace ruleextraction {
class RuleSolution
    : public hx::optimisation::discrete::OptimisationSolution<std::size_t> {
public:
    RuleSolution(std::size_t dims,
                 std::size_t stride,
                 hx::memory::Storage *ruleCentres,
                 hx::memory::Storage *controlPoints,
                 const std::vector<std::size_t> &controlClasses,
                 const std::vector<std::size_t> &ruleClasses,
                 std::default_random_engine &entropy)
        : _dims(dims)
        , _stride(stride)
        , _size(ruleClasses.size())
        , _ruleBoxes(2 * _stride * _size * sizeof(float),
                     boost::alignment::aligned_allocator<float, 32>())
        , _ruleCentres(ruleCentres)
        , _controlPoints(controlPoints)
        , _ruleClasses(ruleClasses)
        , _controlClasses(controlClasses)
        , _randomness(entropy) {
        std::normal_distribution<float> dist(0.0, 1.0);

        for (std::size_t i = 0; i < _size * 2; ++i) {
            for (std::size_t j = 0; j < _dims; ++j) {
                auto current = this->_ruleBoxes.get_as<float>() + i * this->_stride + j;
                *current = std::abs(dist(this->_randomness));
            }
        }
    };

    RuleSolution(const RuleSolution &rhs)
        : _dims(rhs._dims)
        , _stride(rhs._stride)
        , _size(rhs._size)
        , _ruleBoxes(rhs._ruleBoxes.copy())
        , _ruleCentres(rhs._ruleCentres)
        , _controlPoints(rhs._controlPoints)
        , _ruleClasses(rhs._ruleClasses)
        , _controlClasses(rhs._controlClasses)
        , _randomness(rhs._randomness) {}

    virtual ~RuleSolution() = default;

    RuleSolution &operator=(const RuleSolution &rhs) {
        this->_dims = rhs._dims;
        this->_stride = rhs._stride;
        this->_size = rhs._size;

        this->_ruleBoxes = rhs._ruleBoxes.copy();
        this->_ruleCentres = rhs._ruleCentres;
        this->_controlPoints = rhs._controlPoints;

        return *this;
    }

    float *getData() { return _ruleBoxes.get_as<float>(); }
    const float *getData() const { return _ruleBoxes.get_as<float>(); }

private:
    std::size_t scoreSolution() const override;
    void neighbouring_() override;

    std::size_t _dims;
    std::size_t _stride;
    std::size_t _size;

    hx::memory::Storage _ruleBoxes;
    hx::memory::Storage *_ruleCentres;
    hx::memory::Storage *_controlPoints;

    const std::vector<std::size_t> &_ruleClasses;
    const std::vector<std::size_t> &_controlClasses;
    std::default_random_engine &_randomness;
};
}// namespace ruleextraction
}// namespace hx