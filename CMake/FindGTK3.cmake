# FindGTK3.cmake
#
# Finds the GNOME GTK framework
#
# This will define the following variables
#
# GTK3_INCLUDE_DIR
# GTK3_LIBRARY
# GDK3_LIBRARY
# GDK_PIXBUF2_INCLUDE_DIR
# GDK_PIXBUF2_LIBRARY
# GLIB2_INCLUDE_DIR
# GLIB2_GLIBCONFIG_INCLUDE_DIR
# GLIB2_LIBRARY
# GLIB2_GOBJECT_LIBRARY
# ATK1_INCLUDE_DIR
# ATK1_LIBRARY
# PANGO1_INCLUDE_DIR
# PANGO1_LIBRARY
# PANGOCAIRO1_LIBRARY
# CAIRO_INCLUDE_DIR
# CAIRO_LIBRARY
# GLIB_COMPILE_RESOURCES_PROGRAM
#
# and the following imported targets
#
#   GTK3::GTK3
#
# The following variables can be set as arguments
#
#   GTK3_ROOT_DIR
#

FIND_PACKAGE(PkgConfig QUIET)

PKG_CHECK_MODULES(_GTK3_PC QUIET gtk+-3.0)

# GTK 3.0

FIND_PATH(
    GTK3_INCLUDE_DIR
    NAMES gtk/gtk.h
    PATHS 
        ${GTK3_ROOT_DIR}
        ${_GTK3_PC_INCLUDE_DIRS}
    PATH_SUFFIXES 
        include
)

FIND_LIBRARY(
    GTK3_LIBRARY
    NAMES gtk-3 gtk-3.0
    PATHS
        ${GTK3_ROOT_DIR}
        ${_GTK3_PC_LIBRARY_DIRS}
    PATH_SUFFIXES
        lib
)

FIND_LIBRARY(
    GDK3_LIBRARY
    NAMES gdk-3 gtk-3.0
    PATHS
        ${GTK3_ROOT_DIR}
        ${_GTK3_PC_LIBRARY_DIRS}
    PATH_SUFFIXES
        lib
)

# GDK Pixbuf 2.0

FIND_PATH(
    GDK_PIXBUF2_INCLUDE_DIR
    NAMES gdk-pixbuf/gdk-pixbuf.h
    PATHS 
        ${GTK3_ROOT_DIR}
        ${_GTK3_PC_INCLUDE_DIRS}
    PATH_SUFFIXES 
        include
)

FIND_LIBRARY(
    GDK_PIXBUF2_LIBRARY
    NAMES gdk_pixbuf-2.0
    PATHS
        ${GTK3_ROOT_DIR}
        ${_GTK3_PC_LIBRARY_DIRS}
    PATH_SUFFIXES
        lib
)

# GLib 2.0

FIND_PATH(
    GLIB2_INCLUDE_DIR
    NAMES glib.h
    PATHS 
        ${GTK3_ROOT_DIR}
        ${_GTK3_PC_INCLUDE_DIRS}
    PATH_SUFFIXES 
        include
)

FIND_PATH(
    GLIB2_GLIBCONFIG_INCLUDE_DIR
    NAMES glibconfig.h
    PATHS
        ${GTK3_ROOT_DIR}
        ${_GTK3_PC_INCLUDE_DIRS}
    PATH_SUFFIXES
        include
)

FIND_LIBRARY(
    GLIB2_LIBRARY
    NAMES glib-2.0
    PATHS
        ${GTK3_ROOT_DIR}
        ${_GTK3_PC_LIBRARY_DIRS}
    PATH_SUFFIXES
        lib
)

FIND_LIBRARY(
    GLIB2_GOBJECT_LIBRARY
    NAMES gobject-2.0
    PATHS
        ${GTK3_ROOT_DIR}
        ${_GTK3_PC_LIBRARY_DIRS}
    PATH_SUFFIXES
        lib
)

FIND_LIBRARY(
    GIO2_LIBRARY
    NAMES gio-2.0
    PATHS
        ${GTK3_ROOT_DIR}
        ${_GTK3_PC_LIBRARY_DIRS}
    PATH_SUFFIXES
        lib
)

# ATK 1.0

