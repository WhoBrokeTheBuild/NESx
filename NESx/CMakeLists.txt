
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
        SDL2::SDL2
)

IF(GTK3_FOUND)
    TARGET_COMPILE_DEFINITIONS(
        ${target}
        PRIVATE
            NESX_HAVE_GTK3
    )

    TARGET_LINK_LIBRARIES(
        ${target}
        PUBLIC
            GTK3::GTK3
    )

    SET(RESOURCES_DIR   ${CMAKE_CURRENT_SOURCE_DIR}/Resources)
    SET(RESOURCE_CONFIG ${RESOURCES_DIR}/debugger.gresource.xml)

    SET(RESOURCE_HEADER ${CMAKE_CURRENT_BINARY_DIR}/resource.h)
    SET(RESOURCE_SOURCE ${CMAKE_CURRENT_BINARY_DIR}/resource.c)

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

    TARGET_SOURCES(
        ${target}
        PRIVATE
            ${RESOURCE_HEADER}
            ${RESOURCE_SOURCE}
    )

    TARGET_INCLUDE_DIRECTORIES(
        ${target}
        PRIVATE
            ${CMAKE_CURRENT_BINARY_DIR}
    )
ENDIF()

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
