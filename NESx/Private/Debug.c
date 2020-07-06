#include "Debug.h"

#if defined(NESX_HAVE_GTK3)

#include <gtk/gtk.h>

bool _DebugRunning = false;

GtkScrolledWindow * _DebugLogScroll = NULL;
GtkTextBuffer * _DebugLogText = NULL;

#define DEBUG_TICKS_PER_IDLE 32

static void _DebugLogState(nesx_t * nes) 
{
    static char line[256];

    snprintf(line, sizeof(line),
        "IR:%02X:%d PC:%04X AB:%04X DB:%02X A:%02X X:%02X Y:%02X S:%02X "
        "P:%c%c%c%c%c%c\n",
        (nes->CPU.IR >> 3), (nes->CPU.IR & 0b111),
        nes->CPU.PC, nes->CPU.AB, nes->CPU.DB, 
        nes->CPU.A, nes->CPU.X, nes->CPU.Y, nes->CPU.S,
        (nes->CPU.FC ? 'C' : 'c'),
        (nes->CPU.FZ ? 'Z' : 'z'),
        (nes->CPU.FI ? 'I' : 'i'),
        (nes->CPU.FD ? 'D' : 'd'),
        (nes->CPU.FV ? 'V' : 'v'),
        (nes->CPU.FN ? 'N' : 'n'));
    
    if (_DebugLogText) {
        GtkTextIter iter;
        gtk_text_buffer_get_end_iter(_DebugLogText, &iter);
        gtk_text_buffer_insert(_DebugLogText, &iter, line, -1);

        GtkAdjustment * vadj = gtk_scrolled_window_get_vadjustment(_DebugLogScroll);
        gtk_adjustment_set_value(vadj, gtk_adjustment_get_upper(vadj));
        gtk_scrolled_window_set_vadjustment(_DebugLogScroll, vadj);
    }
}

static bool _DebugIdle(nesx_t * nes)
{
    NESx_Tick(nes);
    _DebugLogState(nes);
    return _DebugRunning;
}

static void _DebugTick(GtkButton * button, nesx_t * nes)
{
    NESx_Tick(nes);
    _DebugLogState(nes);
}

static void _DebugStep(GtkButton * button, nesx_t * nes)
{
    do {
        NESx_Tick(nes);
        _DebugLogState(nes);
    } 
    while (!nes->CPU.SYNC);
}

static void _DebugPlay(GtkButton * button, nesx_t * nes)
{
    _DebugRunning = true;
    g_idle_add_full(G_PRIORITY_DEFAULT_IDLE, (GSourceFunc)(&_DebugIdle), nes, NULL);
}

static void _DebugStop(GtkButton * button, nesx_t * nes)
{
    _DebugRunning = false;
}

static void _DebugWindowDestroy(GtkWidget *widget, gpointer user_data)
{
    _DebugRunning = false;
}

static void _DebugActivate(GtkApplication * app, nesx_t * nes)
{
    GtkWidget * window;

    window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "NESx");
    gtk_window_set_default_size(GTK_WINDOW(window), 640, 480);
    gtk_widget_show_all(window);

    g_signal_connect(window, "destroy", G_CALLBACK(_DebugWindowDestroy), NULL);

    GtkWidget * cols = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_container_add(GTK_CONTAINER(window), GTK_WIDGET(cols));

    GtkWidget * scroll = gtk_scrolled_window_new(NULL, NULL);
    _DebugLogScroll = GTK_SCROLLED_WINDOW(scroll);
    gtk_box_pack_start(GTK_BOX(cols), scroll, true, true, 0);

    GtkWidget * log = gtk_text_view_new();
    gtk_text_view_set_monospace(GTK_TEXT_VIEW(log), true);
    _DebugLogText = gtk_text_view_get_buffer(GTK_TEXT_VIEW(log));
    gtk_container_add(GTK_CONTAINER(scroll), log);

    GtkWidget * rows = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_box_pack_start(GTK_BOX(cols), rows, false, true, 0);

    gtk_container_set_border_width(GTK_CONTAINER(rows), 10);
    
    // "media-playback-start"
    GtkWidget * play_button = gtk_button_new_with_label("Run");
    g_signal_connect(play_button, "clicked", G_CALLBACK(_DebugPlay), nes);
    gtk_box_pack_start(GTK_BOX(rows), play_button, false, false, 0);
    
    // "media-playback-stop"
    GtkWidget * stop_button = gtk_button_new_with_label("Stop");
    g_signal_connect(stop_button, "clicked", G_CALLBACK(_DebugStop), nes);
    gtk_box_pack_start(GTK_BOX(rows), stop_button, false, false, 0);
    
    // "media-seek-forward"
    GtkWidget * tick_button = gtk_button_new_with_label("Tick");
    g_signal_connect(tick_button, "clicked", G_CALLBACK(_DebugTick), nes);
    gtk_box_pack_start(GTK_BOX(rows), tick_button, false, false, 0);
    
    // "media-skip-forward"
    GtkWidget * step_button = gtk_button_new_with_label("Step");
    g_signal_connect(step_button, "clicked", G_CALLBACK(_DebugStep), nes);
    gtk_box_pack_start(GTK_BOX(rows), step_button, false, false, 0);

    gtk_widget_show_all(window);

    _DebugRunning = false;
}

bool DebugInit(nesx_t * nes, int argc, char ** argv)
{
    int status;
    GtkApplication * app;

    app = gtk_application_new("com.stephenlw.nesx", 
        G_APPLICATION_FLAGS_NONE);
    g_signal_connect(app, "activate", G_CALLBACK(_DebugActivate), nes);

    // TODO: Fix argc/argv
    status = g_application_run(G_APPLICATION(app), 0, NULL);
    g_object_unref(app);

    return true;
}

#else

bool InitDebug(int argc, char ** argv)
{
    (void)argc;
    (void)argv;

    return true;
}

#endif