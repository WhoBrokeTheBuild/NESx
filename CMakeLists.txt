CMAKE_MINIMUM_REQUIRED(VERSION 3.15 FATAL_ERROR)

# Allow for custom CMake modules
LIST(INSERT CMAKE_MODULE_PATH 0 ${CMAKE_SOURCE_DIR}/CMake)

###
### Project
###

PROJECT(
    NESx
    LANGUAGES C
    VERSION 0.0.01
)

###
### Options
###

OPTION(BUILD_TOOLS      "Build Tools"                               OFF)
OPTION(BUILD_TESTS      "Build Tests"                               OFF)
OPTION(BUILD_THIRDPARTY "Build Missing Third-Party Dependencies"    ON)

###
### Configuration
###

# Allow for custom organization of files in VisualStudio
SET_PROPERTY(GLOBAL PROPERTY USE_FOLDERS ON)

IF(BUILD_TESTS)
    ENABLE_TESTING()
ENDIF()

###
### Third Party Dependencies
###

IF(BUILD_THIRDPARTY)
    SET(THIRDPARTY_SOURCE_DIR ${CMAKE_SOURCE_DIR}/ThirdParty)
    SET(THIRDPARTY_BINARY_DIR ${CMAKE_BINARY_DIR}/ThirdParty)

    FILE(MAKE_DIRECTORY ${THIRDPARTY_BINARY_DIR})

    # Set _ROOT variables for config-based packages
    SET(cflags_ROOT ${THIRDPARTY_BINARY_DIR}/cflags)

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

    SET(_THIRDPARTY_BUILD_COMMAND ${CMAKE_COMMAND} --build . )
    IF(CMAKE_GENERATOR STREQUAL "Unix Makefiles")
        SET(_THIRDPARTY_BUILD_COMMAND ${CMAKE_MAKE_PROGRAM} -l )
    ENDIF()

    EXECUTE_PROCESS(
        COMMAND ${_THIRDPARTY_BUILD_COMMAND}
        WORKING_DIRECTORY ${THIRDPARTY_BINARY_DIR}
        RESULT_VARIABLE _result
    )

    IF(_result)
        MESSAGE("${_result}")
        MESSAGE(FATAL_ERROR "Failed to build ThirdParty projects")
    ENDIF()
ENDIF()

FIND_PACKAGE(cflags CONFIG REQUIRED)

FIND_PACKAGE(ClangFormat)

# Automation

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

ADD_SUBDIRECTORY(MOS6502)

###
### Executables
###

IF(BUILD_TOOLS)
    ADD_SUBDIRECTORY(Assembler)
ENDIF()

ADD_SUBDIRECTORY(NESx)

# Set VisualStudio startup project
#SET_PROPERTY(DIRECTORY ${CMAKE_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT "")