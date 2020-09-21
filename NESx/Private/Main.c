
#include <NESx/NESx.h>
#include <cflags.h>
#include <gtk/gtk.h>

// Removed while my patch is reviewed by the portaudio mailing list
// #include <portaudio.h>

#include "NESxWindow.h"
#include "Resource.h"


int main(int argc, char ** argv)
{
    int status = 0;

    nesx_t nes;

    GError * error = NULL;
    GtkApplication * app = NULL;
    NESxWindow * window = NULL;

    cflags_t * flags = cflags_init();

    bool help = false;
    cflags_add_bool(flags, 'h', "help", &help, "display this help and exit");

    int scale = 1;
    cflags_add_int(flags, 's', "scale", &scale, "set the initial window scale, default is 1");

    bool debug = false;
    cflags_add_bool(flags, 'd', "debug", &debug, "show the debug window");

    cflags_flag_t * verbose = cflags_add_bool(flags, 'v', "verbose", NULL,
        "Enables verbose output, repeat up to 4 times for more verbosity");

    cflags_parse(flags, argc, argv);

    if (help) {
        cflags_print_usage(flags,
            "[OPTION]... [ROM_FILENAME]",
            "A Toy Nintendo Entertainment System Emulator",
            "Additional information about this program can be found by "
            "contacting:\n"
            "  sdl.slane@gmail.com");

        cflags_free(flags);
        return 0;
    }

    nes.Verbosity = verbose->count;
    if (!NESx_Init(&nes)) {
        status = 1;
        goto cleanup;
    }

    app = gtk_application_new("com.stephenlw.nesx", G_APPLICATION_FLAGS_NONE);
    if (!app) {
        fprintf(stderr, "failed to create GTK application\n");
        status = 1;
        goto cleanup;
    }

    if (!g_application_register(G_APPLICATION(app), NULL, &error)) {
        fprintf(stderr, "failed to register GTK application: %s", error->message);
        status = 1;
        goto cleanup;
    }

    window = NESX_WINDOW(nesx_window_new(&nes));
    nesx_window_set_scale(window, scale);

    if (flags->argc > 1) {
        if (!NESx_ROM_Load(&nes, flags->argv[1])) {
            status = 1;
            goto cleanup;
        }

        NESx_ROM_PrintHeader(&nes);
    }
    
    gtk_widget_show_all(GTK_WIDGET(window));

    nesx_window_run(window);

cleanup:

    if (error) {
        g_error_free(error);
    }

    if (app) {
        g_object_unref(app);
    }

    NESx_Term(&nes);

    cflags_free(flags);

    return status;
}
