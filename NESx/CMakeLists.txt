
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

SET_TARGET_PROPERTIES(
    ${target}
    PROPERTIES
        C_STANDARD 11
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
