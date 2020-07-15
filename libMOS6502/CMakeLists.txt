
FILE(
    GLOB_RECURSE
    public
    "Public/*.h"
)

FILE(
    GLOB_RECURSE
    private
    "Private/*.h"
    "Private/*.c"
)

SET(target libMOS6502)

ADD_LIBRARY(
    ${target}
    ${public}
    ${private}
)

SET_SOURCE_FOLDERS(${public} ${private})

SET_TARGET_PROPERTIES(
    ${target}
    PROPERTIES
        C_STANDARD 11
        OUTPUT_NAME "MOS6502"
)

TARGET_INCLUDE_DIRECTORIES(
    ${target}
    PUBLIC
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Public>
    PRIVATE
        Private
)

TARGET_COMPILE_OPTIONS(
    ${target}
    PUBLIC
        # Disable unknown pragmas warning
        $<$<C_COMPILER_ID:GNU>:-Wall -Wno-unknown-pragmas>
        $<$<C_COMPILER_ID:Clang>:-Wall -Wno-unknown-pragmas>
        $<$<C_COMPILER_ID:MSVC>:/wd4068>
        # Enable mutlithreaded builds
        $<$<C_COMPILER_ID:MSVC>:/MP>
)

TARGET_COMPILE_DEFINITIONS(
    ${target}
    PRIVATE
        # Disable Visual Studio "not secure" warnings
        $<$<C_COMPILER_ID:MSVC>:_CRT_SECURE_NO_WARNINGS>
)

# Install

INSTALL(
    FILES ${public}
    DESTINATION include/NESx/MOS6502
)

# Tests

FILE(
    GLOB_RECURSE
    tests
    "Tests/*.c"
)

IF(BUILD_TESTING)
    FOREACH(test_source ${tests})
        GET_FILENAME_COMPONENT(test_target ${test_source} NAME_WE)
        SET(test_target "${target}_${test_target}")

        ADD_EXECUTABLE(
            ${test_target}
            ${test_source}
        )

        TARGET_LINK_LIBRARIES(
            ${test_target}
            PRIVATE
                ${target}
        )

        ADD_TEST(
            NAME "${test_target}"
            COMMAND $<TARGET_FILE:${test_target}>
            WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/Tests"
        )
    ENDFOREACH()
ENDIF()

# Automation

IF(ClangFormat_FOUND)
    ADD_CUSTOM_TARGET(
        format-${target}
        COMMAND ${ClangFormat_PROGRAM} -i ${public} ${private} ${tests}
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    )

    SET_TARGET_PROPERTIES(
        format-${target}
        PROPERTIES 
            FOLDER "Automation"
    )

    ADD_DEPENDENCIES(format format-${target})
ENDIF()