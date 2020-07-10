#include "Debug.h"

#if defined(NESX_HAVE_GTK3)

#include <gtk/gtk.h>

#include "MemoryView.h"

#define MARGIN 8

typedef struct debug_ctx
{
    bool running;

    bool running2;

    GtkApplication * app;
    GtkWindow * window;

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

GtkLabel * _DebugMakeStatusLabel(const char * text)
{
    GtkLabel * tmp = GTK_LABEL(gtk_label_new(text));
    gtk_label_set_width_chars(tmp, 12);
    gtk_label_set_xalign(tmp, 1.0);
    gtk_widget_set_margin_end(GTK_WIDGET(tmp), MARGIN);
    return tmp;
}

void _DebugInitStatus(debug_ctx_t * ctx, GtkContainer * parent)
{
    GtkGrid * grid = GTK_GRID(gtk_grid_new());
    gtk_widget_set_margin_start(GTK_WIDGET(grid), MARGIN);
    gtk_widget_set_margin_end(GTK_WIDGET(grid), MARGIN);
    gtk_widget_set_margin_top(GTK_WIDGET(grid), MARGIN / 2);
    gtk_widget_set_margin_bottom(GTK_WIDGET(grid), MARGIN);
    gtk_container_add(parent, GTK_WIDGET(grid));

    gtk_grid_attach(grid, GTK_WIDGET(_DebugMakeStatusLabel("CPU Cycles:")), 0, 0, 1, 1);

    ctx->lblCPUCycles = GTK_LABEL(gtk_label_new("0"));
    gtk_grid_attach(grid, GTK_WIDGET(ctx->lblCPUCycles), 1, 0, 1, 1);
    
    gtk_grid_attach(grid, GTK_WIDGET(_DebugMakeStatusLabel("PPU Cycles:")), 0, 1, 1, 1);

    ctx->lblPPUCycles = GTK_LABEL(gtk_label_new("0"));
    gtk_grid_attach(grid, GTK_WIDGET(ctx->lblPPUCycles), 1, 1, 1, 1);
    
    gtk_grid_attach(grid, GTK_WIDGET(_DebugMakeStatusLabel("Pixel:")), 0, 2, 1, 1);

    ctx->lblPixel = GTK_LABEL(gtk_label_new("0"));
    gtk_grid_attach(grid, GTK_WIDGET(ctx->lblPixel), 1, 2, 1, 1);
    
    gtk_grid_attach(grid, GTK_WIDGET(_DebugMakeStatusLabel("Scanline:")), 0, 3, 1, 1);

    ctx->lblScanline = GTK_LABEL(gtk_label_new("0"));
    gtk_grid_attach(grid, GTK_WIDGET(ctx->lblScanline), 1, 3, 1, 1);
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

void _DebugInitControls(debug_ctx_t * ctx, GtkContainer * parent)
{
    GtkGrid * grid = GTK_GRID(gtk_grid_new());
    gtk_widget_set_margin_start(GTK_WIDGET(grid), MARGIN);
    gtk_widget_set_margin_end(GTK_WIDGET(grid), MARGIN);
    gtk_widget_set_margin_top(GTK_WIDGET(grid), MARGIN / 2);
    gtk_widget_set_margin_bottom(GTK_WIDGET(grid), MARGIN);
    gtk_grid_set_column_homogeneous(grid, true);
    gtk_grid_set_row_spacing(grid, MARGIN);
    gtk_grid_set_column_spacing(grid, MARGIN);
    gtk_container_add(parent, GTK_WIDGET(grid));

    ctx->btnRun = GTK_BUTTON(gtk_button_new_with_label("Run"));
    gtk_grid_attach(grid, GTK_WIDGET(ctx->btnRun), 0, 0, 1, 1);
    g_signal_connect(G_OBJECT(ctx->btnRun), "clicked", G_CALLBACK(_DebugRun), ctx);
    
    ctx->btnStop = GTK_BUTTON(gtk_button_new_with_label("Stop"));
    gtk_grid_attach(grid, GTK_WIDGET(ctx->btnStop), 1, 0, 1, 1);
    g_signal_connect(G_OBJECT(ctx->btnStop), "clicked", G_CALLBACK(_DebugStop), ctx);

    ctx->btnTick = GTK_BUTTON(gtk_button_new_with_label("Tick"));
    gtk_grid_attach(grid, GTK_WIDGET(ctx->btnTick), 0, 1, 1, 1);
    g_signal_connect(G_OBJECT(ctx->btnTick), "clicked", G_CALLBACK(_DebugTick), ctx);

    ctx->btnStep = GTK_BUTTON(gtk_button_new_with_label("Step"));
    gtk_grid_attach(grid, GTK_WIDGET(ctx->btnStep), 1, 1, 1, 1);
    g_signal_connect(G_OBJECT(ctx->btnStep), "clicked", G_CALLBACK(_DebugStep), ctx);
}

void _DebugInitStatusRegisters(debug_ctx_t * ctx, GtkContainer * parent)
{
    GtkGrid * grid = GTK_GRID(gtk_grid_new());
    gtk_widget_set_margin_start(GTK_WIDGET(grid), MARGIN);
    gtk_widget_set_margin_end(GTK_WIDGET(grid), MARGIN);
    gtk_widget_set_margin_top(GTK_WIDGET(grid), MARGIN / 2);
    gtk_widget_set_margin_bottom(GTK_WIDGET(grid), MARGIN);
    gtk_grid_set_column_homogeneous(grid, true);
    gtk_container_add(parent, GTK_WIDGET(grid));

    ctx->chkFN = GTK_CHECK_BUTTON(gtk_check_button_new_with_label("N"));
    gtk_grid_attach(grid, GTK_WIDGET(ctx->chkFN), 0, 0, 1, 1);

    ctx->chkFV = GTK_CHECK_BUTTON(gtk_check_button_new_with_label("V"));
    gtk_grid_attach(grid, GTK_WIDGET(ctx->chkFV), 1, 0, 1, 1);

    ctx->chkFU = GTK_CHECK_BUTTON(gtk_check_button_new_with_label("U"));
    gtk_grid_attach(grid, GTK_WIDGET(ctx->chkFU), 2, 0, 1, 1);

    ctx->chkFB = GTK_CHECK_BUTTON(gtk_check_button_new_with_label("B"));
    gtk_grid_attach(grid, GTK_WIDGET(ctx->chkFB), 3, 0, 1, 1);

    ctx->chkFD = GTK_CHECK_BUTTON(gtk_check_button_new_with_label("D"));
    gtk_grid_attach(grid, GTK_WIDGET(ctx->chkFD), 0, 1, 1, 1);

    ctx->chkFI = GTK_CHECK_BUTTON(gtk_check_button_new_with_label("I"));
    gtk_grid_attach(grid, GTK_WIDGET(ctx->chkFI), 1, 1, 1, 1);

    ctx->chkFZ = GTK_CHECK_BUTTON(gtk_check_button_new_with_label("Z"));
    gtk_grid_attach(grid, GTK_WIDGET(ctx->chkFZ), 2, 1, 1, 1);

    ctx->chkFC = GTK_CHECK_BUTTON(gtk_check_button_new_with_label("C"));
    gtk_grid_attach(grid, GTK_WIDGET(ctx->chkFC), 3, 1, 1, 1);

    _DebugUpdateStatusRegisters(ctx);
}

void _DebugInitRegisters(debug_ctx_t * ctx, GtkContainer * parent)
{
    GtkGrid * grid = GTK_GRID(gtk_grid_new());
    gtk_widget_set_margin_start(GTK_WIDGET(grid), MARGIN);
    gtk_widget_set_margin_end(GTK_WIDGET(grid), MARGIN);
    gtk_widget_set_margin_top(GTK_WIDGET(grid), MARGIN / 2);
    gtk_widget_set_margin_bottom(GTK_WIDGET(grid), MARGIN);
    gtk_grid_set_row_spacing(grid, MARGIN);
    gtk_grid_set_column_spacing(grid, MARGIN);
    gtk_container_add(parent, GTK_WIDGET(grid));

    gtk_grid_attach(grid, GTK_WIDGET(gtk_label_new("A:")), 0, 0, 1, 1);

    ctx->entA = GTK_ENTRY(gtk_entry_new());
    gtk_entry_set_width_chars(ctx->entA, 3);
    gtk_entry_set_max_length(ctx->entA, 2);
    gtk_grid_attach(grid, GTK_WIDGET(ctx->entA), 1, 0, 1, 1);

    gtk_grid_attach(grid, GTK_WIDGET(gtk_label_new("X:")), 2, 0, 1, 1);

    ctx->entX = GTK_ENTRY(gtk_entry_new());
    gtk_entry_set_width_chars(ctx->entX, 3);
    gtk_entry_set_max_length(ctx->entX, 2);
    gtk_grid_attach(grid, GTK_WIDGET(ctx->entX), 3, 0, 1, 1);

    gtk_grid_attach(grid, GTK_WIDGET(gtk_label_new("Y:")), 4, 0, 1, 1);

    ctx->entY = GTK_ENTRY(gtk_entry_new());
    gtk_entry_set_width_chars(ctx->entY, 3);
    gtk_entry_set_max_length(ctx->entY, 2);
    gtk_grid_attach(grid, GTK_WIDGET(ctx->entY), 5, 0, 1, 1);

    gtk_grid_attach(grid, GTK_WIDGET(gtk_label_new("S:")), 0, 1, 1, 1);

    ctx->entS = GTK_ENTRY(gtk_entry_new());
    gtk_entry_set_width_chars(ctx->entS, 3);
    gtk_entry_set_max_length(ctx->entS, 2);
    gtk_grid_attach(grid, GTK_WIDGET(ctx->entS), 1, 1, 1, 1);

    gtk_grid_attach(grid, GTK_WIDGET(gtk_label_new("PC:")), 2, 1, 1, 1);

    ctx->entPC = GTK_ENTRY(gtk_entry_new());
    gtk_entry_set_width_chars(ctx->entPC, 5);
    gtk_entry_set_max_length(ctx->entPC, 4);
    gtk_grid_attach(grid, GTK_WIDGET(ctx->entPC), 3, 1, 2, 1);

    _DebugUpdateRegisters(ctx);
}

void _DebugInitCPUInternals(debug_ctx_t * ctx, GtkContainer * parent)
{
}

void _DebugInitMemory(debug_ctx_t * ctx, GtkContainer * parent)
{
    GtkNotebook * nbkMemory = GTK_NOTEBOOK(gtk_notebook_new());
    gtk_container_add(parent, GTK_WIDGET(nbkMemory));

    ctx->memCPU = MEMORY_VIEW(memory_view_new());
    memory_view_add_region(ctx->memCPU, 0x0000, ctx->nes->MMU.InternalRAM, sizeof(ctx->nes->MMU.InternalRAM));
    memory_view_add_region(ctx->memCPU, 0x2000, ctx->nes->ROM, 0x08); // TODO: Change Data
    memory_view_add_region(ctx->memCPU, 0x4000, ctx->nes->ROM, 0x18); // TODO: Change Data
    memory_view_add_region(ctx->memCPU, 0x4020, ctx->nes->ROM, ctx->nes->ROMSize); // TODO: 
    gtk_notebook_append_page(nbkMemory, GTK_WIDGET(ctx->memCPU), GTK_WIDGET(gtk_label_new("CPU")));

    ctx->memPPU = MEMORY_VIEW(memory_view_new());
    gtk_notebook_append_page(nbkMemory, GTK_WIDGET(ctx->memPPU), GTK_WIDGET(gtk_label_new("PPU")));

    ctx->memCartridge = MEMORY_VIEW(memory_view_new());
    gtk_notebook_append_page(nbkMemory, GTK_WIDGET(ctx->memCartridge), GTK_WIDGET(gtk_label_new("Cartridge")));

    ctx->cmbMemoryRegion = GTK_COMBO_BOX_TEXT(gtk_combo_box_text_new());
    _DebugMemoryRegionCPU(ctx);

    gtk_notebook_set_action_widget(nbkMemory, GTK_WIDGET(ctx->cmbMemoryRegion), GTK_PACK_END);
    gtk_widget_show(GTK_WIDGET(ctx->cmbMemoryRegion));
}

void _DebugDestroy(GtkWindow * window, debug_ctx_t * ctx)
{
    ctx->running = false;
}

bool DebugInit(nesx_t * nes, int argc, char ** argv)
{
    int status;

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

    ctx.window = GTK_WINDOW(gtk_application_window_new(ctx.app));
    gtk_window_set_title(GTK_WINDOW(ctx.window), "NESx");
    gtk_window_set_default_size(GTK_WINDOW(ctx.window), 640, 600);
    gtk_widget_show_all(GTK_WIDGET(ctx.window));

    g_signal_connect(G_OBJECT(ctx.window), "destroy", G_CALLBACK(_DebugDestroy), &ctx);


    GtkBox * box1 = GTK_BOX(gtk_box_new(GTK_ORIENTATION_VERTICAL, 0));
    gtk_container_add(GTK_CONTAINER(ctx.window), GTK_WIDGET(box1));

    GtkBox * box2 = GTK_BOX(gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0));
    gtk_widget_set_margin_start(GTK_WIDGET(box2), MARGIN);
    gtk_widget_set_margin_end(GTK_WIDGET(box2), MARGIN);
    gtk_container_add(GTK_CONTAINER(box1), GTK_WIDGET(box2));

    GtkFrame * frmDisassembly = GTK_FRAME(gtk_frame_new(" Disassembly "));
    gtk_frame_set_label_align(frmDisassembly, 0.5f, 0.5f);
    gtk_widget_set_margin_end(GTK_WIDGET(frmDisassembly), MARGIN);
    gtk_container_add(GTK_CONTAINER(box2), GTK_WIDGET(frmDisassembly));
    gtk_box_set_child_packing(GTK_BOX(box2), GTK_WIDGET(frmDisassembly), true, true, 0, GTK_PACK_START);

    GtkBox * box3 = GTK_BOX(gtk_box_new(GTK_ORIENTATION_VERTICAL, 0));
    gtk_container_add(GTK_CONTAINER(box2), GTK_WIDGET(box3));


    GtkFrame * frmStatus = GTK_FRAME(gtk_frame_new(" Status "));
    gtk_frame_set_label_align(frmStatus, 0.5f, 0.5f);
    gtk_container_add(GTK_CONTAINER(box3), GTK_WIDGET(frmStatus));
    
    _DebugInitStatus(&ctx, GTK_CONTAINER(frmStatus));


    GtkFrame * frmControls = GTK_FRAME(gtk_frame_new(" Controls "));
    gtk_frame_set_label_align(frmControls, 0.5f, 0.5f);
    gtk_container_add(GTK_CONTAINER(box3), GTK_WIDGET(frmControls));

    _DebugInitControls(&ctx, GTK_CONTAINER(frmControls));

    GtkFrame * frmRegisters = GTK_FRAME(gtk_frame_new(" Registers "));
    gtk_frame_set_label_align(frmRegisters, 0.5f, 0.5f);
    gtk_container_add(GTK_CONTAINER(box3), GTK_WIDGET(frmRegisters));

    _DebugInitRegisters(&ctx, GTK_CONTAINER(frmRegisters));

    GtkFrame * frmStatusRegisters = GTK_FRAME(gtk_frame_new(" Status Registers "));
    gtk_frame_set_label_align(frmStatusRegisters, 0.5f, 0.5f);
    gtk_container_add(GTK_CONTAINER(box3), GTK_WIDGET(frmStatusRegisters));
    
    _DebugInitStatusRegisters(&ctx, GTK_CONTAINER(frmStatusRegisters));

    GtkFrame * frmCPU = GTK_FRAME(gtk_frame_new(" CPU Internals "));
    gtk_frame_set_label_align(frmCPU, 0.5f, 0.5f);
    gtk_container_add(GTK_CONTAINER(box3), GTK_WIDGET(frmCPU));

    _DebugInitCPUInternals(&ctx, GTK_CONTAINER(frmCPU));

    GtkFrame * frmMemory = GTK_FRAME(gtk_frame_new(" Memory "));
    gtk_frame_set_label_align(frmMemory, 0.5f, 0.5f);
    gtk_container_add(GTK_CONTAINER(box1), GTK_WIDGET(frmMemory));
    gtk_box_set_child_packing(box1, GTK_WIDGET(frmMemory), true, true, 0, GTK_PACK_START);

    _DebugInitMemory(&ctx, GTK_CONTAINER(frmMemory));

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