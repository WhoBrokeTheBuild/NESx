#include "MemoryView.h"

#include <ctype.h>

G_DEFINE_TYPE(NESxMemoryView, nesx_memory_view, GTK_TYPE_SCROLLED_WINDOW)

#define _ADDRESS_LINE_LENGTH    5
#define _DATA_LINE_LENGTH       48
#define _TEXT_LINE_LENGTH       17

#define MARGIN 8

void nesx_memory_view_init(NESxMemoryView * mem)
{
    GtkScrolledWindow * window = &mem->window;
    mem->region = NULL;

    gtk_scrolled_window_set_hadjustment(window, NULL);
    gtk_scrolled_window_set_vadjustment(window, NULL);

    gtk_scrolled_window_set_policy(window, GTK_POLICY_NEVER, GTK_POLICY_ALWAYS);

    // mem->scrollbar = GTK_SCROLLBAR(gtk_scrollbar_new(GTK_ORIENTATION_VERTICAL, mem->adj));
    // g_signal_connect(G_OBJECT(mem->adj), "value_changed", G_CALLBACK(memory_view_scrolled), mem);
    // gtk_container_add(GTK_CONTAINER(mem), GTK_WIDGET(mem->scrollbar));

    GtkBox * box = GTK_BOX(gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0));
    if (!box) {

    }

    mem->addressView = GTK_TEXT_VIEW(gtk_text_view_new());
    gtk_container_add(GTK_CONTAINER(box), GTK_WIDGET(mem->addressView));
    gtk_text_view_set_monospace(mem->addressView, TRUE);
    gtk_text_view_set_editable(mem->addressView, FALSE);
    gtk_text_view_set_left_margin(mem->addressView, MARGIN);
    gtk_widget_set_sensitive(GTK_WIDGET(mem->addressView), FALSE);

    mem->addressBuffer = gtk_text_buffer_new(NULL);
    gtk_text_view_set_buffer(mem->addressView, mem->addressBuffer);

    mem->dataView = GTK_TEXT_VIEW(gtk_text_view_new());
    gtk_container_add(GTK_CONTAINER(box), GTK_WIDGET(mem->dataView));
    gtk_text_view_set_monospace(mem->dataView, TRUE);
    gtk_text_view_set_editable(mem->dataView, FALSE);
    gtk_text_view_set_left_margin(mem->dataView, MARGIN);

    mem->dataBuffer = gtk_text_buffer_new(NULL);
    gtk_text_view_set_buffer(mem->dataView, mem->dataBuffer);
    
    mem->textView = GTK_TEXT_VIEW(gtk_text_view_new());
    gtk_container_add(GTK_CONTAINER(box), GTK_WIDGET(mem->textView));
    gtk_text_view_set_monospace(mem->textView, TRUE);
    gtk_text_view_set_editable(mem->textView, FALSE);
    gtk_text_view_set_left_margin(mem->dataView, MARGIN);
    gtk_text_view_set_right_margin(mem->textView, MARGIN);

    mem->textBuffer = gtk_text_buffer_new(NULL);
    gtk_text_view_set_buffer(mem->textView, mem->textBuffer);

    gtk_container_add(GTK_CONTAINER(&mem->window), GTK_WIDGET(box));
    gtk_widget_show_all(GTK_WIDGET(&mem->window));
}

void nesx_memory_view_class_init(NESxMemoryViewClass * klass)
{
}

GtkWidget * nesx_memory_view_new()
{
    NESxMemoryView * mem;

    mem = NESX_MEMORY_VIEW(g_object_new(nesx_memory_view_get_type(), NULL));
    g_return_val_if_fail(mem != NULL, NULL);

    return GTK_WIDGET(mem);
}

void nesx_memory_view_add_region(NESxMemoryView * mem, uint16_t baseAddress, uint8_t * data, size_t size)
{
    // FIXME: Totally a hack
    const double FONT_HEIGHT = 18.0;

    char addressLine[6];
    char dataLine[49];
    char textLine[18];

    NESxMemoryViewRegion ** next = &mem->region;
    while (*next) {
        next = &(*next)->next;
    }

    gtk_widget_show_all(GTK_WIDGET(mem));

    *next = (NESxMemoryViewRegion*)malloc(sizeof(NESxMemoryViewRegion));
    if (!*next) {
        fprintf(stderr, "out of memory\n");
        return;
    }

    (*next)->start = baseAddress;
    (*next)->end = baseAddress + size;
    (*next)->next = NULL;
    (*next)->scroll = mem->scrollHeight;

    const char hex[] = { 
        '0', '1', '2', '3', '4', '5', '6', '7', 
        '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' 
    };

    GtkTextIter iter;

    unsigned i;
    for (i = 0; i < size; i += 0x10) {
        snprintf(addressLine, sizeof(addressLine), "%04X\n", baseAddress + i);

        unsigned remaining = MIN(0x10, (size - i));
        for (unsigned b = 0; b < remaining; ++b) {
            uint8_t byte = data[i + b];

            dataLine[(b * 3) + 0] = hex[(byte & 0xF0) >> 4];
            dataLine[(b * 3) + 1] = hex[(byte & 0x0F)];
            dataLine[(b * 3) + 2] = ' ';

            textLine[b] = (isalnum(byte) ? (char)byte : '.');
        }

        // Fill in the rest with ' '
        memset(dataLine + (remaining * 3), ' ', ((0x10 - remaining) * 3));
        memset(textLine + remaining, ' ', (0x10 - remaining));

        dataLine[47] = '\n';
        dataLine[48] = '\0';

        textLine[16] = '\n';
        textLine[17] = '\0';

        gtk_text_buffer_get_end_iter(mem->addressBuffer, &iter);
        gtk_text_buffer_insert(mem->addressBuffer, &iter, addressLine, -1);

        gtk_text_buffer_get_end_iter(mem->dataBuffer, &iter);
        gtk_text_buffer_insert(mem->dataBuffer, &iter, dataLine, -1);

        gtk_text_buffer_get_end_iter(mem->textBuffer, &iter);
        gtk_text_buffer_insert(mem->textBuffer, &iter, textLine, -1);

        mem->scrollHeight += FONT_HEIGHT;
    }

    gtk_text_buffer_get_end_iter(mem->addressBuffer, &iter);
    gtk_text_buffer_insert(mem->addressBuffer, &iter, "\n", -1);

    gtk_text_buffer_get_end_iter(mem->dataBuffer, &iter);
    gtk_text_buffer_insert(mem->dataBuffer, &iter, "\n", -1);

    gtk_text_buffer_get_end_iter(mem->textBuffer, &iter);
    gtk_text_buffer_insert(mem->textBuffer, &iter, "\n", -1);

    mem->scrollHeight += FONT_HEIGHT;
}

void nesx_memory_view_scroll_to_address(NESxMemoryView * mem, uint16_t address)
{
    NESxMemoryViewRegion * region = mem->region;
    while (region) {
        if (address >= region->start && address < region->end) {
            GtkAdjustment * vadjust = gtk_scrolled_window_get_vadjustment(&mem->window);
            gtk_adjustment_set_value(vadjust, region->scroll);
            return;
        }
        
        region = region->next;
    }
}