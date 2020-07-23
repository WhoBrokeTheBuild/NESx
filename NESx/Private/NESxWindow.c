#include "NESxWindow.h"

#include "Resource.h"

G_DEFINE_TYPE(NESxWindow, nesx_window, GTK_TYPE_APPLICATION_WINDOW)

GtkWidget * nesx_window_new()
{
    NESxWindow * window;
    window = NESX_WINDOW(g_object_new(nesx_window_get_type(), NULL));
    return GTK_WIDGET(window);
}

void nesx_window_init(NESxWindow * self)
{
    gtk_widget_init_template(GTK_WIDGET(self));

    self->fullscreen = false;
    self->menubarHeight = 0;
}

void nesx_window_set_scale(NESxWindow * self, int scale)
{
    gtk_widget_set_size_request(GTK_WIDGET(self->glarea), NESX_WIDTH * scale, NESX_HEIGHT * scale);
    gtk_window_resize(GTK_WINDOW(self), 1, 1);
}

void nesx_window_toggle_fullscreen(NESxWindow * self)
{
    if (self->fullscreen) {
        gtk_window_unfullscreen(GTK_WINDOW(self));
        gtk_widget_show(GTK_WIDGET(self->menubar));
        self->fullscreen = false;
    }
    else {
        gtk_window_fullscreen(GTK_WINDOW(self));
        gtk_widget_hide(GTK_WIDGET(self->menubar));
        self->fullscreen = true;
    }
}

gboolean nesx_window_on_key_release(NESxWindow * self, GdkEventKey * event)
{
    switch (event->keyval)
    {
    case GDK_KEY_Escape:
        if (self->fullscreen) {
            nesx_window_toggle_fullscreen(self);
        }
    }
    return false;
}

void nesx_window_zoom_1(NESxWindow * self)
{
    nesx_window_set_scale(self, 1);
}

void nesx_window_zoom_2(NESxWindow * self)
{
    nesx_window_set_scale(self, 2);   
}

void nesx_window_zoom_3(NESxWindow * self)
{
    nesx_window_set_scale(self, 3);
}

void nesx_window_zoom_4(NESxWindow * self)
{
    nesx_window_set_scale(self, 4);   
}

void nesx_window_class_init(NESxWindowClass * klass)
{
    GtkWidgetClass * widget_class = GTK_WIDGET_CLASS(klass);

    GError * error = NULL;
    GBytes * data = g_resource_lookup_data(nesx_get_resource(), "/main.glade", 0, &error);
    if (!data) {
        fprintf(stderr, "failed to load main.glade: %s\n", error->message);
        g_error_free(error);
    }

    gtk_widget_class_set_template(widget_class, data);

    gtk_widget_class_bind_template_child(widget_class, NESxWindow, menubar);
    gtk_widget_class_bind_template_child(widget_class, NESxWindow, glarea);
    
    gtk_widget_class_bind_template_callback(widget_class, nesx_window_on_key_release);
    gtk_widget_class_bind_template_callback(widget_class, nesx_window_toggle_fullscreen);
    gtk_widget_class_bind_template_callback(widget_class, nesx_window_zoom_1);
    gtk_widget_class_bind_template_callback(widget_class, nesx_window_zoom_2);
    gtk_widget_class_bind_template_callback(widget_class, nesx_window_zoom_3);
    gtk_widget_class_bind_template_callback(widget_class, nesx_window_zoom_4);

}
