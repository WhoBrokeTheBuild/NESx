
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

SET_SOURCE_FOLDERS(${private})

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

TARGET_COMPILE_DEFINITIONS(
    ${target}
    PRIVATE
        # Disable Visual Studio "not secure" warnings
        $<$<C_COMPILER_ID:MSVC>:_CRT_SECURE_NO_WARNINGS>
)

TARGET_LINK_LIBRARIES(
    ${target}
    PUBLIC
        libNESx
        cflags::cflags
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
