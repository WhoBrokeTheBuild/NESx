CMAKE_MINIMUM_REQUIRED(VERSION 3.15 FATAL_ERROR)

# Allow for custom CMake modules
LIST(INSERT CMAKE_MODULE_PATH 0 ${CMAKE_SOURCE_DIR}/CMake)

INCLUDE(SourceFolders)
INCLUDE(Version)

# Enable testing if we are not a subdirectory
IF(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    INCLUDE(CTest)
ENDIF()

###
### Project
###

PROJECT(
    NESx
    LANGUAGES C
    DESCRIPTION "A Toy Nintendo Entertainment System Emulator"
    VERSION ${PROJECT_VERSION}
)

###
### Options
###

OPTION(BUILD_TOOLS      "Build Tools"                               OFF)
OPTION(BUILD_THIRDPARTY "Build Missing Third-Party Dependencies"    ON)

###
### Configuration
###

# Allow for custom organization of files in IDEs
SET_PROPERTY(GLOBAL PROPERTY USE_FOLDERS ON)

IF(BUILD_TESTING)
    ENABLE_TESTING()
ENDIF()

###
### Macros
###

MACRO(DOWNLOAD_IF_NOT_EXIST URL FILENAME)
    IF(NOT EXISTS ${FILENAME})
        FILE(DOWNLOAD ${URL} ${FILENAME})
    ENDIF()
ENDMACRO()

###
### Third Party Dependencies
###

IF(BUILD_THIRDPARTY)
    SET(THIRDPARTY_SOURCE_DIR ${CMAKE_SOURCE_DIR}/ThirdParty)
    SET(THIRDPARTY_BINARY_DIR ${CMAKE_BINARY_DIR}/ThirdParty)

    FILE(MAKE_DIRECTORY ${THIRDPARTY_BINARY_DIR})

    # Set _ROOT variables for config-based packages
    SET(cflags_ROOT     ${THIRDPARTY_BINARY_DIR}/cflags)
    SET(SDL2_ROOT       ${THIRDPARTY_BINARY_DIR}/SDL2)

    EXECUTE_PROCESS(
        COMMAND ${CMAKE_COMMAND} 
            -G "${CMAKE_GENERATOR}" 
            -A "${CMAKE_GENERATOR_PLATFORM}"
            -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
            ${THIRDPARTY_SOURCE_DIR}
        WORKING_DIRECTORY ${THIRDPARTY_BINARY_DIR}
        RESULT_VARIABLE _result
    )

    IF(_result)
        MESSAGE("${_result}")
        MESSAGE(FATAL_ERROR "Failed to configure ThirdParty projects")
    ENDIF()

    SET(THIRDPARTY_BUILD_COMMAND ${CMAKE_COMMAND} --build . )
    IF(CMAKE_GENERATOR STREQUAL "Unix Makefiles")
        # If we are using make, build in parallel
        SET(THIRDPARTY_BUILD_COMMAND ${CMAKE_MAKE_PROGRAM} -l )
    ENDIF()

    EXECUTE_PROCESS(
        COMMAND ${THIRDPARTY_BUILD_COMMAND}
        WORKING_DIRECTORY ${THIRDPARTY_BINARY_DIR}
        RESULT_VARIABLE _result
    )

    IF(_result)
        MESSAGE("${_result}")
        MESSAGE(FATAL_ERROR "Failed to build ThirdParty projects")
    ENDIF()
ENDIF()

FIND_PACKAGE(cflags CONFIG REQUIRED)
FIND_PACKAGE(SDL2 2.0.6 CONFIG QUIET)

FIND_PACKAGE(GTK3 QUIET)

FIND_PACKAGE(ClangFormat)

###
### Automation
###

IF(ClangFormat_FOUND)
    ADD_CUSTOM_TARGET(format)

    SET_TARGET_PROPERTIES(
        format
        PROPERTIES 
            FOLDER "Automation"
    )
ENDIF()

###
### Libraries
###

ADD_SUBDIRECTORY(libMOS6502)
ADD_SUBDIRECTORY(libNESx)

###
### Executables
###

IF(BUILD_TOOLS)
    ADD_SUBDIRECTORY(Assembler)
ENDIF()

ADD_SUBDIRECTORY(NESx)

# Set VisualStudio startup project
#SET_PROPERTY(DIRECTORY ${CMAKE_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT "")

###
### Install
###

INSTALL(
    TARGETS 
        libMOS6502
        libNESx
        NESx
    EXPORT NESxTargets
)

INCLUDE(CMakePackageConfigHelpers)

WRITE_BASIC_PACKAGE_VERSION_FILE(
    ${CMAKE_BINARY_DIR}/NESxConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion
)

INSTALL(
    EXPORT NESxTargets
    NAMESPACE NESx::
    DESTINATION lib/cmake/NESx
)

CONFIGURE_PACKAGE_CONFIG_FILE(
    ${CMAKE_SOURCE_DIR}/CMake/NESxConfig.cmake.in
    ${CMAKE_BINARY_DIR}/NESxConfig.cmake
    INSTALL_DESTINATION lib/cmake/NESx
)

FIND_PACKAGE(PkgConfig QUIET)

IF(PKG_CONFIG_FOUND)
    CONFIGURE_FILE(
        ${CMAKE_SOURCE_DIR}/CMake/MOS6502.pc.in
        ${CMAKE_BINARY_DIR}/MOS6502.pc
    )

    INSTALL(
        FILES
        ${CMAKE_BINARY_DIR}/MOS6502.pc
        DESTINATION share/pkgconfig
    )
ENDIF()