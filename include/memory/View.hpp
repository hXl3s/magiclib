#pragma once

#include <array>
#include <iostream>

#include "memory/Storage.hpp"
#include "memory/Utils.hpp"

namespace hx {
namespace memory {

template <typename T, std::size_t Dimensions>
class View {
public:
    View(T *storage,
         const std::array<std::size_t, Dimensions> &strides,
         const std::array<std::size_t, Dimensions> &sizes)
        : _strides(strides), _sizes(sizes), _storage(storage) {}

    View(T *storage, const std::array<std::size_t, Dimensions> &sizes)
        : View(storage, getStridesFromSizes(sizes), sizes) {}

    template <typename... DimsValues,
              typename = std::enable_if_t<
                  sizeof...(DimsValues) == Dimensions
                  && (... && std::is_convertible_v<DimsValues, std::size_t>)>>
    View(T *storage, DimsValues... dims)
        : View(storage, {static_cast<std::size_t>(dims)...}) {}

    constexpr std::size_t rank() const { return Dimensions; }
    constexpr const std::array<std::size_t, Dimensions> &strides() const {
        return this->_strides;
    }
    constexpr const std::array<std::size_t, Dimensions> &shape() const {
        return this->_sizes;
    }
    std::size_t stride(std::size_t n) const { return this->_strides[n]; }
    std::size_t size(std::size_t n) const { return this->_sizes[n]; }

    T *data() { return this->_storage; }
    const T *data() const { return this->_storage; }

    template <typename... DimsValues,
              typename = std::enable_if_t<
                  sizeof...(DimsValues) == Dimensions
                  && (... && std::is_convertible_v<DimsValues, std::size_t>)>>
    T &operator()(DimsValues... indexes) {
        return *(this->_storage
                 + this->getOffset(
                       std::array<std::size_t, Dimensions>{
                           static_cast<std::size_t>(indexes)...},
                       std::index_sequence_for<DimsValues...>{}));
    }

    template <typename... DimsValues,
              typename = std::enable_if_t<
                  sizeof...(DimsValues) == Dimensions
                  && (... && std::is_convertible_v<DimsValues, std::size_t>)>>
    const T &operator()(DimsValues... indexes) const {
        return *(this->_storage
                 + this->getOffset(
                       std::array<std::size_t, Dimensions>{
                           static_cast<std::size_t>(indexes)...},
                       std::index_sequence_for<DimsValues...>{}));
    }

    View<T, Dimensions - 1> operator[](std::size_t index) {
        return View<T, Dimensions - 1>(this->_storage + this->_strides[0] * index,
                                       hx::memory::arrayTail(this->_strides),
                                       hx::memory::arrayTail(this->_sizes));
    }

protected:
    static std::array<std::size_t, Dimensions> getStridesFromSizes(
        const std::array<std::size_t, Dimensions> &sizes) {
        std::array<std::size_t, Dimensions> result;
        result.fill(1);

        for (std::size_t i = Dimensions - 1; i > 0; --i) {
            result[i - 1] = result[i] * sizes[i];
        }

        return result;
    }

private:
    template <std::size_t... Indexes>
    std::size_t getOffset(std::array<std::size_t, Dimensions> values,
                          std::index_sequence<Indexes...>) const {
        return (... + (values[Indexes] * this->_strides[Indexes]));
    }

    std::array<std::size_t, Dimensions> _strides;
    std::array<std::size_t, Dimensions> _sizes;
    T *_storage;
};

}// namespace memory
}// namespace hx