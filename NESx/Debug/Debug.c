#include "Debug.h"

#include "resource.h"
#include "Mapper/NROM.h"

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

void _DebugMemoryRegionChanged(GtkComboBox * cmb, debug_ctx_t * ctx)
{
    int index = gtk_combo_box_get_active(cmb);
    int page = gtk_notebook_get_current_page(ctx->nbkMemoryView);
    DebugMemoryView * mem
        = DEBUG_MEMORY_VIEW(gtk_notebook_get_nth_page(ctx->nbkMemoryView, page));
    assert(mem);

    debug_memory_view_scroll_to_address(mem, ctx->memoryRegions[index].address);
}

void _DebugSetMemoryRegions(debug_ctx_t * ctx, mem_region_select_t * regions)
{
    ctx->memoryRegions = regions;

    gtk_combo_box_text_remove_all(ctx->cmbMemoryRegion);

    int i = 0;
    while (regions[i].name) {
        gtk_combo_box_text_append_text(ctx->cmbMemoryRegion, regions[i].name);
        ++i;
    }

    gtk_combo_box_set_active(GTK_COMBO_BOX(ctx->cmbMemoryRegion), 0);
}

void _DebugUpdateStatus(debug_ctx_t * ctx)
{
    char buffer[32];

    snprintf(buffer, sizeof(buffer), "%lu", ctx->cpu->Cycles);
    gtk_label_set_text(ctx->lblCPUCycles, buffer);

    snprintf(buffer, sizeof(buffer), "%d", ctx->ppu->Cycle);
    gtk_label_set_text(ctx->lblPPUCycle, buffer);

    snprintf(buffer, sizeof(buffer), "%d", ctx->ppu->Scanline);
    gtk_label_set_text(ctx->lblPPUScanline, buffer);
}

void _DebugUpdateRegisters(debug_ctx_t * ctx)
{
    char buffer[5];

    snprintf(buffer, sizeof(buffer), "%02X", ctx->cpu->A);
    gtk_entry_set_text(ctx->entA, buffer);

    snprintf(buffer, sizeof(buffer), "%02X", ctx->cpu->X);
    gtk_entry_set_text(ctx->entX, buffer);

    snprintf(buffer, sizeof(buffer), "%02X", ctx->cpu->Y);
    gtk_entry_set_text(ctx->entY, buffer);

    snprintf(buffer, sizeof(buffer), "%02X", ctx->cpu->S);
    gtk_entry_set_text(ctx->entS, buffer);

    snprintf(buffer, sizeof(buffer), "%04X", ctx->cpu->PC);
    gtk_entry_set_text(ctx->entPC, buffer);
}

void _DebugUpdateStatusRegisters(debug_ctx_t * ctx)
{
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(ctx->chkFN), ctx->cpu->FN);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(ctx->chkFV), ctx->cpu->FV);
    // gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(ctx->chkFU),
    // ctx->cpu->FU);
    // gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(ctx->chkFB),
    // ctx->cpu->FB);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(ctx->chkFD), ctx->cpu->FD);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(ctx->chkFI), ctx->cpu->FI);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(ctx->chkFZ), ctx->cpu->FZ);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(ctx->chkFN), ctx->cpu->FC);
}

void _DebugUpdateCPUInternals(debug_ctx_t * ctx)
{
    char buffer[5];

    snprintf(buffer, sizeof(buffer), "%04X", ctx->cpu->AB);
    gtk_entry_set_text(ctx->entAB, buffer);

    snprintf(buffer, sizeof(buffer), "%04X", ctx->cpu->AD);
    gtk_entry_set_text(ctx->entAD, buffer);

    snprintf(buffer, sizeof(buffer), "%02X", (ctx->cpu->IR >> 3));
    gtk_entry_set_text(ctx->entIR, buffer);

    snprintf(buffer, sizeof(buffer), "%02X", ctx->cpu->DB);
    gtk_entry_set_text(ctx->entDB, buffer);

    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(ctx->chkRW), ctx->cpu->RW);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(ctx->chkSYNC), ctx->cpu->SYNC);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(ctx->chkIRQ), ctx->cpu->IRQ);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(ctx->chkNMI), ctx->cpu->NMI);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(ctx->chkRDY), ctx->cpu->RDY);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(ctx->chkRES), ctx->cpu->RES);
}

void _DebugUpdate(debug_ctx_t * ctx)
{
    _DebugUpdateStatus(ctx);
    _DebugUpdateRegisters(ctx);
    _DebugUpdateStatusRegisters(ctx);
    _DebugUpdateCPUInternals(ctx);
}

void _DebugTick(GtkButton * btn, debug_ctx_t * ctx)
{
    NESx_Tick(ctx->nes);
    _DebugUpdate(ctx);
}

void _DebugStep(GtkButton * btn, debug_ctx_t * ctx)
{
    NESx_Step(ctx->nes);
    _DebugUpdate(ctx);
}

