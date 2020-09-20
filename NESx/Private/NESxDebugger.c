#include "NESxDebugger.h"

// #include "Mapper/NROM.h"
#include "Resource.h"

#define MARGIN 8
#define NESX_EXECUTION_LOG_HEADER "INSTRUCTION      A  X  Y  S  PC   P      CYCLE          "
#define NESX_EXECUTION_LOG_FORMAT "%-16s %02X %02X %02X %02X %04X %c%c%c%c%c%c %lu\n"

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
    return GTK_WIDGET(debugger);
}

void nesx_debugger_init(NESxDebugger * self)
{
    gtk_widget_init_template(GTK_WIDGET(self));

    gtk_text_buffer_set_text(gtk_text_view_get_buffer(self->txtExecutionLogHeader), 
        NESX_EXECUTION_LOG_HEADER, -1);
}

void nesx_debugger_class_init(NESxDebuggerClass * klass)
{
    GtkWidgetClass * wc = GTK_WIDGET_CLASS(klass);

    GError * error = NULL;
    GBytes * data = g_resource_lookup_data(nesx_get_resource(), "/NESxDebugger.glade", 0, &error);
    if (!data) {
        fprintf(stderr, "failed to load NESxDebugger.glade %s\n", error->message);
        g_error_free(error);
    }

    gtk_widget_class_set_template(wc, data);

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

    // Execution Log
    gtk_widget_class_bind_template_child(wc, NESxDebugger, txtExecutionLogHeader);
    gtk_widget_class_bind_template_child(wc, NESxDebugger, txtExecutionLog);
}

void nesx_debugger_tick(NESxDebugger * self)
{
    NESx_Tick(self->nes);
    nesx_debugger_update(self);
}

void nesx_debugger_step(NESxDebugger * self)
{
    NESx_Step(self->nes);
    nesx_debugger_update(self);
}

void nesx_debugger_frame(NESxDebugger * self)
{
    NESx_Frame(self->nes);
    nesx_debugger_update(self);
}

void nesx_debugger_run(NESxDebugger * self)
{
    *(self->running) = true;
}

void nesx_debugger_stop(NESxDebugger * self)
{
    *(self->running) = false;
    nesx_debugger_update(self);
}

void nesx_debugger_update(NESxDebugger * self)
{
    nesx_debugger_update_status(self);
    nesx_debugger_update_registers(self);
    nesx_debugger_update_status_registers(self);
    nesx_debugger_update_cpu_internals(self);
    nesx_debugger_add_log_entry(self);
}

void nesx_debugger_update_status(NESxDebugger * self)
{
    char buffer[32];

    snprintf(buffer, sizeof(buffer), "%lu", self->cpu->Cycles);
    gtk_label_set_text(self->lblCPUCycle, buffer);

    snprintf(buffer, sizeof(buffer), "%d", self->ppu->Cycle);
    gtk_label_set_text(self->lblPPUCycle, buffer);

    snprintf(buffer, sizeof(buffer), "%d", self->ppu->Scanline);
    gtk_label_set_text(self->lblPPUScanline, buffer);
}

void nesx_debugger_update_registers(NESxDebugger * self)
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

void nesx_debugger_update_status_registers(NESxDebugger * self)
{
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(self->chkFN), self->cpu->FN);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(self->chkFV), self->cpu->FV);
    // gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(self->chkFU),
    // self->cpu->FU);
    // gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(self->chkFB),
    // self->cpu->FB);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(self->chkFD), self->cpu->FD);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(self->chkFI), self->cpu->FI);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(self->chkFZ), self->cpu->FZ);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(self->chkFN), self->cpu->FC);
}

void nesx_debugger_update_cpu_internals(NESxDebugger * self)
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

void nesx_debugger_add_log_entry(NESxDebugger * self)
{
    GtkTextBuffer * gtkBuffer = gtk_text_view_get_buffer(self->txtExecutionLog);
    GtkTextIter iter;

    mos6502_t * cpu = &self->nes->CPU;

    char buffer[256];
    snprintf(buffer, sizeof(buffer), NESX_EXECUTION_LOG_FORMAT,
        cpu->Disassembly, 
        cpu->A, 
        cpu->X, 
        cpu->Y, 
        cpu->S, 
        cpu->PC,
        (cpu->FN ? 'N' : 'n'),
        (cpu->FV ? 'V' : 'v'),
        (cpu->FD ? 'D' : 'd'),
        (cpu->FI ? 'I' : 'i'),
        (cpu->FZ ? 'Z' : 'z'),
        (cpu->FC ? 'C' : 'c'),
        cpu->Cycles);
    
    gtk_text_buffer_get_end_iter(gtkBuffer, &iter);
    gtk_text_buffer_insert(gtkBuffer, &iter, buffer, -1);

    GtkTextMark * mark = gtk_text_buffer_get_insert(gtkBuffer);
    gtk_text_view_scroll_to_mark(self->txtExecutionLog, mark, 0.0, false, 0.0, false);
}