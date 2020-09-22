#include "Breakpoint.h"

#include <stdio.h>
#include <stdlib.h>

const char * nesx_breakpoint_numeric_target_string(int numericTarget)
{
    switch (numericTarget)
    {
    case BREAKPOINT_NUMERIC_TARGET_PC:
        return "PC";
    case BREAKPOINT_NUMERIC_TARGET_A:
        return "A";
    case BREAKPOINT_NUMERIC_TARGET_X:
        return "X";
    case BREAKPOINT_NUMERIC_TARGET_Y:
        return "Y";
    case BREAKPOINT_NUMERIC_TARGET_S:
        return "S";
    case BREAKPOINT_NUMERIC_TARGET_CPU_CYCLE:
        return "CPU Cycle";
    case BREAKPOINT_NUMERIC_TARGET_PPU_CYCLE:
        return "PPU Cycle";
    case BREAKPOINT_NUMERIC_TARGET_SCANLINE:
        return "Scanline";
    case BREAKPOINT_NUMERIC_TARGET_AB:
        return "AB";
    case BREAKPOINT_NUMERIC_TARGET_AD:
        return "AD";
    case BREAKPOINT_NUMERIC_TARGET_DB:
        return "DB";
    case BREAKPOINT_NUMERIC_TARGET_IR:
        return "IR";
    }

    return "?";
}

const char * nesx_breakpoint_numeric_comp_string(int numericComp)
{
    switch (numericComp)
    {
    case BREAKPOINT_COMP_EQUAL:
        return "==";
    case BREAKPOINT_COMP_NOT_EQUAL:
        return "!=";
    case BREAKPOINT_COMP_GREATER_OR_EQUAL:
        return ">=";
    case BREAKPOINT_COMP_LESS_OR_EQUAL:
        return "<=";
    case BREAKPOINT_COMP_GREATER:
        return ">";
    case BREAKPOINT_COMP_LESS:
        return "<";
    }

    return "?";
}

const char * nesx_breakpoint_flag_target_string(int flagTarget)
{
    switch (flagTarget)
    {
    case BREAKPOINT_FLAG_TARGET_IRQ:
        return "IRQ";
    case BREAKPOINT_FLAG_TARGET_NMI:
        return "NMI";
    case BREAKPOINT_FLAG_TARGET_VBLANK:
        return "VBlank";
    case BREAKPOINT_FLAG_TARGET_FN:
        return "FN";
    case BREAKPOINT_FLAG_TARGET_FV:
        return "FV";
    case BREAKPOINT_FLAG_TARGET_FB:
        return "FB";
    case BREAKPOINT_FLAG_TARGET_FD:
        return "FD";
    case BREAKPOINT_FLAG_TARGET_FI:
        return "FI";
    case BREAKPOINT_FLAG_TARGET_FZ:
        return "FZ";
    case BREAKPOINT_FLAG_TARGET_FC:
        return "FC";
    case BREAKPOINT_FLAG_TARGET_RW:
        return "RW";
    case BREAKPOINT_FLAG_TARGET_SYNC:
        return "SYNC";
    case BREAKPOINT_FLAG_TARGET_RDY:
        return "RDY";
    case BREAKPOINT_FLAG_TARGET_RES:
        return "RES";
    }

    return "?";
}

const char * nesx_breakpoint_flag_trigger_string(int flagTrigger)
{
    switch (flagTrigger)
    {
    case BREAKPOINT_STATE_CHANGED:
        return "Changed";
    case BREAKPOINT_STATE_ON:
        return "On";
    case BREAKPOINT_STATE_OFF:
        return "Off";
    }

    return "?";
}

NESxBreakpoint * nesx_breakpoint_list_add(NESxBreakpoint ** first)
{
    NESxBreakpoint ** ptmp = first;
    while (*ptmp) {
        ptmp = &(*ptmp)->next;
    }

    *ptmp = (NESxBreakpoint *)malloc(sizeof(NESxBreakpoint));
    (*ptmp)->next = NULL;
    return *ptmp;
}

void nesx_breakpoint_list_clear(NESxBreakpoint ** first)
{
    NESxBreakpoint * tmp;
    while (*first) {
        tmp = (*first)->next;
        free(*first);
        *first = tmp;
    }

    *first = NULL;
}

void nesx_breakpoint_list_remove(NESxBreakpoint ** first, int index)
{
    NESxBreakpoint * tmp;
    NESxBreakpoint ** ptmp = first;
    while (index > 0) {
        --index;
        if (!*ptmp) {
            return;
        }
        ptmp = &(*ptmp)->next;
    }

    tmp = *ptmp;
    if (tmp) {
        *ptmp = tmp->next;
        free(tmp);
    }
}

