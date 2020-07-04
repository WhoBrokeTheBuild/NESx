
FILE(
    GLOB_RECURSE
    private
    "Private/*.h"
    "Private/*.c"
)

FILE(
    GLOB_RECURSE
    public
    "Public/*.h"
)

SET(target MOS6502)

ADD_LIBRARY(${target} "")

TARGET_SOURCES(
    ${target}
    PRIVATE
        ${private}
        ${public}
)

SET_TARGET_PROPERTIES(
    ${target}
    PROPERTIES
        C_STANDARD 11
)

TARGET_INCLUDE_DIRECTORIES(
    ${target}
    PUBLIC
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Public>
    PRIVATE
        Private
)

# Automation

IF(ClangFormat_FOUND)
    ADD_CUSTOM_TARGET(
        format-${target}
        COMMAND ${ClangFormat_PROGRAM} -i ${public} ${private}
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    )

    SET_TARGET_PROPERTIES(
        format-${target}
        PROPERTIES 
            FOLDER "Automation"
    )

    ADD_DEPENDENCIES(format format-${target})
ENDIF()

# Tests

IF(BUILD_TESTS)
    FILE(
        GLOB_RECURSE
        tests
        "Tests/*.c"
    )

    FOREACH(test_source ${tests})
        GET_FILENAME_COMPONENT(test_target ${test_source} NAME_WE)

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
            NAME "${target}_${test_target}"
            COMMAND $<TARGET_FILE:${test_target}>
            WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/Tests"
        )
    ENDFOREACH()
ENDIF()