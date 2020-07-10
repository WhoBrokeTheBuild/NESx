#include "MemoryView.h"

#include <ctype.h>

#define _ADDRESS_LINE_LENGTH    5
#define _DATA_LINE_LENGTH       48
#define _TEXT_LINE_LENGTH       17

#define MARGIN 8

void memory_view_scrolled(GtkAdjustment * adj, MemoryView * mem)
{
    
}

// void memory_view_move_cursor(GtkTextView * txt, 
//     GtkTextExtendSelection granularity, GtkTextIter * location, 
//     GtkTextIter *s, GtkTextIter * e, MemoryView * mem)
// {
//     int start;
//     int end;

//     gtk_editable_get_selection_bounds(GTK_EDITABLE(txt), &start, &end);

//     if (txt == mem->dataView) {
//         start /= _DATA_LINE_LENGTH;
//         start *= _TEXT_LINE_LENGTH;
//         end /= _DATA_LINE_LENGTH;
//         end *= _TEXT_LINE_LENGTH;

//         gtk_editable_select_region(GTK_EDITABLE(mem->textView), start, end);
//     } 
//     else if (txt == mem->textView) {
//         start /= _TEXT_LINE_LENGTH;
//         start *= _DATA_LINE_LENGTH;
//         end /= _TEXT_LINE_LENGTH;
//         end *= _DATA_LINE_LENGTH;

//         gtk_editable_select_region(GTK_EDITABLE(mem->dataView), start, end);
//     }
// }

void memory_view_init(MemoryView * mem, gpointer klass)
{
    GtkScrolledWindow * window = &mem->window;

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

static void memory_view_class_init(MemoryViewClass * klass, gpointer data)
{

}

GType memory_view_get_type()
{
    static GType type = 0;

    if (!type) {
        GTypeInfo info = {
            sizeof(MemoryViewClass),
            NULL,   // base_init
            NULL,   // base_finalize
            (GClassInitFunc)memory_view_class_init,
            NULL,    // class_finalize
            NULL,   // class_data
            sizeof(MemoryView),
            0,      // n_preallocs
            (GInstanceInitFunc)memory_view_init
        };

        type = g_type_register_static(gtk_scrolled_window_get_type(), 
            "MemoryView", &info, 0);
    }

    return type;
}

GtkWidget * memory_view_new()
{
    MemoryView * mem;

    mem = MEMORY_VIEW(g_object_new(MEMORY_VIEW_TYPE, NULL));
    g_return_val_if_fail(mem != NULL, NULL);

    return GTK_WIDGET(mem);
}

void memory_view_add_region(MemoryView * mem, uint16_t baseAddress, uint8_t * data, size_t size)
{
    char addressLine[6];
    char dataLine[49];
    char textLine[18];

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
    }

    gtk_text_buffer_get_end_iter(mem->addressBuffer, &iter);
    gtk_text_buffer_insert(mem->addressBuffer, &iter, "\n", -1);

    gtk_text_buffer_get_end_iter(mem->dataBuffer, &iter);
    gtk_text_buffer_insert(mem->dataBuffer, &iter, "\n", -1);

    gtk_text_buffer_get_end_iter(mem->textBuffer, &iter);
    gtk_text_buffer_insert(mem->textBuffer, &iter, "\n", -1);
}
