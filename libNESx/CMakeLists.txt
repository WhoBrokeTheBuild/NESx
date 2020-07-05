
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

SET(target libNESx)

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
        OUTPUT_NAME "NESx"
)

TARGET_INCLUDE_DIRECTORIES(
    ${target}
    PUBLIC
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Public>
    PRIVATE
        Private
)

TARGET_LINK_LIBRARIES(
    ${target}
    PUBLIC
        libMOS6502
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
    # Test ROMs and assets
    DOWNLOAD_IF_NOT_EXIST(
        "http://nickmass.com/images/nestest.nes"
        ${CMAKE_CURRENT_BINARY_DIR}/nestest.nes
    )
    DOWNLOAD_IF_NOT_EXIST(
        "https://www.qmtpro.com/~nes/misc/nestest.log"
        ${CMAKE_CURRENT_BINARY_DIR}/nestest.log
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
        )

        TARGET_LINK_LIBRARIES(
            ${test_target}
            PRIVATE
                libNESx
        )

        ADD_TEST(
            NAME "${test_target}"
            COMMAND $<TARGET_FILE:${test_target}>
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        )
    ENDFOREACH()
ENDIF()