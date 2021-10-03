function (enable_doxygen)
    option(ENABLE_DOXYGEN "Enable doxygen doc builds of source" OFF)
    if (ENABLE_DOXYGEN)
        set(DOXYFILE_SUFFIX doxygen-docs)

        set(DOXYGEN_CALLER_GRAPH YES)
        set(DOXYGEN_CALL_GRAPH YES)
        set(DOXYGEN_EXTRACT_ALL YES)
        set(DOXYGEN_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/docs")

        find_package(Doxygen REQUIRED dot)
        doxygen_add_docs(${DOXYFILE_SUFFIX} ${PROJECT_SOURCE_DIR})

    endif ()
endfunction ()
