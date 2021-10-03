set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

configure_file(
    "${PROJECT_SOURCE_DIR}/cmake/ModCompileCommands.cmake.in" "${CMAKE_BINARY_DIR}/ModCompileCommands.cmake" @ONLY
)
if (CMAKE_EXPORT_COMPILE_COMMANDS AND NOT MSVC)
    add_custom_target(
        fix_compile_commands ALL COMMAND ${CMAKE_COMMAND} -P "${CMAKE_BINARY_DIR}/ModCompileCommands.cmake"
    )
endif ()

macro (link_compile_commands)
    set(current_list_path ${CMAKE_CURRENT_LIST_DIR})

    string(
        REGEX
        REPLACE "\/|\\|\ "
                "_"
                dir
                ${current_list_path}
    )

    if (CMAKE_EXPORT_COMPILE_COMMANDS AND NOT MSVC)
        add_custom_target(
            ${dir}_compile_commands ALL
            COMMAND ${CMAKE_COMMAND} -E create_symlink ${CMAKE_BINARY_DIR}/compile_commands.json
                    ${current_list_path}/compile_commands.json
        )
    endif ()

    unset(current_list_path)
    unset(dir)
endmacro (link_compile_commands)
