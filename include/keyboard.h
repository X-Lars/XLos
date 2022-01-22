#pragma once

#include "types.h"

typedef struct 
{
    uint8 Key;
    uint8 ScanCode;
    bool Shift;
    bool Control;
} __attribute__ ((packed)) KEY_INFO;

KEY_INFO *KeyInfo = (KEY_INFO*)0x1600;


uint8 GetKey()
{
    uint8 result;

    while (!KeyInfo->Key)
    {
        asm volatile("hlt");
    }

    result = KeyInfo->Key;
    KeyInfo->Key = 0;

    return result;
 
}

// KEYBOARD_SCANCODES: db 0, 0x1B, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', 0x08,
//                            0x09, 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', 0x0D, 0,
//                                   'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', "'", '`', 0, '\',
//                                    'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0,
//                                     '*', 0, ' ', 0 ; F1, F2, F3, F4, F5, F6, F7, F8, F9, F10, NumLock, ScrollLock
//                                     ; 7/Home, 8/Up, 9/PgUp, -, 4/Left, 5, 6/Right, +, 1/End, 2/Down, 3/PgDn, 0/Ins, ./Del
//                                     ; Alt-SysRq, ???, (Windows?), F11, F12
//     ; TODO: Implement Keys
//     ; L-Shift  -> '`'
//     ; R-Shift  -> '/'
//     ; L-Alt    -> '*'
//     ; CapsLock -> ' '