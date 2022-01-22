#pragma once

#include "idt.h"

__attribute__ ((interrupt)) void IRQ01KeyboardHandler(INTFrame *frame)
{
    // Scancodes Set 1
    const uint8 SCANCODE_ASCII[] = "\x00\x1B" "1234567890-=" "\x08"
                                        "\x09" "qwertyuiop[]" "\x0D\x00"
                                                "asdfghjkl;'`" "\x00" "\\"
                                                 "zxcvbnm,./" "\x00" "*" "\x00" " ";

    // Numerical Shift Scancodes                                                 
    const uint8 KEYBOARD_SHIFT_SCANCODES[] = ")!@#$%^&*(";

    static bool E0 = 0;
    static bool E1 = 0;

    uint8 key;

    KeyInfo->Key = 0;

    key = PortInByte(0x60);

    if(key)
    {
        // 0x2A = Left Shift Pressed,   0xAA = Left Shift Released
        // 0x36 = Right Shift Pressed,  0xB6 = Right Shift Released
        // 0x1D = Left Control Pressed, 0x9D = Left Control Released
        if     (key == 0x2A || key == 0x36) KeyInfo->Shift = true;
        else if(key == 0xAA || key == 0xB6) KeyInfo->Shift = false;
        else if(key == 0x1D) KeyInfo->Control = true;
        else if(key == 0x9D) KeyInfo->Control = false;
        else if(key == 0xE0) E0 = true;
        else
        {
            // Key Released
            if(!(key & 0x80))
            {
                // if(!E0) // Doesn't work on hyper-v
                // {

                    key = SCANCODE_ASCII[key];

                    if(KeyInfo->Shift)
                    {
                        if(key >= 'a' && key <= 'z') key -= 0x20;
                        else if (key >= '0' && key <= '9') key = KEYBOARD_SHIFT_SCANCODES[key - 0x30];
                        else
                        {
                            if      (key == '`') key = '~';
                            else if (key == '-') key = '_';
                            else if (key == '=') key = '+';
                            else if (key == '[') key = '{';
                            else if (key == ']') key = '}';
                            else if (key == ';') key = ':';
                            else if (key == '\'') key = '\"';
                            else if (key == '\\') key = '|';
                            else if (key == ',') key = '<';
                            else if (key == '.') key = '>';
                            else if (key == '/') key = '?';
                        }
                    }
                // }

                KeyInfo->Key = key;
            }
            if(E0) E0 == false;
        }

        
    }

    SendEOI(1);
}


static bool ToggleDateTime = false;

typedef struct 
{
    uint8  Seconds;
    uint8  Minutes;
    uint8  Hours;
    uint8  Day;
    uint8  Month;
    uint16 Year;
} __attribute__ ((packed)) DATETIME;

DATETIME *DateTime = (DATETIME*)0x1800;


#define CMOS_COMMAND 0x70
#define CMOS_DATA 0x71

// Read CMOS update in progress flag
bool CMOSUpdating()
{
    PortOutByte(CMOS_COMMAND, 0x8A); // Read from status register A, disable NMI
    return (PortInByte(CMOS_DATA) & 80); // If bit 7 is set, CMOS is updating
}

// Get RTC register value
uint8 GetRTCRegister(uint8 rtcRegister)
{
    PortOutByte(CMOS_COMMAND, rtcRegister | 0x80); // Disable NMI when 
    return PortInByte(CMOS_DATA); // Return data from register
}

void EnableRTC()
{
    // Set bit 6 of status register b
    uint8 bRegValue = GetRTCRegister(0x0B);

    PortOutByte(CMOS_COMMAND, 0x8B);
    PortOutByte(CMOS_DATA, bRegValue | 0x40); // Set bit 6, enable periodic interrupts (Default Rate: 1024Hz)

    GetRTCRegister(0x0C); // Ensure no left over interrupts to read, otherwise no new interrupts get spawned
}

void DisableRTC()
{
    // Set bit 6 of status register b
    uint8 bRegValue;

    asm volatile ("cli");
    
    bRegValue = GetRTCRegister(0x0B);
    PortOutByte(CMOS_COMMAND, 0x8B);
    PortOutByte(CMOS_DATA, bRegValue & 0xBF); // Clear bit 6, enable periodic interrupts (Default Rate: 1024Hz)

    asm volatile ("sti");

}

