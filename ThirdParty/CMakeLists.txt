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
