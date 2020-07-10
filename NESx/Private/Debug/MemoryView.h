#ifndef DEBUG_MEMORY_VIEW_H
#define DEBUG_MEMORY_VIEW_H

#include <gtk/gtk.h>
#include <stdint.h>

G_BEGIN_DECLS

#define MEMORY_VIEW_TYPE                (memory_view_get_type())
#define MEMORY_VIEW(obj)                (G_TYPE_CHECK_INSTANCE_CAST((obj), MEMORY_VIEW_TYPE, MemoryView))
#define MEMORY_VIEW_CLASS(klass)        (G_TYPE_CHECK_CLASS_CAST((klass), MEMORY_VIEW_TYPE, MemoryViewClass))
#define IS_MEMORY_VIEW(obj)             (G_TYPE_CHECK_INSTANCE_TYPE((obj), MEMORY_VIEW_TYPE))
#define IS_MEMORY_VIEW_CLASS(klass)     (G_TYPE_CHECK_CLASS_TYPE((klass), MEMORY_VIEW_TYPE))
#define MEMORY_VIEW_GET_CLASS(obj)      (G_TYPE_INSTANCE_GET_CLASS((obj), MEMORY_VIEW_TYPE, MemoryViewClass))

typedef struct _MemoryView         MemoryView;
typedef struct _MemoryViewClass    MemoryViewClass;

struct _MemoryView
{
    GtkScrolledWindow window;

    GtkTextView * addressView;
    GtkTextView * dataView;
    GtkTextView * textView;

    GtkTextBuffer * addressBuffer;
    GtkTextBuffer * dataBuffer;
    GtkTextBuffer * textBuffer;
};

struct _MemoryViewClass
{
    GtkScrolledWindowClass parentClass;

};

GType memory_view_get_type();

GtkWidget * memory_view_new();

void memory_view_add_region(MemoryView * mem, uint16_t baseAddress, uint8_t * data, size_t size);

G_END_DECLS

#endif // DEBUG_MEMORY_VIEW_H