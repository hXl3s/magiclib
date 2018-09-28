cmake_minimum_required(VERSION 3.10)
project (hxtk VERSION 1.0 DESCRIPTION "hXl3s Toolkit Library")

set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")
set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

find_package (GTest REQUIRED)
find_package (Boost REQUIRED python3 numpy3)
find_package (PythonLibs REQUIRED)
find_package (TBB REQUIRED)
find_package (MKL REQUIRED)
find_package (Eigen3 REQUIRED NO_MODULE)

include (cmake/clang-format.cmake)

set (C_WARNING_FLAGS "-Wall -Wextra -pedantic")
set (CMAKE_CXX_FLAGS "-std=c++17 -march=native ${C_WARNING_FLAGS}")
set (CMAKE_EXPORT_COMPILE_COMMANDS ON)
add_library(hxtk SHARED "")

target_include_directories(hxtk PRIVATE include)
target_include_directories(hxtk PRIVATE src)
target_include_directories(hxtk PRIVATE ${PYTHON_INCLUDE_DIRS})

target_link_libraries(hxtk 
    Eigen3::Eigen
    Boost::boost
    Boost::python3
    Boost::numpy3
    Intel::MKL
    Intel::TBB
    ${PYTHON_LIBRARIES}
)

target_sources(hxtk
    PRIVATE
        include/ThreadPool.hpp
        include/memory/Storage.hpp
        include/memory/View.hpp
        include/memory/StorageView.hpp
        include/memory/Utils.hpp

        include/automata/TransitionFunction.hpp
        include/automata/DFA.hpp

        include/core/Device.hpp
        include/core/DeviceCPU.hpp

        src/Dummy.cpp
        src/optimisation/discrete/OptimisationSolution.hpp
        src/optimisation/discrete/SimulatedAnnealing.hpp
        src/optimisation/discrete/GeneticAlgorithm.hpp

        src/ruleextraction/RuleSolution.hpp
        src/ruleextraction/RuleSolution.cpp
        src/ruleextraction/RuleExtractor.hpp
        src/ruleextraction/RuleExtractor.cpp

        src/automata/DFA.cpp

        src/python/ModuleBindings.cpp
        src/python/RuleExtractorPython.hpp
)

target_compile_options(hxtk 
    PRIVATE
        $<$<CONFIG:DEBUG>:-Og -ggdb>
        $<$<NOT:$<CONFIG:DEBUG>>:-O2>
)

target_compile_definitions(hxtk
    PUBLIC
        __HX_SUPPORT_BOOST
        __HX_SUPPORT_TBB
)

enable_testing()
add_executable(testsuite)
target_sources(testsuite
    PRIVATE
        test/gtest.cpp
        test/threadpool.cpp
        test/memory.cpp
        test/automata.cpp


        test/optimisation/discrete.cpp

)

target_include_directories(testsuite
    PRIVATE
        include/
        src/
)

target_link_libraries(testsuite 
    Eigen3::Eigen
    GTest::GTest
    Intel::MKL
    Intel::TBB
    hxtk
    ${PYTHON_LIBRARIES}
)

gtest_discover_tests(testsuite)

add_clangformat(hxtk)
add_clangformat(testsuite)
