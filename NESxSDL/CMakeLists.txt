
FILE(
    GLOB_RECURSE
    private
    "Private/*.h"
    "Private/*.c"
)

SET(target NESxSDL)

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
        # Disable Visual Studio's "not secure" warnings
        $<$<C_COMPILER_ID:MSVC>:_CRT_SECURE_NO_WARNINGS>
)

TARGET_LINK_LIBRARIES(
    ${target}
    PUBLIC
        libNESx
        cflags::cflags
        SDL2::SDL2
)
