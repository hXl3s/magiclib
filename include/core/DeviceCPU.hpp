#pragma once

#include <boost/align/aligned_allocator.hpp>
#include "core/Device.hpp"
#include "memory/Storage.hpp"

#include "ThreadPool.hpp"

namespace hx {
class DeviceCPU : public hx::Device {
public:
    hx::DeviceType getType() const override { return hx::DeviceType::CPU; }
    hx::ThreadPool<> &getDefaultThreadPool() { return _defaultThreadPool; }

    template <typename T>
    auto getDefaultAllocator() const {
        return std::allocator<T>();
    }

    template <typename T>
    auto getSpecialAllocator() const {
        return boost::alignment::aligned_allocator<T, 32>();
    }

    hx::memory::Storage getStorage(std::size_t size_in_bytes, bool fast = false) const {
        if (fast) {
            return hx::memory::Storage(size_in_bytes, getSpecialAllocator<std::size_t>());
        } else {
            return hx::memory::Storage(size_in_bytes, getDefaultAllocator<std::size_t>());
        }
    }

private:
    hx::ThreadPool<> _defaultThreadPool;
};
}// namespace hx