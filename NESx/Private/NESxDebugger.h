#ifndef NESX_DEBUGGER_H
#define NESX_DEBUGGER_H

#include <NESx/NESx.h>
#include <gtk/gtk.h>

#include <stdbool.h>

// #include "MemoryView.h"

G_DECLARE_FINAL_TYPE(
    NESxDebugger,
    nesx_debugger,
    NESX, DEBUGGER,
    GtkWindow
)

typedef struct _NESxDebugger NESxDebugger;

struct _NESxDebugger
{
    GtkWindow parent;

    nesx_t * nes;
    mos6502_t * cpu;
    nesx_ppu_t * ppu;
    nesx_mmu_t * mmu;
    nesx_rom_header_t * hdr;

    // Status

    GtkLabel * lblCPUCycle;
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

    // Execution Log

};

GtkWidget * nesx_debugger_new(nesx_t * nes);

#endif // NESX_DEBUGGER_H