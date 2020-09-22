#include "BreakpointDialog.h"

#include "Resource.h"

G_DEFINE_TYPE(NESxBreakpointDialog, nesx_breakpoint_dialog, GTK_TYPE_DIALOG);

void nesx_breakpoint_dialog_init(NESxBreakpointDialog * self)
{
    gtk_widget_init_template(GTK_WIDGET(self));

    gtk_dialog_set_default_response(GTK_DIALOG(self), GTK_RESPONSE_APPLY);
}

void nesx_breakpoint_dialog_class_init(NESxBreakpointDialogClass * klass)
{
    GtkWidgetClass * wc = GTK_WIDGET_CLASS(klass);

    GError * error = NULL;
    GBytes * data = g_resource_lookup_data(nesx_get_resource(), "/nesx/BreakpointDialog.glade", 0, &error);
    if (!data) {
        fprintf(stderr, "failed to load /nesx/BreakpointDialog.glade %s\n", error->message);
        g_error_free(error);
    }

    gtk_widget_class_set_template(wc, data);

    gtk_widget_class_bind_template_child(wc, NESxBreakpointDialog, nbkBreakpoint);
    gtk_widget_class_bind_template_child(wc, NESxBreakpointDialog, entNumericValue);
    gtk_widget_class_bind_template_child(wc, NESxBreakpointDialog, cmbNumericTarget);
    gtk_widget_class_bind_template_child(wc, NESxBreakpointDialog, cmbNumericComp);
    gtk_widget_class_bind_template_child(wc, NESxBreakpointDialog, cmbFlagTarget);
    gtk_widget_class_bind_template_child(wc, NESxBreakpointDialog, cmbFlagTrigger);
}

GtkWidget * nesx_breakpoint_dialog_new(GtkWindow * parent)
{
    NESxBreakpointDialog * self;
    self = NESX_BREAKPOINT_DIALOG(g_object_new(nesx_breakpoint_dialog_get_type(), NULL));
    gtk_window_set_transient_for(GTK_WINDOW(self), parent);
    return GTK_WIDGET(self);
}

int nesx_breakpoint_dialog_get_breakpoint_type(NESxBreakpointDialog * self)
{
    return gtk_notebook_get_current_page(self->nbkBreakpoint);
}

void nesx_breakpoint_dialog_set_breakpoint_type(NESxBreakpointDialog * self, int type)
{
    gtk_notebook_set_current_page(self->nbkBreakpoint, type);
}

int nesx_breakpoint_dialog_get_numeric_target(NESxBreakpointDialog * self)
{
    return gtk_combo_box_get_active(GTK_COMBO_BOX(self->cmbNumericTarget));
}

void nesx_breakpoint_dialog_set_numeric_target(NESxBreakpointDialog * self, int numericTarget)
{
    gtk_combo_box_set_active(GTK_COMBO_BOX(self->cmbNumericTarget), numericTarget);
}

int nesx_breakpoint_dialog_get_numeric_comp(NESxBreakpointDialog * self)
{
    return gtk_combo_box_get_active(GTK_COMBO_BOX(self->cmbNumericComp));
}

void nesx_breakpoint_dialog_set_numeric_comp(NESxBreakpointDialog * self, int numericComp)
{
    gtk_combo_box_set_active(GTK_COMBO_BOX(self->cmbNumericComp), numericComp);
}

const char * nesx_breakpoint_dialog_get_numeric_value(NESxBreakpointDialog * self)
{
    return gtk_entry_get_text(self->entNumericValue);
}

int nesx_breakpoint_dialog_get_flag_target(NESxBreakpointDialog * self)
{
    return gtk_combo_box_get_active(GTK_COMBO_BOX(self->cmbFlagTarget));
}

void nesx_breakpoint_dialog_set_flag_target(NESxBreakpointDialog * self, int flagTarget)
{
    gtk_combo_box_set_active(GTK_COMBO_BOX(self->cmbFlagTarget), flagTarget);
}

int nesx_breakpoint_dialog_get_flag_trigger(NESxBreakpointDialog * self)
{
    return gtk_combo_box_get_active(GTK_COMBO_BOX(self->cmbFlagTrigger));
}

void nesx_breakpoint_dialog_set_flag_trigger(NESxBreakpointDialog * self, int flagTrigger)
{
    gtk_combo_box_set_active(GTK_COMBO_BOX(self->cmbFlagTrigger), flagTrigger);
}