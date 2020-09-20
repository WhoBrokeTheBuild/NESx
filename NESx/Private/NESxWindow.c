#include "NESxWindow.h"

#include "Resource.h"
#include <epoxy/gl.h>
#include <NESx/Version.h>

#define ATTRIB_POSITION  0
#define ATTRIB_UV        1

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
"uniform sampler2D Texture;                             \n"
"out vec4 color;                                        \n"
"void main() {                                          \n"
"   color = texture2D(Texture, UV);                     \n"
"}                                                      \n";

const float VERTS[] = {
    -1.0f,  1.0f, 0.0f,
     1.0f,  1.0f, 0.0f,
     1.0f, -1.0f, 0.0f,
    
     1.0f, -1.0f, 0.0f,
    -1.0f, -1.0f, 0.0f,
    -1.0f,  1.0f, 0.0f,     
};

const float UVS[] = {
    0.0f, 0.0f,
    1.0f, 0.0f,
    1.0f, 1.0f,

    1.0f, 1.0f,
    0.0f, 1.0f,
    0.0f, 0.0f,
};

G_DEFINE_TYPE(NESxWindow, nesx_window, GTK_TYPE_APPLICATION_WINDOW)

GtkWidget * nesx_window_new(nesx_t * nes)
{
    NESxWindow * window;
    window = NESX_WINDOW(g_object_new(nesx_window_get_type(), NULL));
    window->nes = nes;
    return GTK_WIDGET(window);
}

void nesx_window_init(NESxWindow * self)
{
    gtk_widget_init_template(GTK_WIDGET(self));

    self->open = true;
    self->running = false;
    self->fullscreen = false;
    self->debugger = NULL;
}

void nesx_window_term(NESxWindow * self)
{
    self->open = false;
    self->running = false;
    if (self->debugger) {
        gtk_widget_destroy(GTK_WIDGET(self->debugger));
    }
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

void nesx_window_open_rom(NESxWindow * self)
{
    GtkWidget * dialog = gtk_file_chooser_dialog_new(
        "Open ROM", 
        GTK_WINDOW(self),
        GTK_FILE_CHOOSER_ACTION_OPEN,
        "Cancel", GTK_RESPONSE_CANCEL,
        "Open", GTK_RESPONSE_ACCEPT,
        NULL);

    GtkFileFilter * filterROMs = gtk_file_filter_new();
    gtk_file_filter_set_name(filterROMs, "NES ROMs (*.nes)");
    gtk_file_filter_add_pattern(filterROMs, "*.nes");
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filterROMs);

    GtkFileFilter * filterAll = gtk_file_filter_new();
    gtk_file_filter_set_name(filterAll, "All Files");
    gtk_file_filter_add_pattern(filterAll, "*");
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filterAll);
    
    
    gint result = gtk_dialog_run(GTK_DIALOG(dialog));
    if (result == GTK_RESPONSE_ACCEPT) {
        char * filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
        if (NESx_ROM_Load(self->nes, filename)) {
            if (!self->debugger) {
                self->running = true;
            }
            else {
                nesx_debugger_update(self->debugger);
            }
        }
        g_free(filename);
    }

    gtk_widget_destroy(dialog);
}

void nesx_window_show_debugger(NESxWindow * self)
{
    if (!self->debugger) {
        self->debugger = NESX_DEBUGGER(nesx_debugger_new(self->nes, &self->running));
    }

    gtk_widget_show_all(GTK_WIDGET(self->debugger));
    nesx_debugger_update(self->debugger);
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

    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

    glGenTextures(1, &self->texture);
    glBindTexture(GL_TEXTURE_2D, self->texture);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    self->pixels = (uint8_t *)malloc(NESX_WIDTH * NESX_HEIGHT * 4);
    memset(self->pixels, 0xFF, NESX_WIDTH * NESX_HEIGHT * 4);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, NESX_WIDTH, NESX_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, self->pixels);

    unsigned vert = glCreateShader(GL_VERTEX_SHADER);
    unsigned frag = glCreateShader(GL_FRAGMENT_SHADER);

    glShaderSource(vert, 1, &VERTEX_SHADER, NULL);
    glShaderSource(frag, 1, &FRAGMENT_SHADER, NULL);
    
    glCompileShader(vert);
    glCompileShader(frag);

    self->shader = glCreateProgram();

    glAttachShader(self->shader, vert);
    glAttachShader(self->shader, frag);

    glLinkProgram(self->shader);

    glDeleteShader(vert);
    glDeleteShader(frag);

    glUseProgram(self->shader);

    glGenVertexArrays(1, &self->vao);
    glBindVertexArray(self->vao);

    unsigned vbo[2];
    glGenBuffers(2, vbo);

    glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(VERTS), VERTS, GL_STATIC_DRAW);

    glVertexAttribPointer(ATTRIB_POSITION, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, NULL);
    glEnableVertexAttribArray(ATTRIB_POSITION);

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

