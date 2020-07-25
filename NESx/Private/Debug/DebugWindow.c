#include "DebugWindow.h"

#include "Mapper/NROM.h"
#include "Resource.h"

#define MARGIN 8

mem_region_select_t CPU_MEMORY_SECTIONS[] = {
    { 0x0000, "0000: Internal RAM" },
    { 0x2000, "2000: PPU Registers" },
    { 0x4000, "4000: APU and I/O Registers" },
    { 0x4020, "4020: ROM" },
    { 0, NULL }
    // TODO: Generate list based on mapper
};

mem_region_select_t PPU_MEMORY_SECTIONS[] = { 
    { 0x1000, "1000: Pattern Tables" },
    { 0x2000, "2000: Name Tables" },
    { 0x3F00, "3F00: Palette RAM" },
    { 0, NULL }
    //
};

// void _DebugMemoryRegionChanged(GtkComboBox * cmb, NESxDebugWindow * win)
// {
//     int index = gtk_combo_box_get_active(cmb);
//     int page = gtk_notebook_get_current_page(win->nbkMemoryView);
//     DebugMemoryView * mem
//         = DEBUG_MEMORY_VIEW(gtk_notebook_get_nth_page(win->nbkMemoryView, page));
//     assert(mem);

//     debug_memory_view_scroll_to_address(mem, win->memoryRegions[index].address);
// }

// void _DebugSetMemoryRegions(NESxDebugWindow * win, mem_region_select_t * regions)
// {
//     win->memoryRegions = regions;

//     gtk_combo_box_text_remove_all(win->cmbMemoryRegion);

//     int i = 0;
//     while (regions[i].name) {
//         gtk_combo_box_text_append_text(win->cmbMemoryRegion, regions[i].name);
//         ++i;
//     }

//     gtk_combo_box_set_active(GTK_COMBO_BOX(win->cmbMemoryRegion), 0);
// }

void _DebugUpdateStatus(NESxDebugWindow * win)
{
    char buffer[32];

    snprintf(buffer, sizeof(buffer), "%lu", win->cpu->Cycles);
    gtk_label_set_text(win->lblCPUCycles, buffer);

    snprintf(buffer, sizeof(buffer), "%d", win->ppu->Cycle);
    gtk_label_set_text(win->lblPPUCycle, buffer);

    snprintf(buffer, sizeof(buffer), "%d", win->ppu->Scanline);
    gtk_label_set_text(win->lblPPUScanline, buffer);
}

void _DebugUpdateRegisters(NESxDebugWindow * win)
{
    char buffer[5];

    snprintf(buffer, sizeof(buffer), "%02X", win->cpu->A);
    gtk_entry_set_text(win->entA, buffer);

    snprintf(buffer, sizeof(buffer), "%02X", win->cpu->X);
    gtk_entry_set_text(win->entX, buffer);

    snprintf(buffer, sizeof(buffer), "%02X", win->cpu->Y);
    gtk_entry_set_text(win->entY, buffer);

    snprintf(buffer, sizeof(buffer), "%02X", win->cpu->S);
    gtk_entry_set_text(win->entS, buffer);

    snprintf(buffer, sizeof(buffer), "%04X", win->cpu->PC);
    gtk_entry_set_text(win->entPC, buffer);
}

void _DebugUpdateStatusRegisters(NESxDebugWindow * win)
{
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(win->chkFN), win->cpu->FN);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(win->chkFV), win->cpu->FV);
    // gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(win->chkFU),
    // win->cpu->FU);
    // gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(win->chkFB),
    // win->cpu->FB);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(win->chkFD), win->cpu->FD);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(win->chkFI), win->cpu->FI);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(win->chkFZ), win->cpu->FZ);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(win->chkFN), win->cpu->FC);
}

void _DebugUpdateCPUInternals(NESxDebugWindow * win)
{
    char buffer[5];

    snprintf(buffer, sizeof(buffer), "%04X", win->cpu->AB);
    gtk_entry_set_text(win->entAB, buffer);

    snprintf(buffer, sizeof(buffer), "%04X", win->cpu->AD);
    gtk_entry_set_text(win->entAD, buffer);

    snprintf(buffer, sizeof(buffer), "%02X", (win->cpu->IR >> 3));
    gtk_entry_set_text(win->entIR, buffer);

    snprintf(buffer, sizeof(buffer), "%02X", win->cpu->DB);
    gtk_entry_set_text(win->entDB, buffer);

    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(win->chkRW), win->cpu->RW);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(win->chkSYNC), win->cpu->SYNC);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(win->chkIRQ), win->cpu->IRQ);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(win->chkNMI), win->cpu->NMI);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(win->chkRDY), win->cpu->RDY);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(win->chkRES), win->cpu->RES);
}

