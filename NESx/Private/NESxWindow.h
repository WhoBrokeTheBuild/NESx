#ifndef NESX_WINDOW_H
#define NESX_WINDOW_H

#include <NESx/NESx.h>
#include <gtk/gtk.h>

#include "NESxDebugger.h"

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

    bool open;
    bool running;
    
    nesx_t * nes;

    GtkMenuBar * menubar;
    GtkGLArea * glarea;

    bool fullscreen;

    NESxDebugger * debugger;

    unsigned shader;
    unsigned texture;
    unsigned vao;

    uint8_t * pixels;

};

GtkWidget * nesx_window_new();

void nesx_window_set_scale(NESxWindow * self, int scale);

void nesx_window_toggle_fullscreen(NESxWindow * self);

void nesx_window_run(NESxWindow * self);

#endif // NESX_WINDOW_H