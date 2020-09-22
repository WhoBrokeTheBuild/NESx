#ifndef NESX_BREAKPOINT_DIALOG_H
#define NESX_BREAKPOINT_DIALOG_H

#include <gtk/gtk.h>
#include <stdint.h>

G_DECLARE_FINAL_TYPE(
    NESxBreakpointDialog,
    nesx_breakpoint_dialog,
    NESX, BREAKPOINT_DIALOG,
    GtkDialog
)

typedef struct _NESxBreakpointDialog NESxBreakpointDialog;

struct _NESxBreakpointDialog
{
    GtkDialog dialog;

    GtkNotebook * nbkBreakpoint;

    GtkEntry * entNumericValue;

    GtkComboBoxText * cmbNumericTarget;
    GtkComboBoxText * cmbNumericComp;
    GtkComboBoxText * cmbFlagTarget;
    GtkComboBoxText * cmbFlagTrigger;
};

GtkWidget * nesx_breakpoint_dialog_new(GtkWindow * parent);

int nesx_breakpoint_dialog_get_breakpoint_type(NESxBreakpointDialog * self);

void nesx_breakpoint_dialog_set_breakpoint_type(NESxBreakpointDialog * self, int type);

int nesx_breakpoint_dialog_get_numeric_target(NESxBreakpointDialog * self);

void nesx_breakpoint_dialog_set_numeric_target(NESxBreakpointDialog * self, int numericTarget);

int nesx_breakpoint_dialog_get_numeric_comp(NESxBreakpointDialog * self);

void nesx_breakpoint_dialog_set_numeric_comp(NESxBreakpointDialog * self, int numericComp);

const char * nesx_breakpoint_dialog_get_numeric_value(NESxBreakpointDialog * self);

int nesx_breakpoint_dialog_get_flag_target(NESxBreakpointDialog * self);

void nesx_breakpoint_dialog_set_flag_target(NESxBreakpointDialog * self, int flagTarget);

int nesx_breakpoint_dialog_get_flag_trigger(NESxBreakpointDialog * self);

void nesx_breakpoint_dialog_set_flag_trigger(NESxBreakpointDialog * self, int flagState);

#endif // NESX_BREAKPOINT_DIALOG_H