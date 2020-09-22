#include "NESxDebugger.h"

// #include "Mapper/NROM.h"
#include "Resource.h"
#include "Debug/BreakpointDialog.h"

#include <time.h>
#include <errno.h>
#include <string.h>

#define MARGIN 8
#define NESX_EXECUTION_LOG_HEADER "INSTRUCTION      A  X  Y  S  PC   P       CYCLE          "
#define NESX_EXECUTION_LOG_FORMAT "%-16s %02X %02X %02X %02X %04X %c%c%c%c%c%c%c %lu\n"

// mem_region_select_t CPU_MEMORY_SECTIONS[] = {
//     { 0x0000, "0000: Internal RAM" },
//     { 0x2000, "2000: PPU Registers" },
//     { 0x4000, "4000: APU and I/O Registers" },
//     { 0x4020, "4020: ROM" },
//     { 0, NULL }
//     // TODO: Generate list based on mapper
// };

// mem_region_select_t PPU_MEMORY_SECTIONS[] = { 
//     { 0x1000, "1000: Pattern Tables" },
//     { 0x2000, "2000: Name Tables" },
//     { 0x3F00, "3F00: Palette RAM" },
//     { 0, NULL }
//     //
// };

// void _DebugMemoryRegionChanged(GtkComboBox * cmb, NESxDebugger * self)
// {
//     int index = gtk_combo_box_get_active(cmb);
//     int page = gtk_notebook_get_current_page(self->nbkMemoryView);
//     DebugMemoryView * mem
//         = DEBUG_MEMORY_VIEW(gtk_notebook_get_nth_page(self->nbkMemoryView, page));
//     assert(mem);

//     debug_memory_view_scroll_to_address(mem, self->memoryRegions[index].address);
// }

// void _DebugSetMemoryRegions(NESxDebugger * self, mem_region_select_t * regions)
// {
//     self->memoryRegions = regions;

//     gtk_combo_box_text_remove_all(self->cmbMemoryRegion);

//     int i = 0;
//     while (regions[i].name) {
//         gtk_combo_box_text_append_text(self->cmbMemoryRegion, regions[i].name);
//         ++i;
//     }

//     gtk_combo_box_set_active(GTK_COMBO_BOX(self->cmbMemoryRegion), 0);
// }

G_DEFINE_TYPE(NESxDebugger, nesx_debugger, GTK_TYPE_WINDOW)

GtkWidget * nesx_debugger_new(nesx_t * nes, bool * running)
{
    NESxDebugger * debugger;
    debugger = NESX_DEBUGGER(g_object_new(nesx_debugger_get_type(), NULL));
    debugger->nes = nes;
    debugger->cpu = &nes->CPU;
    debugger->ppu = &nes->PPU;
    debugger->mmu = &nes->MMU;
    debugger->hdr = &nes->ROM.Header;
    debugger->running = running;
    debugger->firstBreakpoint = NULL;
    return GTK_WIDGET(debugger);
}

void nesx_debugger_init(NESxDebugger * self)
{
    gtk_widget_init_template(GTK_WIDGET(self));

    gtk_text_buffer_set_text(gtk_text_view_get_buffer(self->txtExecutionLogHeader), 
        NESX_EXECUTION_LOG_HEADER, -1);

    nesx_debugger_display_breakpoints(self);
}

