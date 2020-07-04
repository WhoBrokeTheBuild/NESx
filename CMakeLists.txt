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

# TODO

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

ADD_SUBDIRECTORY(NESx)

# Set VisualStudio startup project
#SET_PROPERTY(DIRECTORY ${CMAKE_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT "")