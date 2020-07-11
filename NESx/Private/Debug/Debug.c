#include "Debug.h"

#if defined(NESX_HAVE_GTK3)

#include <gtk/gtk.h>

#include "resource.h"
#include "MemoryView.h"

#define MARGIN 8

typedef struct debug_ctx
{
    bool running;

    bool running2;

    GtkApplication * app;
    GtkApplicationWindow * window;

    nesx_t * nes;
    mos6502_t * cpu;
    nesx_ppu_t * ppu;
    nesx_mmu_t * mmu;

    // Status

    GtkLabel * lblCPUCycles;
    GtkLabel * lblPPUCycles;
    GtkLabel * lblPixel;
    GtkLabel * lblScanline;

    // Controls

    GtkButton * btnRun;
    GtkButton * btnStop;
    GtkButton * btnTick;
    GtkButton * btnStep;

    // Status Register

    GtkCheckButton * chkFN;
    GtkCheckButton * chkFV;
    GtkCheckButton * chkFU;
    GtkCheckButton * chkFB;
    GtkCheckButton * chkFD;
    GtkCheckButton * chkFI;
    GtkCheckButton * chkFZ;
    GtkCheckButton * chkFC;

    // Registers

    GtkEntry * entA;
    GtkEntry * entX;
    GtkEntry * entY;
    GtkEntry * entS;
    GtkEntry * entPC;

    // CPU Internals

    GtkEntry * entIR;
    GtkEntry * entAB;
    GtkEntry * entDB;

    GtkCheckButton * chkRW;
    GtkCheckButton * chkSYNC;
    GtkCheckButton * chkIRQ;
    GtkCheckButton * chkNMI;
    GtkCheckButton * chkRDY;
    GtkCheckButton * chkRES;

    // Disassembly View

    
    // Memory View

    GtkNotebook * nbkMemoryView;

    GtkComboBoxText * cmbMemoryRegion;

    MemoryView * memCPU;
    MemoryView * memPPU;
    MemoryView * memCartridge;


} debug_ctx_t;

typedef struct mem_region_select
{
    uint16_t address;
    const char * name;

} mem_region_select_t;

mem_region_select_t CPU_MEMORY_SECTIONS[] = {
    { 0x0000, "0000: Internal RAM" },
    { 0x2000, "2000: PPU Registers" },
    { 0x4000, "4000: APU and I/O Registers" },
    { 0x4020, "4020: ROM" },
    // TODO: Generate list based on mapper
};

mem_region_select_t PPU_MEMORY_SECTIONS[] = {
    { 0x1000, "1000: Pattern Tables" },
    { 0x2000, "2000: Name Tables" },
    { 0x3F00, "3F00: Palette RAM" },
};

void _DebugMemoryRegionCPU(debug_ctx_t * ctx)
{
    gtk_combo_box_text_remove_all(ctx->cmbMemoryRegion);

    for (int i = 0; i < G_N_ELEMENTS(CPU_MEMORY_SECTIONS); ++i) {
        gtk_combo_box_text_append_text(ctx->cmbMemoryRegion, CPU_MEMORY_SECTIONS[i].name);
    }

    gtk_combo_box_set_active(GTK_COMBO_BOX(ctx->cmbMemoryRegion), 0);
}

void _DebugMemoryRegionPPU(debug_ctx_t * ctx)
{
    gtk_combo_box_text_remove_all(ctx->cmbMemoryRegion);

    for (int i = 0; i < G_N_ELEMENTS(PPU_MEMORY_SECTIONS); ++i) {
        gtk_combo_box_text_append_text(ctx->cmbMemoryRegion, PPU_MEMORY_SECTIONS[i].name);
    }

    gtk_combo_box_set_active(GTK_COMBO_BOX(ctx->cmbMemoryRegion), 0);
}

void _DebugUpdateStatus(debug_ctx_t * ctx)
{
    char buffer[32];

    snprintf(buffer, sizeof(buffer), "%lu", ctx->cpu->Cycles);
    gtk_label_set_text(ctx->lblCPUCycles, buffer);
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
    // gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(ctx->chkFU), ctx->cpu->FU);
    // gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(ctx->chkFB), ctx->cpu->FB);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(ctx->chkFD), ctx->cpu->FD);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(ctx->chkFI), ctx->cpu->FI);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(ctx->chkFZ), ctx->cpu->FZ);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(ctx->chkFN), ctx->cpu->FC);
}

void _DebugUpdate(debug_ctx_t * ctx)
{
    _DebugUpdateStatus(ctx);
    _DebugUpdateRegisters(ctx);
    _DebugUpdateStatusRegisters(ctx);
}

