
FILE(
    GLOB_RECURSE
    private
    "Private/*.h"
    "Private/*.c"
)

FILE(
    GLOB_RECURSE
    debug
    "Debug/*.h"
    "Debug/*.c"
)

SET(target NESx)
SET(target_debug NESxd)

ADD_EXECUTABLE(
    ${target}
    ${private}
)

SET_SOURCE_FOLDERS(${private} ${debug})

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
        SDL2::SDL2
)

IF(GTK3_FOUND)
    SET(RESOURCES_DIR   ${CMAKE_CURRENT_SOURCE_DIR}/Resources)
    SET(RESOURCE_CONFIG ${RESOURCES_DIR}/debugger.gresource.xml)

    FILE(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/Debug)
    SET(RESOURCE_HEADER ${CMAKE_CURRENT_BINARY_DIR}/Debug/resource.h)
    SET(RESOURCE_SOURCE ${CMAKE_CURRENT_BINARY_DIR}/Debug/resource.c)

    FILE(
        GLOB_RECURSE
        resources
        "Resources/*.glade"
        "Resources/*.xml"
    )

    ADD_CUSTOM_COMMAND(
        COMMAND ${GLIB_COMPILE_RESOURCES_PROGRAM} ${RESOURCE_CONFIG} --target ${RESOURCE_HEADER} --generate-header
        OUTPUT ${RESOURCE_HEADER}
        DEPENDS ${resources}
        WORKING_DIRECTORY ${RESOURCES_DIR}
    )

    ADD_CUSTOM_COMMAND(
        COMMAND ${GLIB_COMPILE_RESOURCES_PROGRAM} ${RESOURCE_CONFIG} --target ${RESOURCE_SOURCE} --generate-source
        OUTPUT ${RESOURCE_SOURCE}
        DEPENDS ${resources}
        WORKING_DIRECTORY ${RESOURCES_DIR}
    )

    ADD_EXECUTABLE(
        ${target_debug}
        ${private}
        ${debug}
        ${RESOURCE_HEADER}
        ${RESOURCE_SOURCE}
    )

    SET_TARGET_PROPERTIES(
        ${target_debug}
        PROPERTIES
            C_STANDARD 11
    )
    
    TARGET_INCLUDE_DIRECTORIES(
        ${target_debug}
        PRIVATE
            Private
            Debug
            ${CMAKE_CURRENT_BINARY_DIR}/Debug
    )

    TARGET_COMPILE_DEFINITIONS(
        ${target_debug}
        PRIVATE
            NESX_DEBUGGER
            # Disable Visual Studio "not secure" warnings
            $<$<C_COMPILER_ID:MSVC>:_CRT_SECURE_NO_WARNINGS>
    )

    TARGET_LINK_LIBRARIES(
        ${target_debug}
        PUBLIC
            libNESx
            cflags::cflags
            SDL2::SDL2
            GTK3::GTK3
    )
ENDIF()

# Automation

IF(ClangFormat_FOUND)
    ADD_CUSTOM_TARGET(
        format-${target}
        COMMAND ${ClangFormat_PROGRAM} -i ${private}
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    )

    SET_TARGET_PROPERTIES(
        format-${target}
        PROPERTIES 
            FOLDER "Automation"
    )

    ADD_DEPENDENCIES(format format-${target})
ENDIF()
