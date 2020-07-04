
# Configure IDEs to use proper folders for sources
MACRO(SET_SOURCE_FOLDERS)
    SET(files "${ARGN}")
    FOREACH(file ${files})
        # Get relative path
        FILE(RELATIVE_PATH file_path ${CMAKE_CURRENT_SOURCE_DIR} ${file})
        GET_FILENAME_COMPONENT(folder ${file_path} DIRECTORY)
        # Convert to native `/` or `\` path
        FILE(TO_NATIVE_PATH ${folder} folder)
        # Set the folder in the IDE
        SOURCE_GROUP(${folder} FILES ${file})
    ENDFOREACH()
ENDMACRO()
