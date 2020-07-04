# FindClangFormat.cmake
#
# Finds the Clang Formatter
#
# This will define the following variables
#
#   ClangFormat_FOUND
#   ClangFormat_PROGRAM
#
# The following variables can be set as arguments
#
#   ClangFormat_ROOT_DIR
#

FIND_PROGRAM(
    ClangFormat_PROGRAM
    clang-format
    PATHS
        ${ClangFormat_ROOT_DIR}
    PATH_SUFFIXES
        bin
)

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(
    ClangFormat
    HANDLE_COMPONENTS
    REQUIRED_VARS 
        ClangFormat_PROGRAM
)

MARK_AS_ADVANCED(
    ClangFormat_PROGRAM
)