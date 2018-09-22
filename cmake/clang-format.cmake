find_program(CLANG_FORMAT "clang-format")
find_package(ClangFormat REQUIRED)

function (add_clangformat _targetname)
    if (CLANG_FORMAT_FOUND)
        if (NOT TARGET ${_targetname})
            message (FATAL_ERROR "add_clangformat called on non target variable")
        endif ()

        get_target_property (_clang_sources ${_targetname} SOURCES)
        get_target_property (_builddir ${_targetname} BINARY_DIR)

        set (_sources "")
        foreach (_source ${_clang_sources})
            if (NOT TARGET ${_source})
                get_filename_component (_source_file ${_source} NAME)
                get_source_file_property (_clang_loc "${_source}" LOCATION)

                list (APPEND _sources ${_source})
            endif ()
        endforeach ()

        if (_sources)
            add_custom_target(${_targetname}_clangformat
                COMMAND ${CLANG_FORMAT_EXECUTABLE} -style=file -i ${_sources}
                COMMENT "Clang-Format for target ${_target}"
                WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")

            add_dependencies(${_targetname} ${_targetname}_clangformat)
        endif ()
    endif ()
endfunction()
