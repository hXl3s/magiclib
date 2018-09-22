set (INTEL_COMPILER_DIR "/opt/intel" CAHCE STRING
	"Root directory of the Intel Compiler Suite")
set (INTEL_TBB_DIR "/usr" CACHE STRING
	"Root directory of the Intel TBB")

include (FindPackageHandleStandardArgs)

if (EXISTS ${INTEL_TBB_DIR})
	find_path (TBB_INCLUDE_DIR tbb.h
		PATH ${INTEL_TBB_DIR}/include/tbb)
	
	find_library(TBB_LIBRARY tbb
		PATHS ${INTEL_TBB_DIR}/lib)
endif ()

find_package_handle_standard_args(TBB DEFAULT_MSG
	TBB_INCLUDE_DIR TBB_LIBRARY)

if (TBB_FOUND)
    set (TBB_LIBRARIES ${TBB_LIBRARY})

    if (NOT TARGET Intel::TBB)
        add_library(Intel::TBB SHARED IMPORTED)
        set_target_properties(Intel::TBB PROPERTIES
            IMPORTED_LOCATION ${TBB_LIBRARIES}
            INTERFACE_INCLUDE_DIRECTORY ${TBB_INCLUDE_DIR}    
        )
    endif ()
endif ()

