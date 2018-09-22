#pragma once

#include <cstdint>

namespace hx {
enum class DeviceType : uint16_t {
    CPU,

    // not supported yet

    CUDA,
    UNKNOWN
};

class Device {
public:
    virtual hx::DeviceType getType() const = 0;

    template <typename T>
    T* as() {
        return static_cast<T*>(this);
    }
};
}// namespace hx