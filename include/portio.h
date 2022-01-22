#pragma once

void PortOutByte(uint16 port, uint8 data)
{
    asm volatile("outb %0, %1" : : "a"(data), "Nd"(port));
}

uint8 PortInByte(uint16 port)
{
    uint8 result;

    asm volatile("inb %1, %0" : "=a"(result) : "Nd"(port));

    return result;
}

void PortOutWord(uint16 port, uint16 data)
{
    asm volatile("outw %0, %1" : : "a"(data), "Nd"(port));
}

uint16 PortInWord(uint16 port)
{
    uint16 result;

    asm volatile("inw %1, %0" : "=a"(result) : "Nd"(port));

    return result;
}

void PortDelay()
{
    PortOutByte(0x80, 0);
}