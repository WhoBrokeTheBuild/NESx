#ifndef NESX_MEMORY_VIEW_H
#define NESX_MEMORY_VIEW_H

#include <gtk/gtk.h>
#include <stdint.h>

G_DECLARE_FINAL_TYPE(NESxMemoryView, nesx_memory_view, NESX, MEMORY_VIEW, GtkScrolledWindow)

typedef struct _NESxMemoryView         NESxMemoryView;
typedef struct _NESxMemoryViewRegion   NESxMemoryViewRegion;

struct _NESxMemoryViewRegion
{
    uint16_t start;
    uint16_t end;

    double scroll;

    NESxMemoryViewRegion * next;
};

struct _NESxMemoryView
{
    GtkScrolledWindow window;

    GtkTextView * addressView;
    GtkTextView * dataView;
    GtkTextView * textView;

    GtkTextBuffer * addressBuffer;
    GtkTextBuffer * dataBuffer;
    GtkTextBuffer * textBuffer;

    NESxMemoryViewRegion * region;

    double scrollHeight;
};

GtkWidget * nesx_memory_view_new();

void nesx_memory_view_add_region(NESxMemoryView * mem, uint16_t startAddress, uint16_t endAddress, uint8_t * data, size_t size);

void nesx_memory_view_scroll_to_address(NESxMemoryView * mem, uint16_t address);

#endif // NESX_MEMORY_VIEW_H