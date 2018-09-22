#pragma once

#include <optional>

namespace hx {
namespace optimisation {
namespace discrete {

template <typename T>
class OptimisationSolution {
public:
    T getScore() noexcept {
        if (!score.has_value()) score = this->scoreSolution();
        return score.value();
    }

    void invalidateScore() noexcept { score.reset(); }
    void changeSolution() noexcept {
        this->neighbouring_();
        this->invalidateScore();
    }

protected:
    virtual T scoreSolution() const = 0;
    virtual void neighbouring_() = 0;

private:
    std::optional<T> score;
};

}// namespace discrete
}// namespace optimisation

}// namespace hx