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

target_include_directories(
    ${EXE_NAME} SYSTEM PRIVATE ${CONAN_INCLUDE_DIRS_SPDLOG} ${CONAN_INCLUDE_DIRS_MS-GSL} ${CONAN_INCLUDE_DIRS_BOOST}
)
target_link_libraries(
    ${EXE_NAME}
    PRIVATE project_options
            project_warnings
            CONAN_PKG::spdlog
            CONAN_PKG::ms-gsl
            ${BOOST_PKG}
)

target_compile_options(${EXE_NAME} PRIVATE "-fconcepts-diagnostics-depth=2")