uint8_t system_palette[] = {
     84,  84,  84,
      0,  30, 116,
      8,  16, 144,
     48,   0, 136,
     68,   0, 100,
     92,   0,  48,
     84,   4,   0,
     60,  24,   0,
     32,  42,   0,
      8,  58,   0,
      0,  64,   0,
      0,  60,   0,
      0,  50,  60,
      0,   0,   0,
    152, 150, 152,
      8,  76, 196,
     48,  50, 236,
     92,  30, 228,
    136,  20, 176,
    160,  20, 100,
    152,  34,  32,
    120,  60,   0,
     84,  90,   0,
     40, 114,   0,
      8, 124,   0,
      0, 118,  40,
      0, 102, 120,
      0,   0,   0,
    236, 238, 236,
     76, 154, 236,
    120, 124, 236,
    176,  98, 236,
    228,  84, 236,
    236,  88, 180,
    236, 106, 100,
    212, 136,  32,
    160, 170,   0,
    116, 196,   0,
     76, 208,  32,
     56, 204, 108,
     56, 180, 204,
     60,  60,  60,
    236, 238, 236,
    168, 204, 236,
    188, 188, 236,
    212, 178, 236,
    236, 174, 236,
    236, 174, 212,
    236, 180, 176,
    228, 196, 144,
    204, 210, 120,
    180, 222, 120,
    168, 226, 144,
    152, 226, 180,
    160, 214, 228,
    160, 162, 160,
};

void nesx_window_gl_render(NESxWindow * self)
{
    glClear(GL_COLOR_BUFFER_BIT);

    nesx_ppu_t * ppu = &self->nes->PPU;

    // for (int y = 0; y < NESX_HEIGHT; ++y) {
    //     for (int x = 0; x < NESX_WIDTH; ++x) {
    //         int pixel     = ((y * NESX_WIDTH) + x) * 4;
    //         // int tile      = (y * NESX_WIDTH / 8) + (x / 8);
    //         // int attribute = (y * NESX_WIDTH / 32) + (x / 32);
    //         int pattern = ((y / 8) * NESX_WIDTH * 16) + ((x / 8) * 16) + y;
    //         int patternIndex = 7 - (x % 8);

    //         if (pattern >= 0x2000) {
    //             break;
    //         }
    //         // pattern %= 0x2000;

    //         // int quadrantShift = (((y / 16) % 2) * 4) + ((x / 16) % 2);

    //         // uint8_t tileData = ppu->NameTables[0][tile];
    //         // printf("%02x ", tileData);
    //         uint8_t patternData1 = (ppu->PatternTables[pattern] >> patternIndex) & 1;
    //         uint8_t patternData2 = (ppu->PatternTables[pattern + 8] >> patternIndex) & 1;
    //         uint8_t patternData = patternData1 | (patternData2 << 1);

    //         // printf("%d:%d:%d\n", pattern, pattern + 8, patternIndex);

    //         // uint8_t attribData = ppu->NameTables[0][0x3C0 + attribute];
    //         // int palette = (attribData >> quadrantShift) & 0b11;

    //         // ppu->PaletteRAM[palette * 4];

    //         uint8_t colors[] = {
    //             0x00,
    //             0x55,
    //             0xAA,
    //             0xFF,
    //         };

    //         self->pixels[pixel + 0] = colors[patternData];
    //         self->pixels[pixel + 1] = colors[patternData];
    //         self->pixels[pixel + 2] = colors[patternData];
    //         self->pixels[pixel + 3] = 0xFF;
    //     }
    //     // printf("\n");
    //     // exit(1);
    // }

    // for (int i = 0; i < NESX_WIDTH * NESX_HEIGHT * 4; i += 4) {
    //     self->pixels[i + 0] = rand() % 0xFF;
    //     self->pixels[i + 1] = rand() % 0xFF;
    //     self->pixels[i + 2] = rand() % 0xFF;
    //     self->pixels[i + 3] = 0xFF;
    // }

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, NESX_WIDTH, NESX_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, self->pixels);

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
    GtkWidgetClass * wc = GTK_WIDGET_CLASS(klass);

    GError * error = NULL;
    GBytes * data = g_resource_lookup_data(nesx_get_resource(), "/NESxWindow.glade", 0, &error);
    if (!data) {
        fprintf(stderr, "failed to load NESxWindow.glade: %s\n", error->message);
        g_error_free(error);
    }

    gtk_widget_class_set_template(wc, data);

    // gtk_widget_class_bind_template_child(wc, NESxWindow, menubar);
    gtk_widget_class_bind_template_child(wc, NESxWindow, glarea);

    gtk_widget_class_bind_template_callback(wc, nesx_window_term);
    gtk_widget_class_bind_template_callback(wc, nesx_window_open_rom);
    gtk_widget_class_bind_template_callback(wc, nesx_window_show_debugger);
    gtk_widget_class_bind_template_callback(wc, nesx_window_show_about);
    gtk_widget_class_bind_template_callback(wc, nesx_window_on_key_release);
    gtk_widget_class_bind_template_callback(wc, nesx_window_toggle_fullscreen);
    gtk_widget_class_bind_template_callback(wc, nesx_window_gl_init);
    gtk_widget_class_bind_template_callback(wc, nesx_window_gl_term);
    gtk_widget_class_bind_template_callback(wc, nesx_window_gl_configure);
    gtk_widget_class_bind_template_callback(wc, nesx_window_gl_render);
    gtk_widget_class_bind_template_callback(wc, nesx_window_zoom_1);
    gtk_widget_class_bind_template_callback(wc, nesx_window_zoom_2);
    gtk_widget_class_bind_template_callback(wc, nesx_window_zoom_3);
    gtk_widget_class_bind_template_callback(wc, nesx_window_zoom_4);

}

void nesx_window_run(NESxWindow * self)
{
    while (self->open) {
        while (g_main_context_iteration(NULL, false)) { }

        if (self->running) {
            NESx_Frame(self->nes);
        }

        gtk_gl_area_queue_render(self->glarea);
    }
}