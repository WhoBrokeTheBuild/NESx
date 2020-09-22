#ifndef NESX_BREAKPOINT_H
#define NESX_BREAKPOINT_H

#include <stdbool.h>

#include <NESx/NESx.h>

enum {
    BREAKPOINT_NUMERIC_TARGET_PC = 0,
    BREAKPOINT_NUMERIC_TARGET_A,
    BREAKPOINT_NUMERIC_TARGET_X,
    BREAKPOINT_NUMERIC_TARGET_Y,
    BREAKPOINT_NUMERIC_TARGET_S,
    BREAKPOINT_NUMERIC_TARGET_CPU_CYCLE,
    BREAKPOINT_NUMERIC_TARGET_PPU_CYCLE,
    BREAKPOINT_NUMERIC_TARGET_SCANLINE,
    BREAKPOINT_NUMERIC_TARGET_AB,
    BREAKPOINT_NUMERIC_TARGET_AD,
    BREAKPOINT_NUMERIC_TARGET_DB,
    BREAKPOINT_NUMERIC_TARGET_IR,
};

enum {
    BREAKPOINT_COMP_EQUAL = 0,
    BREAKPOINT_COMP_NOT_EQUAL,
    BREAKPOINT_COMP_GREATER_OR_EQUAL,
    BREAKPOINT_COMP_LESS_OR_EQUAL,
    BREAKPOINT_COMP_GREATER,
    BREAKPOINT_COMP_LESS,
};

enum {
    BREAKPOINT_FLAG_TARGET_IRQ = 0,
    BREAKPOINT_FLAG_TARGET_NMI,
    BREAKPOINT_FLAG_TARGET_VBLANK,
    BREAKPOINT_FLAG_TARGET_FN,
    BREAKPOINT_FLAG_TARGET_FV,
    BREAKPOINT_FLAG_TARGET_FB,
    BREAKPOINT_FLAG_TARGET_FD,
    BREAKPOINT_FLAG_TARGET_FI,
    BREAKPOINT_FLAG_TARGET_FZ,
    BREAKPOINT_FLAG_TARGET_FC,
    BREAKPOINT_FLAG_TARGET_RW,
    BREAKPOINT_FLAG_TARGET_SYNC,
    BREAKPOINT_FLAG_TARGET_RDY,
    BREAKPOINT_FLAG_TARGET_RES,
};

enum {
    BREAKPOINT_STATE_CHANGED = 0,
    BREAKPOINT_STATE_ON,
    BREAKPOINT_STATE_OFF,
};

enum {
    BREAKPOINT_TYPE_NUMERIC = 0,
    BREAKPOINT_TYPE_FLAG,
};

typedef struct _NESxBreakpoint NESxBreakpoint;

struct _NESxBreakpoint
{
    int type;

    int numericTarget;
    int numericComp;
    
    int flagTarget;
    int flagTrigger;

    int value;
    bool lastState;

    struct _NESxBreakpoint * next;
};

const char * nesx_breakpoint_numeric_target_string(int numericTarget);
const char * nesx_breakpoint_numeric_comp_string(int numericComp);
const char * nesx_breakpoint_flag_target_string(int flagTarget);
const char * nesx_breakpoint_flag_trigger_string(int flagTrigger);

NESxBreakpoint * nesx_breakpoint_list_add(NESxBreakpoint ** first);
void nesx_breakpoint_list_clear(NESxBreakpoint ** first);
void nesx_breakpoint_list_remove(NESxBreakpoint ** first, int index);
bool nesx_breakpoint_list_check(NESxBreakpoint * first, nesx_t * nes);

#endif // NESX_BREAKPOINT_H