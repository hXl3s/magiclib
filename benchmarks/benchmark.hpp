#ifndef _HX_BENCHMARK_FRAMEWORK_H_
#define _HX_BENCHMARK_FRAMEWORK_H_ 1

#define START_MEASURE(name)            \
    const char *measure_name = name; \
    auto measure_start = std::chrono::high_resolution_clock::now();

#define STOP_MEASURE()                                                                 \
    auto measure_stop = std::chrono::high_resolution_clock::now();                             \
    auto elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(measure_stop - measure_start); \
    std::cout << measure_name << ". Elapsed time: " << elapsed.count() / 1000000.0        \
              << " ms." << std::endl;

#endif