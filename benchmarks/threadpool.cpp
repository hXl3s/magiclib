#define __HX_SUPPORT_BOOST
#define __HX_SUPPORT_TBB

#include "ThreadPool.hpp"
#include <chrono>
#include <cmath>
#include <iostream>
#include "benchmark.hpp"

int simple_light_task(int i) {
    return i * i;
}

int data_generator() {
    static int v = 0;
    return v++;
}

void performance_stdadapter_single(const std::vector<int> &input) {
    hx::ThreadPool thread_pool;
    std::vector<std::future<int>> results;
    results.reserve(input.size());
    START_MEASURE("Standard Adapter - Single");

    for (int a : input) {
        results.push_back(std::move(thread_pool.async_task(simple_light_task, a)));
    }

    for (auto &f : results) {
        f.get();
    }

    STOP_MEASURE();
}

void performance_stdadapter_multi(const std::vector<int> &input) {
    hx::ThreadPool thread_pool;

    START_MEASURE("Standard Adapter - Multiple");
    auto r = thread_pool.async_map([](const int* i) -> int { return simple_light_task(*i);}, input.begin(), input.end());
    r.get();
    STOP_MEASURE();
}

void performance_boost_single(const std::vector<int> &input) {
    hx::ThreadPool<hx::__internal::__QueueAdapterBoost<std::unique_ptr<hx::__internal::__PoolTaskBase>>> thread_pool;
    std::vector<std::future<int>> results;
    results.reserve(input.size());

    START_MEASURE("Boost Adapter - Single");
    for (int a : input) {
        results.push_back(std::move(thread_pool.async_task(simple_light_task, a)));
    }

    for (auto &f : results) {
        f.get();
    }
    STOP_MEASURE();
}

void performance_boost_multi(const std::vector<int> &input) {
    hx::ThreadPool<hx::__internal::__QueueAdapterBoost<std::unique_ptr<hx::__internal::__PoolTaskBase>>> thread_pool;

    START_MEASURE("Boost Adapter - Multiple");
    auto r = thread_pool.async_map([](const int* i) -> int { return simple_light_task(*i);}, input.begin(), input.end());
    r.get();
    STOP_MEASURE();
}
void performance_tbb_single(const std::vector<int> &input) {
    hx::ThreadPool<hx::__internal::__QueueAdapterTBB<std::unique_ptr<hx::__internal::__PoolTaskBase>>> thread_pool;
    std::vector<std::future<int>> results;
    results.reserve(input.size());

    START_MEASURE("TBB Adapter - Single");
    for (int a : input) {
        results.push_back(std::move(thread_pool.async_task(simple_light_task, a)));
    }

    for (auto &f : results) {
        f.get();
    }
    STOP_MEASURE();
}

void performance_tbb_multi(const std::vector<int> &input) {
    hx::ThreadPool<hx::__internal::__QueueAdapterTBB<std::unique_ptr<hx::__internal::__PoolTaskBase>>> thread_pool;

    START_MEASURE("TBB Adapter - Multiple");
    auto r = thread_pool.async_map([](const int* i) -> int { return simple_light_task(*i);}, input.begin(), input.end());
    r.get();
    STOP_MEASURE();
}

int main(int argc, char *argv[]) {
    std::vector<int> input_data(1000000);
    std::generate(input_data.begin(), input_data.end(), data_generator);

    performance_stdadapter_single(input_data);
    performance_stdadapter_multi(input_data);
    performance_boost_single(input_data);
    performance_boost_multi(input_data);
    performance_tbb_single(input_data);
    performance_tbb_multi(input_data);
}
