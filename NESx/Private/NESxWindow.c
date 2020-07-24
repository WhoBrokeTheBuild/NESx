#include "NESxWindow.h"

#include "Resource.h"
#include <epoxy/gl.h>
#include <NESx/Version.h>

const unsigned ATTRIB_POSITION  = 0;
const unsigned ATTRIB_UV        = 1;

const char * VERTEX_SHADER = 
"#version 330 core                                      \n"
"in vec3 inPosition;                                    \n"
"in vec2 inUV;                                          \n"
"out vec2 UV;                                           \n"
"void main() {                                          \n"
"   gl_Position = vec4(inPosition, 1.0);                \n"
"   UV = inUV;                                          \n"
"}                                                      \n";

const char * FRAGMENT_SHADER =
"#version 330 core                                      \n"
"in vec2 UV;                                            \n"
"out vec4 color;                                        \n"
"void main() {                                          \n"
"   color = vec4(UV, 0.0, 1.0);                         \n"
"}                                                      \n";

const float VERTS[] = {
    -1.0f, -1.0f, 0.0f,
     1.0f, -1.0f, 0.0f,
     1.0f,  1.0f, 0.0f,
    
     1.0f,  1.0f, 0.0f,
    -1.0f,  1.0f, 0.0f,
    -1.0f, -1.0f, 0.0f,     
};

const float UVS[] = {
    0.0f, 0.0f,
    0.0f, 1.0f,
    1.0f, 1.0f,

    1.0f, 1.0f,
    1.0f, 0.0f,
    0.0f, 0.0f,
};

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
    1.0f, 0.0f, 0.0f,
        gtk_widget_show(GTK_WIDGET(self->menubar));
        self->fullscreen = false;
    }
    else {
        gtk_window_fullscreen(GTK_WINDOW(self));
        gtk_widget_hide(GTK_WIDGET(self->menubar));
        self->fullscreen = true;
    }
}

void nesx_window_show_about(NESxWindow * self)
{
    static const char * authors[] = {
        "Stephen Lane-Walsh",
        "Dillon Beliveau",
        NULL,
    };

    static const char * website = "github.com/WhoBrokeTheBuild/NESx";

    gtk_show_about_dialog(GTK_WINDOW(self),
        "name", "NESx",
        "version", NESX_VERSION_STRING,
        "authors", authors,
        "website", website,
        "website-label", website, 
        "license-type", GTK_LICENSE_MIT_X11,
        "logo-icon-name", "application-x-executable",
        NULL);
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

void nesx_window_gl_init(NESxWindow * self)
{
    gtk_gl_area_make_current(self->glarea);
    if (gtk_gl_area_get_error(self->glarea) != NULL) {
        fprintf(stderr, "failed to create GtkGLArea\n");
    }

    printf("OpenGL Version: %s\n", glGetString(GL_VERSION));
    printf("OpenGL Renderer: %s\n", glGetString(GL_RENDERER));


    char infoLog[512];



    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    unsigned vert = glCreateShader(GL_VERTEX_SHADER);
    unsigned frag = glCreateShader(GL_FRAGMENT_SHADER);

    glShaderSource(vert, 1, &VERTEX_SHADER, NULL);
    glShaderSource(frag, 1, &FRAGMENT_SHADER, NULL);
    
    glCompileShader(vert);
    glCompileShader(frag);

    glGetShaderInfoLog(vert, sizeof(infoLog), NULL, infoLog);
    printf("%s\n", infoLog);

    glGetShaderInfoLog(frag, sizeof(infoLog), NULL, infoLog);
    printf("%s\n", infoLog);

    self->shader = glCreateProgram();

    glAttachShader(self->shader, vert);
    glAttachShader(self->shader, frag);

    glLinkProgram(self->shader);

    glGetProgramInfoLog(self->shader, sizeof(infoLog), NULL, infoLog);
    printf("%s\n", infoLog);

    glDeleteShader(vert);
    glDeleteShader(frag);

    glUseProgram(self->shader);

    glGenVertexArrays(1, &self->vao);
    glBindVertexArray(self->vao);

    unsigned vbo[2];
    glGenBuffers(2, vbo);

    // Vertices

    glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(VERTS), VERTS, GL_STATIC_DRAW);

    glVertexAttribPointer(ATTRIB_POSITION, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, NULL);
    glEnableVertexAttribArray(ATTRIB_POSITION);

    // UVs

    glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(UVS), UVS, GL_STATIC_DRAW);

    glVertexAttribPointer(ATTRIB_UV, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 2, NULL);
    glEnableVertexAttribArray(ATTRIB_UV);
}

void nesx_window_gl_term(NESxWindow * self)
{
    // glDeleteProgram(self->shader);
    // glDeleteVertexArrays(1, &self->vao);
}

void nesx_window_gl_configure(NESxWindow * self, GdkEventConfigure * event)
{
    glViewport(0, 0, event->width, event->height);
}

void nesx_window_gl_render(NESxWindow * self)
{
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(self->shader);
    glBindVertexArray(self->vao);
    glDrawArrays(GL_TRIANGLES, 0, sizeof(VERTS) / sizeof(VERTS[0]));
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

    // gtk_widget_class_bind_template_child(widget_class, NESxWindow, menubar);
    gtk_widget_class_bind_template_child(widget_class, NESxWindow, glarea);

    gtk_widget_class_bind_template_callback(widget_class, nesx_window_show_about);
    gtk_widget_class_bind_template_callback(widget_class, nesx_window_on_key_release);
    gtk_widget_class_bind_template_callback(widget_class, nesx_window_toggle_fullscreen);
    gtk_widget_class_bind_template_callback(widget_class, nesx_window_gl_init);
    gtk_widget_class_bind_template_callback(widget_class, nesx_window_gl_term);
    gtk_widget_class_bind_template_callback(widget_class, nesx_window_gl_configure);
    gtk_widget_class_bind_template_callback(widget_class, nesx_window_gl_render);
    gtk_widget_class_bind_template_callback(widget_class, nesx_window_zoom_1);
    gtk_widget_class_bind_template_callback(widget_class, nesx_window_zoom_2);
    gtk_widget_class_bind_template_callback(widget_class, nesx_window_zoom_3);
    gtk_widget_class_bind_template_callback(widget_class, nesx_window_zoom_4);

}
