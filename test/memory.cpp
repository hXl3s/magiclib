#include <gtest/gtest.h>

#include <stdexcept>

#include "memory/Storage.hpp"
#include "memory/StorageView.hpp"

TEST(MemoryTest, CreateCustomView) {
    hx::memory::Storage storage(1024 * 1024);
    auto *ptr = storage.get_as<float>();
    for (std::size_t i = 0; i < 1024 * 1024 / sizeof(float); ++i) {
        *ptr++ = i;
    }

    hx::memory::StorageView<float, 4> view(&storage, 1, 2, 3, 4);

    auto x = view[0];
    auto y = view[1][1][2];
    float a = view(0, 0, 1, 2);

    ASSERT_EQ(x(0, 1, 2), a);
    ASSERT_EQ(y(0), view(1, 1, 2, 0));
    ASSERT_EQ(y(2), view(1, 1, 2, 2));
    ASSERT_EQ(y(3), view(1, 1, 2, 3));
}