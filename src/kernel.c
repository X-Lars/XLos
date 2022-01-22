#include "../include/types.h"
#include "../include/portio.h"
#include "../include/idt.h"
#include "../include/exceptions.h"
#include "../include/interrupts.h"
#include "../include/pic.h"
#include "../include/sys.h"
#include "../include/graphics.h"
#include "../include/string.h"
#include "../include/print.h"
#include "../include/memory.h"
#include "../include/keyboard.h"

__attribute__ ((section ("kernel_entry"))) void main(void)
{
    uint8* KERNEL_MSG_VERSION = "Kernel Version 0.03 32-Bit\x0A\x0D\0";

    uint8 SHELL_PROMPT[] = "\x0A\x0DXL>\0";

    uint8* SHELL_CMD_CLOCK = "clock\0";         // Show CMOS RTC Date Time

    uint16 _CursorX = 0;
    uint16 _CursorY = 0;

    uint8 _Key = 0;

    uint8 _Command[255];
    uint8 _CommandLength;

    uint32 _MemoryTotal;
    uint32 _MemoryEntries;
    MEMORY_ENTRY* _MemoryEntry;

    ClearScreen();

    PrintString(KERNEL_MSG_VERSION, &_CursorX, &_CursorY);


    _MemoryEntries = *(uint32*)0x2000;
    _MemoryEntry = (MEMORY_ENTRY*)0x2004;
    _MemoryEntry += _MemoryEntries - 1;
    _MemoryTotal = _MemoryEntry->Base + _MemoryEntry->Limit - 1;
    
    // Initialize all available memory
    InitializeMemoryManager(0x30000, _MemoryTotal);

    _MemoryEntry = (MEMORY_ENTRY*)0x2004;

    // Initialize available memory regions
    for(uint32 i = 0; i < _MemoryEntries; i++, _MemoryEntry++)
    {
        if(_MemoryEntry->Type == 1)
        {
            InitializeMemoryRegion(_MemoryEntry->Base, _MemoryEntry->Limit);
        }
    }

    // Protect used memory regions for kernel and OS
    DeinitializeMemoryRegion(0x1000, 0x9000); // Reserve memory below 0xA000
    DeinitializeMemoryRegion(0x30000, MEM_MAX_BLOCKS / MEM_BLOCK_BYTE); // Reserve memory manger
    PrintString("Memory Manager Initialized", &_CursorX, &_CursorY);
    PrintMemory(&_CursorX, &_CursorY);

    InitializeIDT();

    SetIDTDescriptor(0, DivideZeroHandler, INT_TRAP_GATE);
    SetIDTDescriptor(0x80, SysDispatcher, INT_GATE_USER);

    DisablePIC();
    InitializePIC();

    SetIDTDescriptor(0x21, IRQ01KeyboardHandler, INT_GATE);
    SetIDTDescriptor(0x28, IRQ8RealTimeClock, INT_GATE);

    // Clear keyboard buffer
    while (PortInByte(0x64) & 1) PortInByte(0x60);

    // PIC1
    ClearIRQMask(1);
    ClearIRQMask(2); // Enable PIC2 line

    // PIC2
    ClearIRQMask(8);

    // Enable RTC
    EnableRTC();

    asm volatile ("sti");

    // System Call
    //asm volatile("movl $0, %eax; int $0x80");
    ToggleDateTime = true;

    while (1)
    {
        PrintString(SHELL_PROMPT, &_CursorX, &_CursorY);
        
        PrintDecimal(StringCompare(SHELL_CMD_CLOCK, KERNEL_MSG_VERSION), &_CursorX, &_CursorY);
        
        while (1)
        {
            _Key = GetKey();

            if(_Key == 0x1B)
            {
                // Exit Qemu
                PortOutWord(0x0604, 0x2000);
            }

            if(_Key == 0x0D) break;

            if(_Key == 0x08)
            {
                if(_CommandLength == 0) continue;

                _CommandLength--;
                _Command[_CommandLength] = '\0';

                _CursorX--;
                PrintCharacter(0x20, &_CursorX, &_CursorY);
                _CursorX--;

                continue;
            }

            _Command[_CommandLength] = _Key;
            _CommandLength++;

            PrintCharacter(_Key, &_CursorX, &_CursorY);
        }

        if(_CommandLength == 0)
        {
            continue;
        }

        _Command[_CommandLength] = '\0';
        
    }
    
}