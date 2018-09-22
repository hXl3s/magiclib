#include "ruleextraction/RuleSolution.hpp"

#include <algorithm>
#include <iostream>

namespace hx {
namespace ruleextraction {

namespace {
enum class RuleSolutionStatus : uint8_t {
    UNMARKED = 0x00,
    MARKED = 0x01,
    WRONG = 0x02,
    OVERLAPPED = 0x03
};

bool insideHyperrect(const float *controlPoint,
                     const float *centerPoints,
                     const float *lowerBounds,
                     const float *upperBounds,
                     std::size_t dims) {
    for (std::size_t i = 0; i < dims;
         ++i, ++controlPoint, ++centerPoints, ++lowerBounds, ++upperBounds) {
        if (*controlPoint < *centerPoints - *lowerBounds
            || *controlPoint > *centerPoints + *upperBounds)
            return false;
    }

    return true;
}

RuleSolutionStatus controlPointStatus(const float *controlPoint,
                                      const float *centerPoints,
                                      const float *lowerBounds,
                                      const float *upperBounds,
                                      const std::size_t *ruleClasses,
                                      std::size_t controlClass,
                                      std::size_t rulesSize,
                                      std::size_t dims,
                                      std::size_t stride) {
    RuleSolutionStatus result = RuleSolutionStatus::UNMARKED;
    for (std::size_t i = 0; i < rulesSize; ++i) {
        if (insideHyperrect(controlPoint, centerPoints, lowerBounds, upperBounds, dims)) {
            auto classesMatch = (*ruleClasses == controlClass);
            if (result == RuleSolutionStatus::UNMARKED)
                result =
                    classesMatch ? RuleSolutionStatus::MARKED : RuleSolutionStatus::WRONG;
            else
                return RuleSolutionStatus::OVERLAPPED;
        }

        centerPoints += stride;
        lowerBounds += stride * 2;
        upperBounds += stride * 2;
        ++ruleClasses;
    }

    return result;
}

}// namespace

void RuleSolution::neighbouring_() {
    std::uniform_int_distribution<> ruleDist(0, 2 * this->_size - 1);
    std::uniform_int_distribution<> dimsDist(0, this->_dims - 1);
    std::uniform_int_distribution<> mode(0, 2);
    std::uniform_real_distribution<> changeDist(0.5, 1.5);

    std::size_t r1i, r2i, r1d, r2d, d1, d2;
    double change;
    switch (mode(this->_randomness)) {
        case 0:
            // switch dimensions
            r1i = ruleDist(this->_randomness);
            r2i = r1i & 0x01 ? r1i - 1 : r1i + 1;
            d1 = dimsDist(this->_randomness);
            d2 = dimsDist(this->_randomness);

            std::iter_swap(this->_ruleBoxes.get_as<float>() + this->_stride * r1i + d1,
                           this->_ruleBoxes.get_as<float>() + this->_stride * r1i + d2);
            std::iter_swap(this->_ruleBoxes.get_as<float>() + this->_stride * r2i + d1,
                           this->_ruleBoxes.get_as<float>() + this->_stride * r2i + d2);
            break;

        case 1:
            r1i = ruleDist(this->_randomness);
            r2i = r1i & 0x01 ? r1i - 1 : r1i + 1;
            d1 = dimsDist(this->_randomness);
            change = changeDist(this->_randomness);

            this->_ruleBoxes.get_as<float>()[this->_stride * r1i + d1] *= change;
            this->_ruleBoxes.get_as<float>()[this->_stride * r2i + d1] *= change;
            break;

        case 2:
            r1i = ruleDist(this->_randomness);
            r2i = r1i & 0x01 ? r1i - 1 : r1i + 1;
            r1d = ruleDist(this->_randomness);
            r2d = r1d & 0x01 ? r1d - 1 : r1d + 1;
            d1 = dimsDist(this->_randomness);
            d2 = dimsDist(this->_randomness);

            std::iter_swap(this->_ruleBoxes.get_as<float>() + this->_stride * r1i + d1,
                           this->_ruleBoxes.get_as<float>() + this->_stride * r1d + d2);
            std::iter_swap(this->_ruleBoxes.get_as<float>() + this->_stride * r2i + d2,
                           this->_ruleBoxes.get_as<float>() + this->_stride * r2d + d1);
            break;

        default:
            return;
    }
}

std::size_t RuleSolution::scoreSolution() const {
    std::size_t energy = this->_controlClasses.size();
    const auto *controlPoint = this->_controlPoints->get_as<float>();

    for (std::size_t i = 0; i < this->_controlClasses.size(); ++i) {
        RuleSolutionStatus status =
            controlPointStatus(controlPoint,
                               this->_ruleCentres->get_as<float>(),
                               this->_ruleBoxes.get_as<float>(),
                               this->_ruleBoxes.get_as<float>() + this->_stride,
                               this->_ruleClasses.data(),
                               this->_controlClasses[i],
                               this->_size,
                               this->_dims,
                               this->_stride);
        if (status == RuleSolutionStatus::MARKED)
            --energy;
        else if (status == RuleSolutionStatus::OVERLAPPED)
            ++energy;
    }

    return energy;
}

}// namespace ruleextraction
}// namespace hx