void nesx_debugger_class_init(NESxDebuggerClass * klass)
{
    GtkWidgetClass * wc = GTK_WIDGET_CLASS(klass);

    GError * error = NULL;
    GBytes * data = g_resource_lookup_data(nesx_get_resource(), "/nesx/Debugger.glade", 0, &error);
    if (!data) {
        fprintf(stderr, "failed to load /nesx/Debugger.glade %s\n", error->message);
        g_error_free(error);
    }

    gtk_widget_class_set_template(wc, data);

    gtk_widget_class_bind_template_callback(wc, nesx_debugger_apply);
    gtk_widget_class_bind_template_callback(wc, nesx_debugger_add_breakpoint);
    gtk_widget_class_bind_template_callback(wc, nesx_debugger_clear_breakpoints);
    gtk_widget_class_bind_template_callback(wc, nesx_debugger_save_execution_log);

    // Status
    gtk_widget_class_bind_template_child(wc, NESxDebugger, lblCPUCycle);
    gtk_widget_class_bind_template_child(wc, NESxDebugger, lblPPUCycle);
    gtk_widget_class_bind_template_child(wc, NESxDebugger, lblPPUScanline);

    // Controls
    gtk_widget_class_bind_template_callback(wc, nesx_debugger_run);
    gtk_widget_class_bind_template_callback(wc, nesx_debugger_stop);
    gtk_widget_class_bind_template_callback(wc, nesx_debugger_tick);
    gtk_widget_class_bind_template_callback(wc, nesx_debugger_step);
    gtk_widget_class_bind_template_callback(wc, nesx_debugger_frame);

    // Status Register
    gtk_widget_class_bind_template_child(wc, NESxDebugger, chkFN);
    gtk_widget_class_bind_template_child(wc, NESxDebugger, chkFV);
    gtk_widget_class_bind_template_child(wc, NESxDebugger, chkFU);
    gtk_widget_class_bind_template_child(wc, NESxDebugger, chkFB);
    gtk_widget_class_bind_template_child(wc, NESxDebugger, chkFD);
    gtk_widget_class_bind_template_child(wc, NESxDebugger, chkFI);
    gtk_widget_class_bind_template_child(wc, NESxDebugger, chkFZ);
    gtk_widget_class_bind_template_child(wc, NESxDebugger, chkFC);

    // Registers
    gtk_widget_class_bind_template_child(wc, NESxDebugger, entA);
    gtk_widget_class_bind_template_child(wc, NESxDebugger, entX);
    gtk_widget_class_bind_template_child(wc, NESxDebugger, entY);
    gtk_widget_class_bind_template_child(wc, NESxDebugger, entS);
    // gtk_widget_class_bind_template_child(wc, NESxDebugger, entSR);
    gtk_widget_class_bind_template_child(wc, NESxDebugger, entPC);

    // CPU Internals
    gtk_widget_class_bind_template_child(wc, NESxDebugger, entAB);
    gtk_widget_class_bind_template_child(wc, NESxDebugger, entAD);
    gtk_widget_class_bind_template_child(wc, NESxDebugger, entIR);
    gtk_widget_class_bind_template_child(wc, NESxDebugger, entDB);
    gtk_widget_class_bind_template_child(wc, NESxDebugger, chkRW);
    gtk_widget_class_bind_template_child(wc, NESxDebugger, chkSYNC);
    gtk_widget_class_bind_template_child(wc, NESxDebugger, chkIRQ);
    gtk_widget_class_bind_template_child(wc, NESxDebugger, chkNMI);
    gtk_widget_class_bind_template_child(wc, NESxDebugger, chkRDY);
    gtk_widget_class_bind_template_child(wc, NESxDebugger, chkRES);

    // PPU Internals
    gtk_widget_class_bind_template_child(wc, NESxDebugger, entScanline);
    gtk_widget_class_bind_template_child(wc, NESxDebugger, chkVBlank);

    // Execution Log
    gtk_widget_class_bind_template_child(wc, NESxDebugger, txtExecutionLogHeader);
    gtk_widget_class_bind_template_child(wc, NESxDebugger, txtExecutionLog);

    // Breakpoints
    gtk_widget_class_bind_template_child(wc, NESxDebugger, lstBreakpoints);
}

void nesx_debugger_tick(NESxDebugger * self)
{
    NESx_Tick(self->nes);
    nesx_debugger_display(self);
}

void nesx_debugger_step(NESxDebugger * self)
{
    NESx_Step(self->nes);
    nesx_debugger_display(self);
}

void nesx_debugger_frame(NESxDebugger * self)
{
    int scanline;
    do {
        scanline = self->nes->PPU.Scanline;
        NESx_Tick(self->nes);
        if (nesx_breakpoint_list_check(self->firstBreakpoint, self->nes)) {
            *(self->running) = false;
            break;
        }
    }
    while (scanline <= self->nes->PPU.Scanline);

    nesx_debugger_display(self);
}

