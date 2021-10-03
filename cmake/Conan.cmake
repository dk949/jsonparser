set(CONAN_CMAKE_VERSION 0.16)

if (DEFINED ENV{CONAN_USER_HOME})
    set(CONAN_CMAKE_LOCATION $ENV{CONAN_USER_HOME}/.conan/${CONAN_CMAKE_VERSION}.conan.cmake)
elseif (EXISTS ENV{HOME}/.conan/)
    set(CONAN_CMAKE_LOCATION $ENV{HOME}/.conan/${CONAN_CMAKE_VERSION}.conan.cmake)
else ()
    set(CONAN_CMAKE_LOCATION ${CMAKE_BINARY_DIR}/${CONAN_CMAKE_VERSION}.conan.cmake)
endif ()

macro (run_conan)
    # Download automatically, you can also just copy the conan.cmake file
    if (NOT EXISTS ${CONAN_CMAKE_LOCATION})
        message(STATUS "Downloading conan.cmake from https://github.com/conan-io/cmake-conan")
        file(DOWNLOAD "https://github.com/conan-io/cmake-conan/raw/v${CONAN_CMAKE_VERSION}/conan.cmake"
             ${CONAN_CMAKE_LOCATION}
        )
    endif ()

    include(${CONAN_CMAKE_LOCATION})
    set(ENV{CONAN_REVISIONS_ENABLED} 1)

    conan_add_remote(
        NAME
        bincrafters
        URL
        https://bincrafters.jfrog.io/artifactory/api/conan/public-conan
    )

    conan_cmake_run(
        REQUIRES
        ${CONAN_EXTRA_REQUIRES}
        catch2/2.13.6
        spdlog/1.8.5
        ms-gsl/3.1.0
        OPTIONS
        ${CONAN_EXTRA_OPTIONS}
        BASIC_SETUP
        CMAKE_TARGETS # individual targets to link to
        BUILD
        missing
    )
endmacro ()
