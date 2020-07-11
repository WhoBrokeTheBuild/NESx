CMAKE_MINIMUM_REQUIRED(VERSION 3.15 FATAL_ERROR)

LIST(INSERT CMAKE_MODULE_PATH 0 "${CMAKE_SOURCE_DIR}/../CMake")

INCLUDE(ExternalProject)

PROJECT(ThirdParty)

# cflags

SET(cflags_ROOT       ${CMAKE_BINARY_DIR}/cflags)
SET(cflags_SOURCE_DIR ${CMAKE_BINARY_DIR}/cflags_src)

FIND_PACKAGE(cflags CONFIG QUIET)

IF(NOT cflags_FOUND)
    ExternalProject_Add(
        ThirdParty_cflags
        GIT_REPOSITORY      "https://github.com/WhoBrokeTheBuild/cflags.git"
        GIT_TAG             "master"
        GIT_SHALLOW         TRUE
        GIT_PROGRESS        FALSE
        SOURCE_DIR          ${cflags_SOURCE_DIR}
        CMAKE_ARGS          -DCMAKE_INSTALL_PREFIX=${cflags_ROOT}
        BUILD_COMMAND       ""
        UPDATE_COMMAND      ""
    )
ENDIF()

# SDL2

SET(SDL2_ROOT       ${CMAKE_BINARY_DIR}/SDL2)
SET(SDL2_SOURCE_DIR ${CMAKE_BINARY_DIR}/SDL2_src)
SET(SDL2_BINARY_DIR ${CMAKE_BINARY_DIR}/SDL2_bin)

FIND_PACKAGE(SDL2 2.0.6 CONFIG QUIET)

IF(NOT SDL2_FOUND)
    ExternalProject_Add(
        ThirdParty_SDL2
        URL                 "https://www.libsdl.org/release/SDL2-2.0.12.tar.gz"
        URL_HASH            MD5=783b6f2df8ff02b19bb5ce492b99c8ff
        SOURCE_DIR          ${SDL2_SOURCE_DIR}
        BINARY_DIR          ${SDL2_BINARY_DIR}
        CMAKE_ARGS          -DCMAKE_INSTALL_PREFIX=${SDL2_ROOT}
                            -DBUILD_SHARED_LIBS=ON
                            BUILD_COMMAND       ""
                            UPDATE_COMMAND      ""
                            TEST_COMMAND        ""
                            INSTALL_COMMAND     ${CMAKE_COMMAND} --build . --target install --config Release
                            COMMAND             ${CMAKE_COMMAND} --build . --target install --config Debug
                            PATCH_COMMAND       ${SDL2_PATCH_COMMAND}
    )
ENDIF()