void nesx_debugger_run(NESxDebugger * self)
{
    *(self->running) = true;
}

void nesx_debugger_stop(NESxDebugger * self)
{
    *(self->running) = false;
    nesx_debugger_display(self);
}

void nesx_debugger_save_execution_log(NESxDebugger * self)
{
    GtkWidget * dialog = gtk_file_chooser_dialog_new(
        "Save Execution Log", 
        GTK_WINDOW(self),
        GTK_FILE_CHOOSER_ACTION_SAVE,
        "Cancel", GTK_RESPONSE_CANCEL,
        "Save", GTK_RESPONSE_ACCEPT,
        NULL);

    time_t t = time(NULL);
    struct tm tm = *localtime(&t);

    char buffer[256];
    snprintf(buffer, sizeof(buffer), "%d-%d-%d %02d:%02d:%02d.log",
        tm.tm_year + 1900, tm.tm_mon, tm.tm_mday, 
        tm.tm_hour, tm.tm_min, tm.tm_sec);

    gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(dialog), buffer);

    GtkFileFilter * filterLogs = gtk_file_filter_new();
    gtk_file_filter_set_name(filterLogs, "Log Files (*.log)");
    gtk_file_filter_add_pattern(filterLogs, "*.log");
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filterLogs);

    GtkFileFilter * filterAll = gtk_file_filter_new();
    gtk_file_filter_set_name(filterAll, "All Files");
    gtk_file_filter_add_pattern(filterAll, "*");
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filterAll);
    
    gint result = gtk_dialog_run(GTK_DIALOG(dialog));
    if (result == GTK_RESPONSE_ACCEPT) {
        char * filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
        
        FILE * fp = fopen(filename, "wt");
        if (!fp) {
            fprintf(stderr, "Failed to open %s", strerror(errno));
        }
        else {
            GtkTextBuffer * textBuffer = gtk_text_view_get_buffer(self->txtExecutionLog);

            GtkTextIter start, end;
            gtk_text_buffer_get_bounds(textBuffer, &start, &end);
            gchar * text = gtk_text_buffer_get_text(textBuffer, &start, &end, false);
            
            if (text) {
                fwrite(text, 1, strlen(text), fp);
            }
            fclose(fp);
        }

        g_free(filename);
    }

    gtk_widget_destroy(dialog);
}

void nesx_debugger_add_log_entry(NESxDebugger * self)
{
    GtkTextBuffer * gtkBuffer = gtk_text_view_get_buffer(self->txtExecutionLog);
    GtkTextIter iter;

    mos6502_t * cpu = &self->nes->CPU;

    char disasm[17];
    snprintf(disasm, sizeof(disasm), cpu->DisasmFormat, cpu->DisasmData1, cpu->DisasmData2);

    char buffer[256];
    snprintf(buffer, sizeof(buffer), NESX_EXECUTION_LOG_FORMAT,
        disasm, 
        cpu->A, 
        cpu->X, 
        cpu->Y, 
        cpu->S, 
        cpu->PC,
        (cpu->FN ? 'N' : 'n'),
        (cpu->FV ? 'V' : 'v'),
        (cpu->FD ? 'D' : 'd'),
        (cpu->FB ? 'B' : 'b'),
        (cpu->FI ? 'I' : 'i'),
        (cpu->FZ ? 'Z' : 'z'),
        (cpu->FC ? 'C' : 'c'),
        cpu->Cycles);
    
    gtk_text_buffer_get_end_iter(gtkBuffer, &iter);
    gtk_text_buffer_insert(gtkBuffer, &iter, buffer, -1);

    GtkTextMark * mark = gtk_text_buffer_get_insert(gtkBuffer);
    gtk_text_view_scroll_to_mark(self->txtExecutionLog, mark, 0.0, false, 0.0, false);
}