void _DebugTick(GtkButton * btn, debug_ctx_t * ctx)
{
    NESx_Tick(ctx->nes);
    _DebugUpdate(ctx);
}

void _DebugStep(GtkButton * btn, debug_ctx_t * ctx)
{
    do {
        NESx_Tick(ctx->nes);
    }
    while (!ctx->cpu->SYNC);
    _DebugUpdate(ctx);
}

void _DebugRun(GtkButton * btn, debug_ctx_t * ctx)
{
    ctx->running2 = true;
}

void _DebugStop(GtkButton * btn, debug_ctx_t * ctx)
{
    ctx->running2 = false;
}

void _DebugDestroy(GtkWindow * window, debug_ctx_t * ctx)
{
    ctx->running = false;
}

bool DebugInit(nesx_t * nes, int argc, char ** argv)
{
    debug_ctx_t ctx;
    ctx.nes = nes;
    ctx.cpu = &nes->CPU;
    ctx.ppu = &nes->PPU;
    ctx.mmu = &nes->MMU;

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

    GBytes * data = g_resource_lookup_data(debugger_get_resource(), "/debugger.glade", 0, &error);
    if (!data) {
        fprintf(stderr, "failed to load glade: %s\n", error->message);
        g_error_free(error);
        return false;
    }

    GtkBuilder * builder = gtk_builder_new_from_string((char *)g_bytes_get_data(data, NULL), -1);
    if (!builder) {
        fprintf(stderr, "failed to load builder\n");
    }

    ctx.window = GTK_APPLICATION_WINDOW(gtk_builder_get_object(builder, "wnd_main"));
    gtk_window_set_default_size(GTK_WINDOW(ctx.window), 640, 600);
    g_signal_connect(G_OBJECT(ctx.window), "destroy", G_CALLBACK(_DebugDestroy), &ctx);

    ctx.lblCPUCycles = GTK_LABEL(gtk_builder_get_object(builder, "lbl_cpu_cycles"));
    ctx.lblPPUCycles = GTK_LABEL(gtk_builder_get_object(builder, "lbl_ppu_cycles"));
    ctx.lblPixel = GTK_LABEL(gtk_builder_get_object(builder, "lbl_pixel"));
    ctx.lblScanline = GTK_LABEL(gtk_builder_get_object(builder, "lbl_scanline"));

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

    ctx.nbkMemoryView = GTK_NOTEBOOK(gtk_builder_get_object(builder, "nbk_memory_view"));

    ctx.memCPU = MEMORY_VIEW(memory_view_new());
    memory_view_add_region(ctx.memCPU, 0x0000, ctx.nes->MMU.InternalRAM, sizeof(ctx.nes->MMU.InternalRAM));
    memory_view_add_region(ctx.memCPU, 0x2000, ctx.nes->ROM, 0x08); // TODO: Change Data
    memory_view_add_region(ctx.memCPU, 0x4000, ctx.nes->ROM, 0x18); // TODO: Change Data
    memory_view_add_region(ctx.memCPU, 0x4020, ctx.nes->ROM, ctx.nes->ROMSize); // TODO: 
    gtk_notebook_append_page(ctx.nbkMemoryView, GTK_WIDGET(ctx.memCPU), GTK_WIDGET(gtk_label_new("CPU")));

    ctx.memPPU = MEMORY_VIEW(memory_view_new());
    gtk_notebook_append_page(ctx.nbkMemoryView, GTK_WIDGET(ctx.memPPU), GTK_WIDGET(gtk_label_new("PPU")));

    ctx.memCartridge = MEMORY_VIEW(memory_view_new());
    gtk_notebook_append_page(ctx.nbkMemoryView, GTK_WIDGET(ctx.memCartridge), GTK_WIDGET(gtk_label_new("Cartridge")));

    ctx.cmbMemoryRegion = GTK_COMBO_BOX_TEXT(gtk_builder_get_object(builder, "cmb_memory_region_select"));
    _DebugMemoryRegionCPU(&ctx);

    _DebugUpdate(&ctx);

    gtk_widget_show_all(GTK_WIDGET(ctx.window));

    unsigned MAX_CYCLES_PER_FRAME = 100;

    ctx.running = true;
    ctx.running2 = false;
    while (ctx.running) {
        while (g_main_context_iteration(main, false)) { }
        
        if (ctx.running2) {
            for (unsigned i = 0; i < MAX_CYCLES_PER_FRAME; ++i) {
                NESx_Tick(ctx.nes);
            }
            _DebugUpdate(&ctx);
        }
    }

    g_main_context_release(main);
    g_object_unref(ctx.app);

    return true;
}

#else

bool DebugInit(int argc, char ** argv)
{
    (void)argc;
    (void)argv;

    return true;
}

#endif