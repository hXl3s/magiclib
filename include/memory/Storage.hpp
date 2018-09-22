#pragma once

#include <functional>
#include <memory>

#include <cstring>// for memcpy

namespace hx {
namespace memory {

class Storage {
public:
    Storage(std::size_t size_in_bytes)
        : _size_in_bytes(size_in_bytes), _deleter(Storage::default_deleter) {
        this->_context = static_cast<void *>(new unsigned char[size_in_bytes]);
    }

    template <typename Allocator>
    Storage(std::size_t size_in_bytes, Allocator allocator)
        : _size_in_bytes(size_in_bytes) {
        this->_deleter = [allocator](void *p, std::size_t size) mutable -> void {
            allocator.deallocate(reinterpret_cast<typename Allocator::pointer>(p), size);
        };
        this->_context = static_cast<void *>(allocator.allocate(
            1 + (_size_in_bytes - 1) / sizeof(typename Allocator::value_type)));
    }

    Storage(void *ptr,
            std::size_t size_in_bytes,
            std::function<void(void *, std::size_t)> deleter =
                hx::memory::Storage::default_deleter)
        : _context(ptr), _size_in_bytes(size_in_bytes), _deleter(deleter) {}

    ~Storage() {
        if (this->_context) this->_deleter(this->_context, this->_size_in_bytes);
    }

    Storage(const Storage &) = delete;
    Storage &operator=(Storage &) = delete;

    Storage(Storage &&rhs) {
        this->_context = rhs._context;
        this->_deleter = rhs._deleter;
        this->_size_in_bytes = rhs._size_in_bytes;

        rhs._context = 0;
    }

    Storage &operator=(Storage &&rhs) {
        this->_context = rhs._context;
        this->_deleter = rhs._deleter;
        this->_size_in_bytes = rhs._size_in_bytes;

        rhs._context = 0;

        return *this;
    }

    void *get() { return this->_context; }

    template <typename T>
    T *get_as() {
        return static_cast<T *>(this->_context);
    }

    template <typename T>
    const T *get_as() const {
        return static_cast<const T *>(this->_context);
    }

    std::size_t size() const { return _size_in_bytes; }

    template <typename Allocator>
    Storage copy(Allocator allocator) const {
        Storage copy(this->_size_in_bytes, allocator);
        memcpy(copy._context, this->_context, this->_size_in_bytes);

        return copy;
    }

    Storage copy() const {
        Storage copy(this->_size_in_bytes);
        memcpy(copy._context, this->_context, this->_size_in_bytes);

        return copy;
    }

    template <typename T>
    std::size_t size_as() const {
        return _size_in_bytes / sizeof(T);
    }

    static void default_deleter(void *p, std::size_t) noexcept {
        delete[] static_cast<unsigned char *>(p);
    }

private:
    void *_context;
    std::size_t _size_in_bytes;
    std::function<void(void *, std::size_t)> _deleter;
};

}// namespace memory
}// namespace hx