void _DebugUpdate(NESxDebugWindow * win)
{
    _DebugUpdateStatus(win);
    _DebugUpdateRegisters(win);
    _DebugUpdateStatusRegisters(win);
    _DebugUpdateCPUInternals(win);
}

void _DebugTick(GtkButton * btn, NESxDebugWindow * win)
{
    NESx_Tick(win->nes);
    _DebugUpdate(win);
}

void _DebugStep(GtkButton * btn, NESxDebugWindow * win)
{
    NESx_Step(win->nes);
    _DebugUpdate(win);
}

// void _DebugRun(GtkButton * btn, NESxDebugWindow * win)
// {
//     win->running = true;
// }

// void _DebugStop(GtkButton * btn, NESxDebugWindow * win)
// {
//     win->running = false;
// }

// void _DebugDestroy(GtkWindow * window, NESxDebugWindow * win)
// {
//     win->windowOpen = false;
// }

bool DebugRun(nesx_t * nes, int argc, char ** argv)
{
    NESxDebugWindow win;
    win.nes = nes;
    win.cpu = &nes->CPU;
    win.ppu = &nes->PPU;
    win.mmu = &nes->MMU;
    win.hdr = &nes->ROM.Header;

    // win.app = gtk_application_new("com.stephenlw.nesx", G_APPLICATION_FLAGS_NONE);

    // GMainContext * main = g_main_context_default();
    // if (!g_main_context_acquire(main)) {
    //     fprintf(stderr, "failed to acquire main context\n");
    //     return false;
    // }

    GError * error = NULL;
    // if (!g_application_register(G_APPLICATION(win.app), NULL, &error)) {
    //     fprintf(stderr, "failed to register application: %s\n", error->message);
    //     g_error_free(error);
    //     return false;
    // }

    GBytes * data = g_resource_lookup_data(nesx_get_resource(), "/debugger.glade", 0, &error);
    if (!data) {
        fprintf(stderr, "failed to load glade: %s\n", error->message);
        g_error_free(error);
        return false;
    }

    GtkBuilder * builder = gtk_builder_new_from_string((char *)g_bytes_get_data(data, NULL), -1);
    if (!builder) {
        fprintf(stderr, "failed to load builder\n");
    }

    // win.window = GTK_APPLICATION_WINDOW(gtk_builder_get_object(builder, "wnd_main"));
    // gtk_window_set_default_size(GTK_WINDOW(win.window), 640, 800);
    // g_signal_connect(G_OBJECT(win.window), "destroy", G_CALLBACK(_DebugDestroy), &win);

    win.lblCPUCycles = GTK_LABEL(gtk_builder_get_object(builder, "lbl_cpu_cycles"));
    win.lblPPUCycle = GTK_LABEL(gtk_builder_get_object(builder, "lbl_ppu_cycle"));
    win.lblPPUScanline = GTK_LABEL(gtk_builder_get_object(builder, "lbl_ppu_scanline"));

    win.btnRun = GTK_BUTTON(gtk_builder_get_object(builder, "btn_run"));
    win.btnStop = GTK_BUTTON(gtk_builder_get_object(builder, "btn_stop"));
    win.btnTick = GTK_BUTTON(gtk_builder_get_object(builder, "btn_tick"));
    win.btnStep = GTK_BUTTON(gtk_builder_get_object(builder, "btn_step"));

    // g_signal_connect(G_OBJECT(win.btnRun), "clicked", G_CALLBACK(_DebugRun), &win);
    // g_signal_connect(G_OBJECT(win.btnStop), "clicked", G_CALLBACK(_DebugStop), &win);
    g_signal_connect(G_OBJECT(win.btnTick), "clicked", G_CALLBACK(_DebugTick), &win);
    g_signal_connect(G_OBJECT(win.btnStep), "clicked", G_CALLBACK(_DebugStep), &win);

    win.chkFN = GTK_CHECK_BUTTON(gtk_builder_get_object(builder, "chk_fn"));
    win.chkFV = GTK_CHECK_BUTTON(gtk_builder_get_object(builder, "chk_fv"));
    win.chkFU = GTK_CHECK_BUTTON(gtk_builder_get_object(builder, "chk_fu"));
    win.chkFB = GTK_CHECK_BUTTON(gtk_builder_get_object(builder, "chk_fb"));
    win.chkFD = GTK_CHECK_BUTTON(gtk_builder_get_object(builder, "chk_fd"));
    win.chkFI = GTK_CHECK_BUTTON(gtk_builder_get_object(builder, "chk_fi"));
    win.chkFZ = GTK_CHECK_BUTTON(gtk_builder_get_object(builder, "chk_fz"));
    win.chkFC = GTK_CHECK_BUTTON(gtk_builder_get_object(builder, "chk_fc"));

    win.entA = GTK_ENTRY(gtk_builder_get_object(builder, "ent_a"));
    win.entX = GTK_ENTRY(gtk_builder_get_object(builder, "ent_x"));
    win.entY = GTK_ENTRY(gtk_builder_get_object(builder, "ent_y"));
    win.entS = GTK_ENTRY(gtk_builder_get_object(builder, "ent_s"));
    win.entPC = GTK_ENTRY(gtk_builder_get_object(builder, "ent_pc"));

    win.entAB = GTK_ENTRY(gtk_builder_get_object(builder, "ent_ab"));
    win.entAD = GTK_ENTRY(gtk_builder_get_object(builder, "ent_ad"));
    win.entIR = GTK_ENTRY(gtk_builder_get_object(builder, "ent_ir"));
    win.entDB = GTK_ENTRY(gtk_builder_get_object(builder, "ent_db"));

    win.chkRW = GTK_CHECK_BUTTON(gtk_builder_get_object(builder, "chk_rw"));
    win.chkSYNC = GTK_CHECK_BUTTON(gtk_builder_get_object(builder, "chk_sync"));
    win.chkIRQ = GTK_CHECK_BUTTON(gtk_builder_get_object(builder, "chk_irq"));
    win.chkNMI = GTK_CHECK_BUTTON(gtk_builder_get_object(builder, "chk_nmi"));
    win.chkRDY = GTK_CHECK_BUTTON(gtk_builder_get_object(builder, "chk_rdy"));
    win.chkRES = GTK_CHECK_BUTTON(gtk_builder_get_object(builder, "chk_res"));

    // win.nbkMemoryView = GTK_NOTEBOOK(gtk_builder_get_object(builder, "nbk_memory_view"));

    // win.memCPU = DEBUG_MEMORY_VIEW(debug_memory_view_new());
    // gtk_notebook_append_page(win.nbkMemoryView,
    //     GTK_WIDGET(win.memCPU),
    //     GTK_WIDGET(gtk_label_new("CPU")));

    // win.memPPU = DEBUG_MEMORY_VIEW(debug_memory_view_new());
    // gtk_notebook_append_page(win.nbkMemoryView,
    //     GTK_WIDGET(win.memPPU),
    //     GTK_WIDGET(gtk_label_new("PPU")));

    // win.memCartridge = DEBUG_MEMORY_VIEW(debug_memory_view_new());
    // gtk_notebook_append_page(win.nbkMemoryView,
    //     GTK_WIDGET(win.memCartridge),
    //     GTK_WIDGET(gtk_label_new("Cartridge")));

    // win.cmbMemoryRegion = GTK_COMBO_BOX_TEXT(
    //     gtk_builder_get_object(builder, "cmb_memory_region_select"));
        
    // g_signal_connect(G_OBJECT(win.cmbMemoryRegion),
    //     "changed", G_CALLBACK(_DebugMemoryRegionChanged), &win);
    // _DebugSetMemoryRegions(&win, CPU_MEMORY_SECTIONS);

    // char buffer[64];

    // win.lblPRGROMSize = GTK_LABEL(gtk_builder_get_object(builder, "lbl_prg_rom_size"));
    // snprintf(buffer, sizeof(buffer), "%d x 16kB", win.hdr->PRGROMBanks);
    // gtk_label_set_text(win.lblPRGROMSize, buffer);

    // win.lblCHRROMSize = GTK_LABEL(gtk_builder_get_object(builder, "lbl_chr_rom_size"));
    // snprintf(buffer, sizeof(buffer), "%d x 8kB", win.hdr->CHRROMBanks);
    // gtk_label_set_text(win.lblCHRROMSize, buffer);

    // win.lblPRGRAMSize = GTK_LABEL(gtk_builder_get_object(builder, "lbl_prg_ram_size"));
    // snprintf(buffer, sizeof(buffer), "%d x 8kB", win.hdr->PRGRAMBanks);
    // gtk_label_set_text(win.lblPRGRAMSize, buffer);

    // win.lblMapperNumber = GTK_LABEL(gtk_builder_get_object(builder, "lbl_mapper_number"));
    // snprintf(buffer, sizeof(buffer), "%s (%d)", NESx_GetMapperName(win.nes), win.hdr->MapperNumber);
    // gtk_label_set_text(win.lblMapperNumber, buffer);

    // win.lblMirrorType = GTK_LABEL(gtk_builder_get_object(builder, "lbl_mirror_type"));
    // gtk_label_set_text(win.lblMirrorType, (win.hdr->MirrorType ? "V-Mirror" : "H-Mirror"));

    // _DebugUpdate(&win);

    // gtk_widget_show_all(GTK_WIDGET(win.window));

    // switch (win.hdr->MapperNumber) {
    // case 0: // NROM
    //     _Debug_NROM(&win);
    //     break;
    // }

    // SDL_Event event;

    return true;
}