__attribute__ ((interrupt)) void IRQ8RealTimeClock(INTFrame *frame)
{
    uint16 x =5, y=5;
    DATETIME newDateTime, oldDateTime;
    uint8 rtcRegisterB;
    static uint16 ticks = 0;

    asm volatile ("cli");

    ticks++;

    if(ticks % 1024 == 0)
    {
        while (CMOSUpdating());

        newDateTime.Seconds = GetRTCRegister(0x00);
        newDateTime.Minutes = GetRTCRegister(0x02);
        newDateTime.Hours   = GetRTCRegister(0x04);
        newDateTime.Day     = GetRTCRegister(0x07);
        newDateTime.Month   = GetRTCRegister(0x08);
        newDateTime.Year    = GetRTCRegister(0x09);

        do
        {
            oldDateTime = newDateTime;

            while (CMOSUpdating());

            newDateTime.Seconds = GetRTCRegister(0x00);
            newDateTime.Minutes = GetRTCRegister(0x02);
            newDateTime.Hours   = GetRTCRegister(0x04);
            newDateTime.Day     = GetRTCRegister(0x07);
            newDateTime.Month   = GetRTCRegister(0x08);
            newDateTime.Year    = GetRTCRegister(0x09);

        } while 
            (
                (newDateTime.Seconds != oldDateTime.Seconds) ||
                (newDateTime.Minutes != oldDateTime.Minutes) ||
                (newDateTime.Hours   != oldDateTime.Hours)   ||
                (newDateTime.Day     != oldDateTime.Day)     ||
                (newDateTime.Month   != oldDateTime.Month)   ||
                (newDateTime.Year    != oldDateTime.Year)
            );
        
        
        rtcRegisterB = GetRTCRegister(0x0B);

        // Conert BCD values to binary if needed (Bit 2 is clear)
        if(!(rtcRegisterB & 0x04))
        {
            newDateTime.Seconds = (newDateTime.Seconds & 0x0F) + ((newDateTime.Seconds / 16) * 10);
            newDateTime.Minutes = (newDateTime.Minutes & 0x0F) + ((newDateTime.Minutes / 16) * 10);
            newDateTime.Hours   = ((newDateTime.Hours  & 0x0F) + (((newDateTime.Hours & 0x70) / 16) * 10)) | (newDateTime.Hours & 0x80);
            newDateTime.Day     = (newDateTime.Day     & 0x0F) + ((newDateTime.Day / 16) * 10);
            newDateTime.Month   = (newDateTime.Month   & 0x0F) + ((newDateTime.Month / 16) * 10);
            newDateTime.Year    = (newDateTime.Year    & 0x0F) + ((newDateTime.Year / 16) * 10);
        }

        // Conert 12 hr to 24 hr (bit 1 is clear)
        if(!(rtcRegisterB & 0x02) && (newDateTime.Hours & 0x80))
        {
            newDateTime.Hours == ((newDateTime.Hours & 0x7F) + 12) % 24;
        }

        // Get year
        newDateTime.Year += 2000;

        // Set date time values to memory location 

        DateTime->Seconds = newDateTime.Seconds;
        DateTime->Minutes = newDateTime.Minutes;
        DateTime->Hours   = newDateTime.Hours;
        DateTime->Day     = newDateTime.Day;
        DateTime->Month   = newDateTime.Month;
        DateTime->Year    = newDateTime.Year;

        if(ToggleDateTime)
        {
            // Screenlocation
            uint16 x = 50, y = 0;

            PrintDecimal(DateTime->Year, &x, &y);
            PrintCharacter('.', &x, &y);
            if(DateTime->Month < 10) PrintCharacter('0', &x, &y);
            PrintDecimal(DateTime->Month, &x, &y);
            PrintCharacter('.', &x, &y);
            if(DateTime->Day < 10) PrintCharacter('0', &x, &y);
            PrintDecimal(DateTime->Day, &x, &y);
            PrintCharacter(' ', &x, &y);
            if(DateTime->Hours < 10) PrintCharacter('0', &x, &y);
            PrintDecimal(DateTime->Hours, &x, &y);
            PrintCharacter(':', &x, &y);
            if(DateTime->Minutes < 10) PrintCharacter('0', &x, &y);
            PrintDecimal(DateTime->Minutes, &x, &y);
            PrintCharacter(':', &x, &y);
            if(DateTime->Seconds < 10) PrintCharacter('0', &x, &y);
            PrintDecimal(DateTime->Seconds, &x, &y);

        }

    }

    // Read or future irq 8 will not occure
    GetRTCRegister(0x0C); 


    // RTC Date Time 0x1610
    SendEOI(8);

    asm volatile ("sti");

}
