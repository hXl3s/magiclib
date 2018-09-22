#pragma once

#include <array>

#include "memory/Storage.hpp"
#include "memory/View.hpp"

namespace hx {
namespace memory {

template <typename T, std::size_t Dimensions>
class StorageView : public hx::memory::View<T, Dimensions> {
public:
    StorageView(hx::memory::Storage *storage,
                const std::array<std::size_t, Dimensions> &strides,
                const std::array<std::size_t, Dimensions> &sizes)
        : View<T, Dimensions>(storage->get_as<T>(), strides, sizes), _storage(storage) {}

    StorageView(hx::memory::Storage *storage,
                const std::array<std::size_t, Dimensions> &sizes)
        : StorageView(storage, View<T, Dimensions>::getStridesFromSizes(sizes), sizes) {}

    template <typename... DimsValues,
              typename = std::enable_if_t<
                  sizeof...(DimsValues) == Dimensions
                  && (... && std::is_convertible_v<DimsValues, std::size_t>)>>
    StorageView(hx::memory::Storage *storage, DimsValues... dims)
        : StorageView(storage, {static_cast<std::size_t>(dims)...}) {}

    hx::memory::Storage &getStorage() { return *(this->_storage); }
    const hx::memory::Storage &getStorage() const { return *(this->_storage); }

private:
    hx::memory::Storage *_storage;
};

}// namespace memory
}// namespace hx