void nesx_debugger_display(NESxDebugger * self)
{
    nesx_debugger_display_status(self);
    nesx_debugger_display_registers(self);
    nesx_debugger_display_status_registers(self);
    nesx_debugger_display_cpu_internals(self);
    nesx_debugger_display_ppu_internals(self);
    nesx_debugger_add_log_entry(self);
}

void nesx_debugger_display_status(NESxDebugger * self)
{
    char buffer[32];

    snprintf(buffer, sizeof(buffer), "%lu", self->cpu->Cycles);
    gtk_label_set_text(self->lblCPUCycle, buffer);

    snprintf(buffer, sizeof(buffer), "%d", self->ppu->Cycle);
    gtk_label_set_text(self->lblPPUCycle, buffer);

    snprintf(buffer, sizeof(buffer), "%d", self->ppu->Scanline);
    gtk_label_set_text(self->lblPPUScanline, buffer);
}

void nesx_debugger_display_registers(NESxDebugger * self)
{
    char buffer[5];

    snprintf(buffer, sizeof(buffer), "%02X", self->cpu->A);
    gtk_entry_set_text(self->entA, buffer);

    snprintf(buffer, sizeof(buffer), "%02X", self->cpu->X);
    gtk_entry_set_text(self->entX, buffer);

    snprintf(buffer, sizeof(buffer), "%02X", self->cpu->Y);
    gtk_entry_set_text(self->entY, buffer);

    snprintf(buffer, sizeof(buffer), "%02X", self->cpu->S);
    gtk_entry_set_text(self->entS, buffer);

    snprintf(buffer, sizeof(buffer), "%04X", self->cpu->PC);
    gtk_entry_set_text(self->entPC, buffer);
}

void nesx_debugger_display_status_registers(NESxDebugger * self)
{
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(self->chkFN), self->cpu->FN);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(self->chkFV), self->cpu->FV);
    // gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(self->chkFU), self->cpu->FU);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(self->chkFD), self->cpu->FD);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(self->chkFB), self->cpu->FB);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(self->chkFI), self->cpu->FI);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(self->chkFZ), self->cpu->FZ);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(self->chkFN), self->cpu->FC);
}

void nesx_debugger_display_cpu_internals(NESxDebugger * self)
{
    char buffer[5];

    snprintf(buffer, sizeof(buffer), "%04X", self->cpu->AB);
    gtk_entry_set_text(self->entAB, buffer);

    snprintf(buffer, sizeof(buffer), "%04X", self->cpu->AD);
    gtk_entry_set_text(self->entAD, buffer);

    snprintf(buffer, sizeof(buffer), "%02X", (self->cpu->IR >> 3));
    gtk_entry_set_text(self->entIR, buffer);

    snprintf(buffer, sizeof(buffer), "%02X", self->cpu->DB);
    gtk_entry_set_text(self->entDB, buffer);

    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(self->chkRW), self->cpu->RW);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(self->chkSYNC), self->cpu->SYNC);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(self->chkIRQ), self->cpu->IRQ);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(self->chkNMI), self->cpu->NMI);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(self->chkRDY), self->cpu->RDY);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(self->chkRES), self->cpu->RES);
}

void nesx_debugger_display_ppu_internals(NESxDebugger * self)
{
    char buffer[4];

    snprintf(buffer, sizeof(buffer), "%d", self->ppu->Scanline);
    gtk_entry_set_text(self->entScanline, buffer);

    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(self->chkVBlank), self->ppu->VBlank);
}

void nesx_debugger_apply(NESxDebugger * self)
{
    nesx_debugger_apply_registers(self);
    nesx_debugger_apply_status_registers(self);
    nesx_debugger_apply_cpu_internals(self);
    nesx_debugger_apply_ppu_internals(self);

    nesx_debugger_display(self);
}

