set (INTEL_COMPILER_DIR "/opt/intel" CAHCE STRING
    "Root directory of the Intel Compiler Suite")
set (INTEL_MKL_DIR "/opt/intel/mkl" CACHE STRING
    "Root directory of the Intel MKL")

include (FindPackageHandleStandardArgs)

if (EXISTS ${INTEL_MKL_DIR})
    find_path (MKL_INCLUDE_DIR mkl.h
        PATH ${INTEL_MKL_DIR}/include)
    
    find_library(MKL_CORE_LIBRARY mkl_rt
        PATHS ${INTEL_MKL_DIR}/lib/intel64/)
endif ()

find_package_handle_standard_args(MKL DEFAULT_MSG
    MKL_INCLUDE_DIR  MKL_CORE_LIBRARY)

if (MKL_FOUND)
    set (MKL_LIBRARIES ${MKL_CORE_LIBRARY})
    set (MKL_INCLUDE_DIR ${MKL_INCLUDE_DIR})

    if (NOT TARGET Intel::MKL)
        add_library(Intel::MKL INTERFACE IMPORTED)
        target_link_libraries(Intel::MKL INTERFACE ${MKL_LIBRARIES})
        target_include_directories(Intel::MKL INTERFACE ${MKL_INCLUDE_DIR})
    endif ()
endif ()

