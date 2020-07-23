
FILE(
    GLOB_RECURSE
    private
    "Private/*.h"
    "Private/*.c"
)

FILE(
    GLOB_RECURSE
    resources
    "Resources/*.glade"
    "Resources/*.xml"
)

SET(resources_dir   ${CMAKE_CURRENT_SOURCE_DIR}/Resources)
SET(gresource       ${resources_dir}/nesx.gresource.xml)
SET(resource_header ${CMAKE_CURRENT_BINARY_DIR}/Resource.h)
SET(resource_source ${CMAKE_CURRENT_BINARY_DIR}/Resource.c)

ADD_CUSTOM_COMMAND(
    COMMAND ${GLIB_COMPILE_RESOURCES_PROGRAM} ${gresource} --target ${resource_header} --generate-header
    OUTPUT ${resource_header}
    DEPENDS ${resources}
    WORKING_DIRECTORY ${resources_dir}
)

ADD_CUSTOM_COMMAND(
    COMMAND ${GLIB_COMPILE_RESOURCES_PROGRAM} ${gresource} --target ${resource_source} --generate-source
    OUTPUT ${resource_source}
    DEPENDS ${resources}
    WORKING_DIRECTORY ${resources_dir}
)

SET(target NESx)

ADD_EXECUTABLE(
    ${target}
    ${private}
    ${resource_header}
    ${resource_source}
)

SET_SOURCE_FOLDERS(${private} ${resources})

SET_TARGET_PROPERTIES(
    ${target}
    PROPERTIES
        C_STANDARD 11
)

TARGET_INCLUDE_DIRECTORIES(
    ${target}
    PRIVATE
        Private
        ${CMAKE_CURRENT_BINARY_DIR}
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
        GTK3::GTK3
        portaudio
)
