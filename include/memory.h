#pragma once

#define MEM_BLOCK_SIZE 4096
#define MEM_BLOCK_BYTE 8

static uint32 *MEM_MAP = 0;
static uint32 MEM_MAX_BLOCKS = 0;
static uint32 MEM_USED_BLOCKS = 0;

void ReserveBlock(uint32 bit)
{
    MEM_MAP[bit/32] |= (1 << (bit % 32));
}

void ReleaseBlock(uint32 bit)
{
    MEM_MAP[bit/32] &= ~(1 << (bit % 32));
}

uint8 ValidateBlock(uint32 bit)
{
    return MEM_MAP[bit/32] & (1 << (bit % 32));
}

int32 GetFreeBlocks(uint32 numBlocks)
{
    if(numBlocks == 0) return -1;

    for (uint32 i = 0; i < MEM_MAX_BLOCKS / 32; i++)
    {
        if(MEM_MAP[i] != 0xFFFFFFFF)
        {
            for(int32 j = 0; j < 32; j++)
            {
                int32 bit = 1 << j;

                if(!(MEM_MAP[i] & bit))
                {
                    int32 startBit = i*32 + bit;
                    uint32 freeBlocks = 0;

                    for(uint32 count = 0; count < numBlocks; count++)
                    {
                        if(!ValidateBlock(startBit + count)) freeBlocks++;

                        if(freeBlocks == numBlocks)
                        {
                            return i*32 + j;
                        }
                    }
                }
            }
        }
    }
    
    return -1;
}

void* MemoryWrite(void* buffer, const uint8 byte, const uint32 size)
{
    uint8* bufferPtr = (uint8*)buffer;

    for(uint32 i = 0; i < size; i++)
    {
        bufferPtr[i] = byte;
    }

    return buffer;
}

void InitializeMemoryManager(uint32 address, uint32 size)
{
    MEM_MAP = (uint32*)address;
    MEM_MAX_BLOCKS = size / MEM_BLOCK_SIZE;
    MEM_USED_BLOCKS = MEM_MAX_BLOCKS;

    MemoryWrite(MEM_MAP, 0xFF, MEM_MAX_BLOCKS / MEM_BLOCK_BYTE);
   
}

void InitializeMemoryRegion(uint32 address, uint32 size)
{
    int32 align = address / MEM_BLOCK_SIZE;
    int32 blocks = size / MEM_BLOCK_SIZE;

    for(;blocks > 0; blocks--)
    {
        ReleaseBlock(align++);
        MEM_USED_BLOCKS--;
    }

    // Prevent overwrite
    ReserveBlock(0);
}

void DeinitializeMemoryRegion(uint32 address, uint32 size)
{
    int32 align = address / MEM_BLOCK_SIZE;
    int32 blocks = size / MEM_BLOCK_SIZE;

    for(;blocks > 0; blocks--)
    {
        ReserveBlock(align++);
        MEM_USED_BLOCKS++;
    }
}

uint32* AllocateBlocks(uint32 count)
{
    // No memory to allocate
    if((MEM_MAX_BLOCKS - MEM_USED_BLOCKS) <= count) return 0;

    int32 startBlock = GetFreeBlocks(count);

    // Not enough memory
    if(startBlock == -1) return 0;

    // Set blocks reserved
    for(uint32 i = 0; i < count; i++)
    {
        ReserveBlock(startBlock + i);
    }

    MEM_USED_BLOCKS += count;

    uint32 address = startBlock * MEM_BLOCK_SIZE;

    return (uint32*)address;
}

void DeallocateBlocks(uint32* address, uint32 count)
{
    int32 startBlock = (uint32)address / MEM_BLOCK_SIZE;

    for(uint32 i = 0; i < count; i++)
    {
        ReleaseBlock(startBlock + i);
    }

    MEM_USED_BLOCKS -= count;
}

typedef struct MEMORY_ENTRY
{
    uint64 Base;
    uint64 Limit;
    uint32 Type;
    uint32 ACPI;
} __attribute__ ((packed)) MEMORY_ENTRY;

void PrintMemory(uint16* cursorX, uint16* cursorY)
{
    // Number of entries
    uint32 MemoryMap = *(uint32*)0x2000;
    // Start of entries
    MEMORY_ENTRY *MemoryEntry = (MEMORY_ENTRY*)0x2004;

    for(uint32 i = 0; i < MemoryMap; i++)
    {
        PrintString("Region: \0", cursorX, cursorY);
        PrintHexadecimal(i, cursorX, cursorY);
        PrintString(" Base: \0", cursorX, cursorY);
        PrintHexadecimal(MemoryEntry->Base, cursorX, cursorY);
        PrintString(" Length: \0", cursorX, cursorY);
        PrintHexadecimal(MemoryEntry->Limit, cursorX, cursorY);
        PrintString(" Type: \0", cursorX, cursorY);
        PrintHexadecimal(MemoryEntry->Type, cursorX, cursorY);
        
        switch ((MemoryEntry->Type))
        {
            case 1: // Available
                PrintString("Available\0", cursorX, cursorY);
            break;
            case 2: // Reserved
                PrintString("Reserved\0", cursorX, cursorY);
            break;
            case 3: // ACPI Reclaim
                PrintString("ACPI Reclaim\0", cursorX, cursorY);
            break;
            case 4: // ACPI NVS Memory
                PrintString("ACPI NVS\0", cursorX, cursorY);
            break;

        
            default: // Reserved
                PrintString("Reserved\0", cursorX, cursorY);
                break;
        }

        PrintString("\x0A\x0D\0", cursorX, cursorY);
        MemoryEntry++;
    }

    PrintString("\x0A\x0D\0", cursorX, cursorY);
    PrintString("Total Memory: \0", cursorX, cursorY);
    PrintHexadecimal(MemoryEntry->Base + MemoryEntry->Limit - 1 ,cursorX, cursorY);
    PrintString("\x0A\x0D\0", cursorX, cursorY);

    // Total 4K 
    PrintString("\x0A\x0DTotal Blocks 4K: \0", cursorX, cursorY);
    PrintHexadecimal(MEM_MAX_BLOCKS ,cursorX, cursorY);
    PrintString("\x0A\x0DUsed Blocks : \0", cursorX, cursorY);
    PrintHexadecimal(MEM_USED_BLOCKS ,cursorX, cursorY);
    PrintString("\x0A\x0D Available Blocks : \0", cursorX, cursorY);
    PrintHexadecimal(MEM_MAX_BLOCKS - MEM_USED_BLOCKS ,cursorX, cursorY);
}
