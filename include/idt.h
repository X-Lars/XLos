#pragma once

#include "pic.h"
#include "graphics.h"
#include "print.h"
#include "keyboard.h"

#define INT_TRAP_GATE 0x8F
#define INT_GATE 0x8E
#define INT_GATE_USER 0xEE

typedef struct
{
    uint16 RoutineLow;
    uint16 Selector;
    uint8 Reserved;
    uint8 Attributes;
    uint16 RoutineHigh;
} __attribute__ ((packed)) IDTEntry;

typedef struct 
{
    uint16 Limit;
    uint32 Base;
} __attribute__ ((packed)) IDTRegister;

IDTEntry IDT[256];
IDTRegister IDTR;

typedef struct 
{
    uint32 EIP;
    uint32 CS;
    uint32 Flags;
    uint32 SP;
    uint32 SS;
} __attribute__ ((packed)) INTFrame;

__attribute__ ((interrupt)) void DefaultExceptionHandler(INTFrame *frame)
{
    uint16 x = 0;
    uint16 y = 0;

    PrintString("Default Exception Handler", &x, &y);
}

__attribute__ ((interrupt)) void DefaultExceptionHandlerCode(INTFrame *frame, uint32 errorCode)
{
    uint16 x = 0;
    uint16 y = 0;

    PrintString((uint8*)"Default Exception Handler: Error Code", &x, &y);
}

__attribute__ ((interrupt)) void DefaultInterruptHandler(INTFrame *frame)
{
    uint16 x = 10;
    uint16 y = 5;

    PrintString("Default Interrupt Handler", &x, &y);
}


void SetIDTDescriptor(uint8 interrupt, void *routine, uint8 flags)
{
    IDTEntry *entry = &IDT[interrupt];

    entry->RoutineLow = (uint32)routine & 0xFFFF;
    entry->Selector = 0x08;
    entry->Reserved = 0x00;
    entry->Attributes = flags;
    entry->RoutineHigh = ((uint32)routine >> 16) & 0xFFFF;

}

void InitializeIDT()
{
    IDTR.Limit = (uint16)(sizeof IDT);
    IDTR.Base = (uint32)&IDT;

    for(uint8 entry = 0; entry < 32; entry++)
    {
        if (entry ==  8 || entry == 10 || entry == 11 || entry == 12 ||
            entry == 13 || entry == 14 || entry == 17 || entry == 21)
        {
            SetIDTDescriptor(entry, DefaultExceptionHandlerCode, INT_TRAP_GATE);
        }
        else
        {
            SetIDTDescriptor(entry, DefaultExceptionHandler, INT_TRAP_GATE);
        }
    }

    for(uint16 entry = 32; entry < 256; entry++)
    {
        SetIDTDescriptor(entry, DefaultInterruptHandler, INT_GATE);
    }

    asm volatile ("lidt %0" : : "memory"(IDTR));
}