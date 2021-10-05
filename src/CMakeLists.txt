# Generate compile_commands.json to make it easier to work with clang based tools
link_compile_commands()

set(EXE_NAME json)

file(
    GLOB
    SOURCE_FILES
    "${CMAKE_SOURCE_DIR}/src"
    "*.cpp"
)

file(
    GLOB
    HEADER_FILES
    "${CMAKE_SOURCE_DIR}/src"
    "*.hpp"
)

add_executable(${EXE_NAME} ${SOURCE_FILES} ${HEADER_FILES})

target_link_libraries(${EXE_NAME} PRIVATE project_options project_warnings)
