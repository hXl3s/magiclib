#include <gtest/gtest.h>

#include <stdexcept>
#include "ThreadPool.hpp"

int test_function_pow(int x) {
    return x * x;
}

TEST(ThreadPoolTest, CreateAndRelease) {
    hx::ThreadPool<> threadPoolA(5);
    hx::ThreadPool<> threadPoolB;

    ASSERT_EQ(threadPoolA.size(), 5);
    ASSERT_EQ(threadPoolB.size(), std::thread::hardware_concurrency());
}

TEST(ThreadPoolTest, QueueSingleTaskLambda) {
    hx::ThreadPool<> threadPool;

    const int expected_result = 5;
    auto result =
        threadPool.async_task([](int x) -> int { return x + 1; }, expected_result - 1);

    ASSERT_EQ(result.get(), expected_result);
}

TEST(ThreadPoolTest, QueueSingleTaskFunction) {
    hx::ThreadPool<> threadPool;

    const int expected_result = 25;
    auto result = threadPool.async_task(test_function_pow, 5);

    ASSERT_EQ(result.get(), expected_result);
}

TEST(ThreadPoolTest, QueueMultipleTaskFunction) {
    hx::ThreadPool<> threadPool;
    std::vector<int> input = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    std::vector<int> expected_results = {1, 4, 9, 16, 25, 36, 49, 64, 81, 100};

    auto result = threadPool
                      .async_map([](std::size_t, int *x) -> int { return (*x) * (*x); },
                                 input.begin(),
                                 input.end())
                      .get();

    for (size_t i = 0; i < 10; ++i) {
        ASSERT_EQ(result[i], expected_results[i]);
    }
}

TEST(ThreadPoolTest, QueueExceptionHandle) {
    hx::ThreadPool<> threadPool;
    auto result = threadPool.async_task(
        [](int) -> int {
            throw std::logic_error("Exception Test String 123456");
            return 0;
        },
        5);

    ASSERT_THROW(result.get(), std::logic_error);
}

TEST(ThreadPoolTest, QueueExceptionMultiple) {
    hx::ThreadPool<> threadPool;
    std::vector<int> input = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    auto result = threadPool.async_map(
        [](std::size_t, int *x) -> int {
            throw std::logic_error("Test");
            return (*x) * (*x);
        },
        input.begin(),
        input.end());

    ASSERT_THROW(result.get(), std::logic_error);
}