bool nesx_breakpoint_list_check(NESxBreakpoint * first, nesx_t * nes)
{
    NESxBreakpoint * tmp = first;
    while (tmp) {
        if (tmp->type == BREAKPOINT_TYPE_NUMERIC) {
            int value;
            switch (tmp->numericTarget) {
            case BREAKPOINT_NUMERIC_TARGET_PC:
                value = nes->CPU.PC;
                break;
            case BREAKPOINT_NUMERIC_TARGET_A:
                value = nes->CPU.A;
                break;
            case BREAKPOINT_NUMERIC_TARGET_X:
                value = nes->CPU.X;
                break;
            case BREAKPOINT_NUMERIC_TARGET_Y:
                value = nes->CPU.Y;
                break;
            case BREAKPOINT_NUMERIC_TARGET_S:
                value = nes->CPU.S;
                break;
            case BREAKPOINT_NUMERIC_TARGET_CPU_CYCLE:
                value = nes->CPU.Cycles;
                break;
            case BREAKPOINT_NUMERIC_TARGET_PPU_CYCLE:
                value = nes->PPU.Cycle;
                break;
            case BREAKPOINT_NUMERIC_TARGET_SCANLINE:
                value = nes->PPU.Scanline;
                break;
            case BREAKPOINT_NUMERIC_TARGET_AB:
                value = nes->CPU.AB;
                break;
            case BREAKPOINT_NUMERIC_TARGET_AD:
                value = nes->CPU.AD;
                break;
            case BREAKPOINT_NUMERIC_TARGET_DB:
                value = nes->CPU.DB;
                break;
            case BREAKPOINT_NUMERIC_TARGET_IR:
                value = nes->CPU.IR;
                break;
            default:
                fprintf(stderr, "Invalid numeric breakpoint target: %d\n", tmp->numericTarget);
                return false;
            }

            bool result = false;
            switch (tmp->numericComp)
            {
            case BREAKPOINT_COMP_EQUAL:
                result = (value == tmp->value);
                break;
            case BREAKPOINT_COMP_NOT_EQUAL:
                result = (value != tmp->value);
                break;
            case BREAKPOINT_COMP_GREATER_OR_EQUAL:
                result = (value >= tmp->value);
                break;
            case BREAKPOINT_COMP_LESS_OR_EQUAL:
                result = (value <= tmp->value);
                break;
            case BREAKPOINT_COMP_GREATER:
                result = (value > tmp->value);
                break;
            case BREAKPOINT_COMP_LESS:
                result = (value < tmp->value);
                break;
            default:
                fprintf(stderr, "Invalid numeric comparison: %d\n", tmp->numericComp);
                return false;
            }

            if (result) {
                return true;
            }
        }
        else {
            bool state;
            switch (tmp->flagTarget) {
            case BREAKPOINT_FLAG_TARGET_IRQ:
                state = nes->CPU.IRQ;
                break;
            case BREAKPOINT_FLAG_TARGET_NMI:
                state = nes->CPU.NMI;
                break;
            case BREAKPOINT_FLAG_TARGET_VBLANK:
                state = nes->PPU.VBlank;
                break;
            case BREAKPOINT_FLAG_TARGET_FN:
                state = nes->CPU.FN;
                break;
            case BREAKPOINT_FLAG_TARGET_FV:
                state = nes->CPU.FV;
                break;
            case BREAKPOINT_FLAG_TARGET_FB:
                state = nes->CPU.FB;
                break;
            case BREAKPOINT_FLAG_TARGET_FD:
                state = nes->CPU.FD;
                break;
            case BREAKPOINT_FLAG_TARGET_FI:
                state = nes->CPU.FI;
                break;
            case BREAKPOINT_FLAG_TARGET_FZ:
                state = nes->CPU.FZ;
                break;
            case BREAKPOINT_FLAG_TARGET_FC:
                state = nes->CPU.FC;
                break;
            case BREAKPOINT_FLAG_TARGET_RW:
                state = nes->CPU.RW;
                break;
            case BREAKPOINT_FLAG_TARGET_SYNC:
                state = nes->CPU.SYNC;
                break;
            case BREAKPOINT_FLAG_TARGET_RDY:
                state = nes->CPU.RDY;
                break;
            case BREAKPOINT_FLAG_TARGET_RES:
                state = nes->CPU.RES;
                break;
            default:
                fprintf(stderr, "Invalid flag target: %d\n", tmp->flagTarget);
                return false;
            }

            bool result = false;
            switch (tmp->flagTrigger)
            {
            case BREAKPOINT_STATE_CHANGED:
                result = (state != tmp->lastState);
                break;
            case BREAKPOINT_STATE_ON:
                result = state;
                break;
            case BREAKPOINT_STATE_OFF:
                result = !state;
                break;
            }

            tmp->lastState = state;

            if (result) {
                return result;
            }
        }
        tmp = tmp->next;
    }

    return false;
}

