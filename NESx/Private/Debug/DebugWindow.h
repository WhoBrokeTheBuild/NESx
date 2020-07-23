#ifndef NESX_DEBUG_WINDOW_H
#define NESX_DEBUG_WINDOW_H

#include <NESx/NESx.h>
#include <gtk/gtk.h>

#include <stdbool.h>

#include "MemoryView.h"

G_DECLARE_FINAL_TYPE(
    NESxDebugWindow,
    nesx_debug_window,
    NESX, DEBUG_WINDOW,
    GtkWindow
)

typedef struct _NESxDebugWindow NESxDebugWindow;

struct _NESxDebugWindow
{
    GtkWindow parent;

    nesx_t * nes;
    mos6502_t * cpu;
    nesx_ppu_t * ppu;
    nesx_mmu_t * mmu;
    nesx_rom_header_t * hdr;

    // Status

    GtkLabel * lblCPUCycles;
    GtkLabel * lblPPUCycle;
    GtkLabel * lblPPUScanline;

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

    GtkEntry * entAB;
    GtkEntry * entAD;
    GtkEntry * entIR;
    GtkEntry * entDB;

    GtkCheckButton * chkRW;
    GtkCheckButton * chkSYNC;
    GtkCheckButton * chkIRQ;
    GtkCheckButton * chkNMI;
    GtkCheckButton * chkRDY;
    GtkCheckButton * chkRES;

    // Disassembly View

};


typedef struct mem_region_select
{
    uint16_t address;
    const char * name;

} mem_region_select_t;

typedef struct debug_ctx
{
    bool windowOpen;

    bool running;

    GtkApplication * app;
    GtkApplicationWindow * window;



    // Memory View

    mem_region_select_t * memoryRegions;

    GtkNotebook * nbkMemoryView;

    GtkComboBoxText * cmbMemoryRegion;

    DebugMemoryView * memCPU;
    DebugMemoryView * memPPU;
    DebugMemoryView * memCartridge;

    // Cartridge Header

    GtkLabel * lblPRGROMSize;
    GtkLabel * lblCHRROMSize;
    GtkLabel * lblPRGRAMSize;
    GtkLabel * lblMapperNumber;
    GtkLabel * lblMirrorType;

} debug_ctx_t;

bool DebugRun(nesx_t * nes, int argc, char ** argv);

#endif // NESX_DEBUG_WINDOW_H