void nesx_debugger_apply_registers(NESxDebugger * self)
{
    int value;

    if (sscanf(gtk_entry_get_text(self->entA), "%2X", &value) == 1) {
        self->cpu->A = value;
    }

    if (sscanf(gtk_entry_get_text(self->entX), "%2X", &value) == 1) {
        self->cpu->X = value;
    }

    if (sscanf(gtk_entry_get_text(self->entY), "%2X", &value) == 1) {
        self->cpu->Y = value;
    }

    if (sscanf(gtk_entry_get_text(self->entS), "%2X", &value) == 1) {
        self->cpu->S = value;
    }

    if (sscanf(gtk_entry_get_text(self->entPC), "%4X", &value) == 1) {
        self->cpu->PC = value;
    }
}

void nesx_debugger_apply_status_registers(NESxDebugger * self)
{
    self->cpu->FN = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(self->chkFN));
    self->cpu->FV = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(self->chkFV));
    // self->cpu->FU = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(self->chkFU));
    self->cpu->FD = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(self->chkFD));
    self->cpu->FB = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(self->chkFB));
    self->cpu->FI = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(self->chkFI));
    self->cpu->FZ = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(self->chkFZ));
    self->cpu->FC = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(self->chkFC));
}

void nesx_debugger_apply_cpu_internals(NESxDebugger * self)
{
    int value;
    
    if (sscanf(gtk_entry_get_text(self->entAB), "%4X", &value) == 1) {
        self->cpu->AB = value;
    }
    
    if (sscanf(gtk_entry_get_text(self->entAD), "%4X", &value) == 1) {
        self->cpu->AD = value;
    }
    
    if (sscanf(gtk_entry_get_text(self->entIR), "%2X", &value) == 1) {
        self->cpu->IR = value;
    }
    
    if (sscanf(gtk_entry_get_text(self->entDB), "%2X", &value) == 1) {
        self->cpu->DB = value;
    }

    self->cpu->RW = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(self->chkRW));
    self->cpu->SYNC = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(self->chkSYNC));
    self->cpu->IRQ = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(self->chkIRQ));
    self->cpu->NMI = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(self->chkNMI));
    self->cpu->RDY = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(self->chkRDY));
    self->cpu->RES = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(self->chkRES));
}

void nesx_debugger_apply_ppu_internals(NESxDebugger * self)
{
    int value;
    
    if (sscanf(gtk_entry_get_text(self->entScanline), "%3d", &value) == 1) {
        self->ppu->Scanline = CLAMP(value, 0, 260);
    }

    self->ppu->VBlank = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(self->chkVBlank));
}

