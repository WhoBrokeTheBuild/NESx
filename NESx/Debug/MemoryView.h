#ifndef NESX_MEMORY_VIEW_H
#define NESX_MEMORY_VIEW_H

#include <gtk/gtk.h>
#include <stdint.h>

G_DECLARE_FINAL_TYPE(
    DebugMemoryView, 
    debug_memory_view, 
    DEBUG, MEMORY_VIEW, 
    GtkScrolledWindow)

typedef struct _DebugMemoryView DebugMemoryView;
typedef struct _DebugMemoryViewRegion DebugMemoryViewRegion;

struct _DebugMemoryViewRegion
{
    uint16_t start;
    uint16_t end;

    double scroll;

    DebugMemoryViewRegion * next;
};

struct _DebugMemoryView
{
    GtkScrolledWindow window;

    GtkTextView * addressView;
    GtkTextView * dataView;
    GtkTextView * textView;

    GtkTextBuffer * addressBuffer;
    GtkTextBuffer * dataBuffer;
    GtkTextBuffer * textBuffer;

    DebugMemoryViewRegion * region;

    double scrollHeight;
};

GtkWidget * debug_memory_view_new();

void debug_memory_view_add_region(DebugMemoryView * mem, uint16_t startAddress,
    uint16_t endAddress, uint8_t * data, size_t size);

void debug_memory_view_scroll_to_address(DebugMemoryView * mem, uint16_t address);

#endif // NESX_MEMORY_VIEW_H