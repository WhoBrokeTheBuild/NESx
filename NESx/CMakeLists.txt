
FILE(
    GLOB_RECURSE
    private
    "Private/*.h"
    "Private/*.c"
)

SET(target NESx)

ADD_EXECUTABLE(
    ${target}
    ${private}
)

SET_SOURCE_FOLDERS(${public} ${private})

SET_TARGET_PROPERTIES(
    ${target}
    PROPERTIES
        C_STANDARD 11
        PUBLIC_HEADER "${public}"
)

TARGET_INCLUDE_DIRECTORIES(
    ${target}
    PRIVATE
        Private
)

TARGET_LINK_LIBRARIES(
    ${target}
    PUBLIC
        MOS6502
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
    # Remove Main.c
    LIST(REMOVE_ITEM private "${CMAKE_CURRENT_SOURCE_DIR}/Private/Main.c")

    INCLUDE(ExternalProject)

    ExternalProject_Add(
        TestData_instr
        URL                     "http://blargg.8bitalley.com/nes-tests/instr_test-v5.zip"
        BUILD_IN_SOURCE         TRUE
        DOWNLOAD_NO_PROGRESS    TRUE
        CONFIGURE_COMMAND       ""
        BUILD_COMMAND           ""
        INSTALL_COMMAND         ${CMAKE_COMMAND} -E copy official_only.nes ${CMAKE_CURRENT_BINARY_DIR}
        # INSTALL_COMMAND         ${CMAKE_COMMAND} -E copy all_instrs.nes ${CMAKE_CURRENT_BINARY_DIR}
    )

    FILE(
        GLOB_RECURSE
        tests
        "Tests/*.c"
    )

    FOREACH(test_source ${tests})
        GET_FILENAME_COMPONENT(test_target ${test_source} NAME_WE)
        SET(test_target "${target}_${test_target}")

        ADD_EXECUTABLE(
            ${test_target}
            ${test_source}
            ${private}
        )

        TARGET_LINK_LIBRARIES(
            ${test_target}
            PRIVATE
                MOS6502
        )

        ADD_TEST(
            NAME "${test_target}"
            COMMAND $<TARGET_FILE:${test_target}>
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        )
    ENDFOREACH()
ENDIF()