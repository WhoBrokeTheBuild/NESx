#ifndef NESX_WINDOW_H
#define NESX_WINDOW_H

#include <NESx/NESx.h>
#include <gtk/gtk.h>

#include "Debug/DebugWindow.h"

G_DECLARE_FINAL_TYPE(
    NESxWindow,
    nesx_window,
    NESX, WINDOW,
    GtkApplicationWindow
)

typedef struct _NESxWindow NESxWindow;

struct _NESxWindow
{
    GtkApplicationWindow parent;

    GtkMenuBar * menubar;
    GtkGLArea * glarea;

    bool fullscreen;

    NESxDebugWindow * wndDebug;

    unsigned shader;
    unsigned vao;

};

GtkWidget * nesx_window_new();

void nesx_window_set_scale(NESxWindow * self, int scale);

void nesx_window_toggle_fullscreen(NESxWindow * self);

#endif // NESX_WINDOW_H