FIND_PATH(
    ATK1_INCLUDE_DIR
    NAMES atk/atk.h
    PATHS 
        ${GTK3_ROOT_DIR}
        ${_GTK3_PC_INCLUDE_DIRS}
    PATH_SUFFIXES 
        include
)

FIND_LIBRARY(
    ATK1_LIBRARY
    NAMES atk-1.0
    PATHS
        ${GTK3_ROOT_DIR}
        ${_GTK3_PC_LIBRARY_DIRS}
    PATH_SUFFIXES
        lib
)

# Pango 1.0

FIND_PATH(
    PANGO1_INCLUDE_DIR
    NAMES pango/pango.h
    PATHS
        ${GTK3_ROOT_DIR}
        ${_GTK3_PC_INCLUDE_DIRS}
    PATH_SUFFIXES
        include
)

FIND_LIBRARY(
    PANGO1_LIBRARY
    NAMES pango-1.0
    PATHS
        ${GTK3_ROOT_DIR}
        ${_GTK3_PC_LIBRARY_DIRS}
    PATH_SUFFIXES
        lib
)

FIND_LIBRARY(
    PANGOCAIRO1_LIBRARY
    NAMES pangocairo-1.0
    PATHS
        ${GTK3_ROOT_DIR}
        ${_GTK3_PC_LIBRARY_DIRS}
    PATH_SUFFIXES
        lib
)

# Cairo

FIND_PATH(
    CAIRO_INCLUDE_DIR
    NAMES cairo.h
    PATHS
        ${GTK3_ROOT_DIR}
        ${_GTK3_PC_INCLUDE_DIRS}
    PATH_SUFFIXES
        include
)

FIND_LIBRARY(
    CAIRO_LIBRARY
    NAMES cairo
    PATHS
        ${GTK3_ROOT_DIR}
        ${_GTK3_PC_LIBRARY_DIRS}
    PATH_SUFFIXES
        lib
)

# glib-compile-resources

FIND_PROGRAM(
    GLIB_COMPILE_RESOURCES_PROGRAM
    NAMES glib-compile-resources
    PATHS
        ${GTK3_ROOT_DIR}
    PATH_SUFFIXES
        bin
)

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(
    GTK3
    REQUIRED_VARS 
        GTK3_INCLUDE_DIR
        GTK3_LIBRARY
        GDK3_LIBRARY
        GDK_PIXBUF2_INCLUDE_DIR
        GDK_PIXBUF2_LIBRARY
        GLIB2_INCLUDE_DIR
        GLIB2_GLIBCONFIG_INCLUDE_DIR
        GLIB2_LIBRARY
        GLIB2_GOBJECT_LIBRARY
        ATK1_INCLUDE_DIR
        ATK1_LIBRARY
        PANGO1_INCLUDE_DIR
        PANGO1_LIBRARY
        PANGOCAIRO1_LIBRARY
        CAIRO_INCLUDE_DIR
        CAIRO_LIBRARY
        GLIB_COMPILE_RESOURCES_PROGRAM
)

IF(GTK3_FOUND)
    SET(GTK3_ALL_INCLUDE_DIRS
        ${GTK3_INCLUDE_DIR}
        ${GDK_PIXBUF2_INCLUDE_DIR}
        ${GLIB2_INCLUDE_DIR}
        ${GLIB2_GLIBCONFIG_INCLUDE_DIR}
        ${ATK1_INCLUDE_DIR}
        ${PANGO1_INCLUDE_DIR}
        ${CAIRO_INCLUDE_DIR}
    )

    SET(GTK3_ALL_LIBRARIES
        ${GTK3_LIBRARY}
        ${GDK3_LIBRARY}
        ${GDK_PIXBUF2_LIBRARY}
        ${GLIB2_LIBRARY}
        ${GLIB2_GOBJECT_LIBRARY}
        ${GIO2_LIBRARY}
        ${ATK1_LIBRARY}
        ${PANGO1_LIBRARY}
        ${PANGOCAIRO1_LIBRARY}
        ${CAIRO_LIBRARY}
    )

    ADD_LIBRARY(GTK3::GTK3 INTERFACE IMPORTED)
    SET_TARGET_PROPERTIES(
        GTK3::GTK3 PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${GTK3_ALL_INCLUDE_DIRS}"
        INTERFACE_LINK_LIBRARIES "${GTK3_ALL_LIBRARIES}"
    )
ENDIF()
