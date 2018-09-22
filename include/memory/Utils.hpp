#pragma once

#include <array>
#include <cstdint>

namespace hx {
namespace memory {

namespace __internal {

template <typename T, std::size_t First, std::size_t... Tail>
constexpr std::array<T, sizeof...(Tail)> arrayTailImpl(
    const std::array<T, sizeof...(Tail) + 1> &array,
    std::index_sequence<First, Tail...>) {
    return {array[Tail]...};
}
}// namespace __internal

template <int Padding = 1>
constexpr std::size_t value_padding(std::size_t value) noexcept {
    if constexpr (!(Padding & (Padding - 1))) {
        return (value + Padding - 1) & ~(Padding - 1);
    } else {
        return (1 + (value - 1) / Padding) * Padding;
    }
}

template <typename T, std::size_t Dims>
constexpr std::array<T, Dims - 1> arrayTail(const std::array<T, Dims> &array) {
    return __internal::arrayTailImpl(array, std::make_index_sequence<Dims>{});
}

}// namespace memory
}// namespace hx