void nesx_debugger_display_breakpoints(NESxDebugger * self)
{
    GList * children = gtk_container_get_children(GTK_CONTAINER(self->lstBreakpoints));
    for (GList * iter = children; iter != NULL; iter = g_list_next(iter)) {
        gtk_widget_destroy(GTK_WIDGET(iter->data));
    }
    g_list_free(children);

    char buffer[32];
    NESxBreakpoint * tmp = self->firstBreakpoint;
    while (tmp) {
        GtkBox * box = GTK_BOX(gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0));

        if (tmp->type == BREAKPOINT_TYPE_NUMERIC) {
            switch (tmp->numericTarget)
            {
            case BREAKPOINT_NUMERIC_TARGET_PC:
            case BREAKPOINT_NUMERIC_TARGET_AB:
            case BREAKPOINT_NUMERIC_TARGET_AD:
                snprintf(buffer, sizeof(buffer), "%s %s 0x%04X",
                    nesx_breakpoint_numeric_target_string(tmp->numericTarget),
                    nesx_breakpoint_numeric_comp_string(tmp->numericComp),
                    tmp->value);
                break;
            case BREAKPOINT_NUMERIC_TARGET_A:
            case BREAKPOINT_NUMERIC_TARGET_X:
            case BREAKPOINT_NUMERIC_TARGET_Y:
            case BREAKPOINT_NUMERIC_TARGET_S:
            case BREAKPOINT_NUMERIC_TARGET_DB:
            case BREAKPOINT_NUMERIC_TARGET_IR:
                snprintf(buffer, sizeof(buffer), "%s %s 0x%02X",
                    nesx_breakpoint_numeric_target_string(tmp->numericTarget),
                    nesx_breakpoint_numeric_comp_string(tmp->numericComp),
                    tmp->value);
                break;
            case BREAKPOINT_NUMERIC_TARGET_CPU_CYCLE:
            case BREAKPOINT_NUMERIC_TARGET_PPU_CYCLE:
            case BREAKPOINT_NUMERIC_TARGET_SCANLINE:
                snprintf(buffer, sizeof(buffer), "%s %s %d",
                    nesx_breakpoint_numeric_target_string(tmp->numericTarget),
                    nesx_breakpoint_numeric_comp_string(tmp->numericComp),
                    tmp->value);
                break;
            }
        }
        else {
            snprintf(buffer, sizeof(buffer), "%s %s",
                nesx_breakpoint_flag_target_string(tmp->flagTarget),
                nesx_breakpoint_flag_trigger_string(tmp->flagTrigger));
        }

        GtkLabel * label = GTK_LABEL(gtk_label_new(buffer));
        gtk_widget_set_margin_start(GTK_WIDGET(label), 8);
        gtk_label_set_xalign(label, 0.0f);
        gtk_box_pack_start(box, GTK_WIDGET(label), true, true, 0);

        GtkButton * button = GTK_BUTTON(gtk_button_new_from_icon_name("edit-delete-symbolic", GTK_ICON_SIZE_BUTTON));
        g_signal_connect_swapped(G_OBJECT(button), "clicked", G_CALLBACK(nesx_debugger_remove_breakpoint), self);
        gtk_box_pack_start(box, GTK_WIDGET(button), false, false, 0);

        gtk_container_add(GTK_CONTAINER(self->lstBreakpoints), GTK_WIDGET(box));

        tmp = tmp->next;
    }

    gtk_widget_show_all(GTK_WIDGET(self->lstBreakpoints));
}

void nesx_debugger_clear_breakpoints(NESxDebugger * self)
{
    nesx_breakpoint_list_clear(&self->firstBreakpoint);
    nesx_debugger_display_breakpoints(self);
}

void nesx_debugger_add_breakpoint(NESxDebugger * self)
{
    NESxBreakpointDialog * dialog = NESX_BREAKPOINT_DIALOG(nesx_breakpoint_dialog_new(GTK_WINDOW(self)));
    
    gint result = gtk_dialog_run(GTK_DIALOG(dialog));
    if (result == GTK_RESPONSE_APPLY) {
        int type = nesx_breakpoint_dialog_get_breakpoint_type(dialog);
        NESxBreakpoint * brk = nesx_breakpoint_list_add(&self->firstBreakpoint);
        brk->type = type;
        if (type == BREAKPOINT_TYPE_NUMERIC) {
            brk->numericTarget = nesx_breakpoint_dialog_get_numeric_target(dialog);
            brk->numericComp = nesx_breakpoint_dialog_get_numeric_comp(dialog);

            const char * value = nesx_breakpoint_dialog_get_numeric_value(dialog);
            if (strncmp("0x", value, 2) == 0) {
                sscanf(value + 2, "%X", &brk->value);
            }
            else {
                sscanf(value, "%d", &brk->value);
            }
        }
        else {
            brk->flagTarget = nesx_breakpoint_dialog_get_flag_target(dialog);
            brk->flagTrigger = nesx_breakpoint_dialog_get_flag_trigger(dialog);
        }

        nesx_debugger_display_breakpoints(self);

        if (nesx_breakpoint_list_check(self->firstBreakpoint, self->nes)) {
            *(self->running) = false;
        }
    }

    gtk_widget_destroy(GTK_WIDGET(dialog));
}

void nesx_debugger_remove_breakpoint(NESxDebugger * self, GtkButton * button)
{
    GtkWidget * box = gtk_widget_get_parent(GTK_WIDGET(button));
    GtkListBoxRow * row = GTK_LIST_BOX_ROW(gtk_widget_get_parent(box));
    int index = gtk_list_box_row_get_index(row);

    nesx_breakpoint_list_remove(&self->firstBreakpoint, index);

    nesx_debugger_display_breakpoints(self);
}
