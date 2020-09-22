#ifndef NESX_DEBUGGER_H
#define NESX_DEBUGGER_H

#include <NESx/NESx.h>
#include <gtk/gtk.h>

#include <stdbool.h>

#include "Debug/Breakpoint.h"
// #include "MemoryView.h"

typedef struct _NESxDebugger NESxDebugger;

G_DECLARE_FINAL_TYPE(
    NESxDebugger,
    nesx_debugger,
    NESX, DEBUGGER,
    GtkWindow
)

struct _NESxDebugger
{
    GtkWindow parent;

    nesx_t * nes;
    mos6502_t * cpu;
    nesx_ppu_t * ppu;
    nesx_mmu_t * mmu;
    nesx_rom_header_t * hdr;

    bool * running;

    NESxBreakpoint * firstBreakpoint;

    GtkLabel * lblStatusBar;

    // Status

    GtkLabel * lblCPUCycle;
    GtkLabel * lblPPUCycle;
    GtkLabel * lblPPUScanline;

    // Controls

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

    // PPU Internals

    GtkEntry * entScanline;

    GtkCheckButton * chkVBlank;

    // Execution Log

    GtkTextView * txtExecutionLogHeader;
    GtkTextView * txtExecutionLog;

    // Breakpoints

    GtkListBox * lstBreakpoints;

};

GtkWidget * nesx_debugger_new(nesx_t * nes, bool * running);

void nesx_debugger_save_execution_log(NESxDebugger * self);
void nesx_debugger_add_log_entry(NESxDebugger * self);

void nesx_debugger_tick(NESxDebugger * self);
void nesx_debugger_step(NESxDebugger * self);
void nesx_debugger_frame(NESxDebugger * self);
void nesx_debugger_run(NESxDebugger * self);
void nesx_debugger_stop(NESxDebugger * self);

void nesx_debugger_display(NESxDebugger * self);
void nesx_debugger_display_status(NESxDebugger * self);
void nesx_debugger_display_registers(NESxDebugger * self);
void nesx_debugger_display_status_registers(NESxDebugger * self);
void nesx_debugger_display_cpu_internals(NESxDebugger * self);
void nesx_debugger_display_ppu_internals(NESxDebugger * self);

void nesx_debugger_apply(NESxDebugger * self);
void nesx_debugger_apply_registers(NESxDebugger * self);
void nesx_debugger_apply_status_registers(NESxDebugger * self);
void nesx_debugger_apply_cpu_internals(NESxDebugger * self);
void nesx_debugger_apply_ppu_internals(NESxDebugger * self);

void nesx_debugger_display_breakpoints(NESxDebugger * self);
void nesx_debugger_add_breakpoint(NESxDebugger * self);
void nesx_debugger_clear_breakpoints(NESxDebugger * self);
void nesx_debugger_remove_breakpoint(NESxDebugger * self, GtkButton * button);

#endif // NESX_DEBUGGER_H