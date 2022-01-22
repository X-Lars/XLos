// (P)rogrammable (I)nterrupt (C)ontroller
// PIC01 Primary CPU
// PIC02 Secondary Motherboard
// 8 IRQ's
#pragma once

#define PIC01_COMMAND 0x20          // PIC01 (W) Command
#define PIC01_STATUS PIC01_COMMAND  // PIC01 (R) Status
#define PIC01_DATA 0x21             // PIC01 (W) Data
#define PIC01_MASK PIC01_DATA       // PIC01 (R) Mask
#define PIC02_COMMAND 0xA0          // PIC02 (W) Command
#define PIC02_STATUS PIC02_COMMAND  // PIC02 (R) Status
#define PIC02_DATA 0xA1             // PIC02 (W) Data
#define PIC02_MASK PIC02_DATA       // PIC02 (W) Mask

#define PIC01_ADDRESS 0x20          // [0x20..0x27] IRQ 32-39
#define PIC02_ADDRESS 0x28          // [0x28..0x2F] IRQ 40-47

#define PIC_EOI 0x20

void DisablePIC(void)
{
    PortOutByte(PIC02_DATA, 0xFF);
    PortOutByte(PIC01_DATA, 0xFF);
}
// Send irq handled
void SendEOI(uint8 irq)
{
    if(irq >= 8) PortOutByte(PIC02_COMMAND, PIC_EOI);

    PortOutByte(PIC01_COMMAND, PIC_EOI);
}

// ignore irq
void SetIRQMask(uint8 irq)
{
    uint16 port;
    uint8 data;

    if(irq < 8)
    {
         port = PIC01_MASK;
    }
    else
    {
        irq -= 8;
        port = PIC02_MASK;
    }

    data = PortInByte(port) | (1 << irq);
    PortOutByte(port, data);

}

void ClearIRQMask(uint8 irq)
{
    uint16 port;
    uint8 data;

    if(irq < 8)
    {
         port = PIC01_MASK;
    }
    else
    {
        irq -= 8;
        port = PIC02_MASK;
    }

    data = PortInByte(port) & ~(1 << irq);
    PortOutByte(port, data);

}

// Initializes the PIC's by remapping the PIC's IRQ numbers.
void InitializePIC(void)
{
    uint8 picMasterMask, picSlaveMask;

    picMasterMask = PortInByte(PIC01_DATA);
    picSlaveMask = PortInByte(PIC02_DATA);

    
    // (I)nitialization (C)ontrol (W)ord
    // ICW01: Set initialization mode
    PortOutByte(PIC01_COMMAND, 0x11);       // 00010001: Initialize PIC
    // PortDelay();
    PortOutByte(PIC02_COMMAND, 0x11);       // 00010001: Initialize PIC
    // PortDelay();
    // ICW02: Set IRQ base address
    // (I)nterrupt (V)ector (T)able reserves interrupts 0..31 [0x00..0x1F]
    PortOutByte(PIC01_DATA, PIC01_ADDRESS); // [0x20..0x27] IRQ0 -> 0x20
    // PortDelay();
    PortOutByte(PIC02_DATA, PIC02_ADDRESS); // [0x28..0x2F] IRQ8 -> 0x28
    // PortDelay();
    // ICW03: Acknowledge cascaded PIC system, set PIC connection
    // 80x86 Architecture user IRQ line 2 to connect the primary with the secondary PIC
    PortOutByte(PIC01_DATA, 0x04);         // 00000100: Bit 2 = IR Line 2
    // PortDelay();
    PortOutByte(PIC02_DATA, 0x02);         // 00000010: Binary 2 = IR Line 2
    // PortDelay();
    // ICW04: Setup operation mode
    PortOutByte(PIC01_DATA, 0x01);         // 00000001: Bit 1 = 80x86 Mode
    // PortDelay();
    PortOutByte(PIC02_DATA, 0x01);         // 00000001: Bit 1 = 80x86 Mode
    // PortDelay();

    // Clear data registers
    PortOutByte(PIC01_DATA, 0x00);
    // PortDelay();
    PortOutByte(PIC02_DATA, 0x00);
    // PortDelay();

    PortOutByte(PIC01_DATA, picMasterMask);
    // PortDelay();
    PortOutByte(PIC02_DATA, picSlaveMask);
    // PortDelay();

}