void _DebugRun(GtkButton * btn, debug_ctx_t * ctx)
{
    ctx->running = true;
}

void _DebugStop(GtkButton * btn, debug_ctx_t * ctx)
{
    ctx->running = false;
}

void _DebugDestroy(GtkWindow * window, debug_ctx_t * ctx)
{
    ctx->windowOpen = false;
}

bool DebugRun(nesx_t * nes, SDL_Window * window, int argc, char ** argv)
{
    debug_ctx_t ctx;
    ctx.nes = nes;
    ctx.cpu = &nes->CPU;
    ctx.ppu = &nes->PPU;
    ctx.mmu = &nes->MMU;
    ctx.hdr = &nes->ROM.Header;

    ctx.app = gtk_application_new("com.stephenlw.nesx", G_APPLICATION_FLAGS_NONE);

    GMainContext * main = g_main_context_default();
    if (!g_main_context_acquire(main)) {
        fprintf(stderr, "failed to acquire main context\n");
        return false;
    }

    GError * error = NULL;
    if (!g_application_register(G_APPLICATION(ctx.app), NULL, &error)) {
        fprintf(stderr, "failed to register application: %s\n", error->message);
        g_error_free(error);
        return false;
    }

    GBytes * data = g_resource_lookup_data(
        debugger_get_resource(), "/debugger.glade", 0, &error);
    if (!data) {
        fprintf(stderr, "failed to load glade: %s\n", error->message);
        g_error_free(error);
        return false;
    }

    GtkBuilder * builder
        = gtk_builder_new_from_string((char *)g_bytes_get_data(data, NULL), -1);
    if (!builder) {
        fprintf(stderr, "failed to load builder\n");
    }

    ctx.window = GTK_APPLICATION_WINDOW(gtk_builder_get_object(builder, "wnd_main"));
    gtk_window_set_default_size(GTK_WINDOW(ctx.window), 640, 800);
    g_signal_connect(G_OBJECT(ctx.window), "destroy", G_CALLBACK(_DebugDestroy), &ctx);

    ctx.lblCPUCycles = GTK_LABEL(gtk_builder_get_object(builder, "lbl_cpu_cycles"));
    ctx.lblPPUCycle = GTK_LABEL(gtk_builder_get_object(builder, "lbl_ppu_cycle"));
    ctx.lblPPUScanline = GTK_LABEL(gtk_builder_get_object(builder, "lbl_ppu_scanline"));

    ctx.btnRun = GTK_BUTTON(gtk_builder_get_object(builder, "btn_run"));
    ctx.btnStop = GTK_BUTTON(gtk_builder_get_object(builder, "btn_stop"));
    ctx.btnTick = GTK_BUTTON(gtk_builder_get_object(builder, "btn_tick"));
    ctx.btnStep = GTK_BUTTON(gtk_builder_get_object(builder, "btn_step"));

    g_signal_connect(G_OBJECT(ctx.btnRun), "clicked", G_CALLBACK(_DebugRun), &ctx);
    g_signal_connect(G_OBJECT(ctx.btnStop), "clicked", G_CALLBACK(_DebugStop), &ctx);
    g_signal_connect(G_OBJECT(ctx.btnTick), "clicked", G_CALLBACK(_DebugTick), &ctx);
    g_signal_connect(G_OBJECT(ctx.btnStep), "clicked", G_CALLBACK(_DebugStep), &ctx);

    ctx.chkFN = GTK_CHECK_BUTTON(gtk_builder_get_object(builder, "chk_fn"));
    ctx.chkFV = GTK_CHECK_BUTTON(gtk_builder_get_object(builder, "chk_fv"));
    ctx.chkFU = GTK_CHECK_BUTTON(gtk_builder_get_object(builder, "chk_fu"));
    ctx.chkFB = GTK_CHECK_BUTTON(gtk_builder_get_object(builder, "chk_fb"));
    ctx.chkFD = GTK_CHECK_BUTTON(gtk_builder_get_object(builder, "chk_fd"));
    ctx.chkFI = GTK_CHECK_BUTTON(gtk_builder_get_object(builder, "chk_fi"));
    ctx.chkFZ = GTK_CHECK_BUTTON(gtk_builder_get_object(builder, "chk_fz"));
    ctx.chkFC = GTK_CHECK_BUTTON(gtk_builder_get_object(builder, "chk_fc"));

    ctx.entA = GTK_ENTRY(gtk_builder_get_object(builder, "ent_a"));
    ctx.entX = GTK_ENTRY(gtk_builder_get_object(builder, "ent_x"));
    ctx.entY = GTK_ENTRY(gtk_builder_get_object(builder, "ent_y"));
    ctx.entS = GTK_ENTRY(gtk_builder_get_object(builder, "ent_s"));
    ctx.entPC = GTK_ENTRY(gtk_builder_get_object(builder, "ent_pc"));

    ctx.entAB = GTK_ENTRY(gtk_builder_get_object(builder, "ent_ab"));
    ctx.entAD = GTK_ENTRY(gtk_builder_get_object(builder, "ent_ad"));
    ctx.entIR = GTK_ENTRY(gtk_builder_get_object(builder, "ent_ir"));
    ctx.entDB = GTK_ENTRY(gtk_builder_get_object(builder, "ent_db"));

    ctx.chkRW = GTK_CHECK_BUTTON(gtk_builder_get_object(builder, "chk_rw"));
    ctx.chkSYNC = GTK_CHECK_BUTTON(gtk_builder_get_object(builder, "chk_sync"));
    ctx.chkIRQ = GTK_CHECK_BUTTON(gtk_builder_get_object(builder, "chk_irq"));
    ctx.chkNMI = GTK_CHECK_BUTTON(gtk_builder_get_object(builder, "chk_nmi"));
    ctx.chkRDY = GTK_CHECK_BUTTON(gtk_builder_get_object(builder, "chk_rdy"));
    ctx.chkRES = GTK_CHECK_BUTTON(gtk_builder_get_object(builder, "chk_res"));

    ctx.nbkMemoryView = GTK_NOTEBOOK(gtk_builder_get_object(builder, "nbk_memory_view"));

    ctx.memCPU = DEBUG_MEMORY_VIEW(debug_memory_view_new());
    gtk_notebook_append_page(ctx.nbkMemoryView,
        GTK_WIDGET(ctx.memCPU),
        GTK_WIDGET(gtk_label_new("CPU")));

    ctx.memPPU = DEBUG_MEMORY_VIEW(debug_memory_view_new());
    gtk_notebook_append_page(ctx.nbkMemoryView,
        GTK_WIDGET(ctx.memPPU),
        GTK_WIDGET(gtk_label_new("PPU")));

    ctx.memCartridge = DEBUG_MEMORY_VIEW(debug_memory_view_new());
    gtk_notebook_append_page(ctx.nbkMemoryView,
        GTK_WIDGET(ctx.memCartridge),
        GTK_WIDGET(gtk_label_new("Cartridge")));

    ctx.cmbMemoryRegion = GTK_COMBO_BOX_TEXT(
        gtk_builder_get_object(builder, "cmb_memory_region_select"));
        
    g_signal_connect(G_OBJECT(ctx.cmbMemoryRegion),
        "changed", G_CALLBACK(_DebugMemoryRegionChanged), &ctx);
    _DebugSetMemoryRegions(&ctx, CPU_MEMORY_SECTIONS);

    char buffer[64];

    ctx.lblPRGROMSize = GTK_LABEL(gtk_builder_get_object(builder, "lbl_prg_rom_size"));
    snprintf(buffer, sizeof(buffer), "%d x 16kB", ctx.hdr->PRGROMBanks);
    gtk_label_set_text(ctx.lblPRGROMSize, buffer);

    ctx.lblCHRROMSize = GTK_LABEL(gtk_builder_get_object(builder, "lbl_chr_rom_size"));
    snprintf(buffer, sizeof(buffer), "%d x 8kB", ctx.hdr->CHRROMBanks);
    gtk_label_set_text(ctx.lblCHRROMSize, buffer);

    ctx.lblPRGRAMSize = GTK_LABEL(gtk_builder_get_object(builder, "lbl_prg_ram_size"));
    snprintf(buffer, sizeof(buffer), "%d x 8kB", ctx.hdr->PRGRAMBanks);
    gtk_label_set_text(ctx.lblPRGRAMSize, buffer);

    ctx.lblMapperNumber = GTK_LABEL(gtk_builder_get_object(builder, "lbl_mapper_number"));
    snprintf(buffer, sizeof(buffer), "%s (%d)", NESx_GetMapperName(ctx.nes), ctx.hdr->MapperNumber);
    gtk_label_set_text(ctx.lblMapperNumber, buffer);

    ctx.lblMirrorType = GTK_LABEL(gtk_builder_get_object(builder, "lbl_mirror_type"));
    gtk_label_set_text(ctx.lblMirrorType, (ctx.hdr->MirrorType ? "V-Mirror" : "H-Mirror"));

    _DebugUpdate(&ctx);

    gtk_widget_show_all(GTK_WIDGET(ctx.window));

    switch (ctx.hdr->MapperNumber) {
    case 0: // NROM
        _Debug_NROM(&ctx);
        break;
    }

    SDL_Event event;

    ctx.windowOpen = true;
    ctx.running = false;
    while (ctx.windowOpen) {
        while (g_main_context_iteration(main, false)) { }
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                ctx.windowOpen = false;
            }
        }

        if (ctx.running) {
            NESx_Frame(ctx.nes);
            _DebugUpdate(&ctx);
        }

        SDL_GL_SwapWindow(window);
    }

    g_main_context_release(main);
    g_object_unref(ctx.app);

    return true;
}
