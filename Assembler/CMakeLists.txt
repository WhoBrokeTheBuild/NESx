
FILE(
    GLOB_RECURSE
    private
    "Private/*.h"
    "Private/*.c"
)

SET(target NESxASM)

ADD_EXECUTABLE(
    ${target}
    ${private}
)

SET_TARGET_PROPERTIES(
    ${target}
    PROPERTIES
        C_STANDARD 11
)

TARGET_LINK_LIBRARIES(
    ${target}
    PUBLIC
        cflags::cflags
)