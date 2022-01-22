#pragma once

#define VBE_MODE_INFO_MEMORY_ADDRESS 0x9000

typedef struct
{
    // Mandatory for all VBE revisions
    uint16 Mode;
    uint8  WinA;
    uint8  WinB;
    uint16 WinGranularity;
    uint16 WinSize;
    uint16 WinASegment;
    uint16 WinBSegment;
    uint32 WinFuncPtr;
    uint16 BytesPerScanline;

    // Mandatory for VBE 1.2 and above
    uint16 ResolutionX;
    uint16 ResolutionY;
    uint8  XCharSize;
    uint8  YCharSize;
    uint8  NumPlanes;
    uint8  BitsPerPixel;
    uint8  NumBanks;
    uint8  MemoryModel;
    uint8  BankSize;
    uint8  NumImagePages;
    uint8  Reserved01;

    uint8  RedMaskSize;
    uint8  RedFieldPos;
    uint8  GreenMaskSize;
    uint8  GreenFieldPos;
    uint8  BlueMaskSize;
    uint8  BlueFieldPos;
    uint8  ReservedMaskSize;
    uint8  ReservedFieldPos;
    uint8  DirectColorModeInfo;

    // Mandatory for VBE 2.0 and above
    uint32 FrameBuffer;
    uint32 Reserved02;
    uint16 Reserved03;

    // Mandatory for VBE 3.0 and above
    uint16 LinearBytesPerScanline;
    uint8  BankNumImagePages;
    uint8  LinearNumImagePages;
    uint8  LinearRedMaskSize;
    uint8  LinearRedFieldPos;
    uint8  LinearGreenMaskSize;
    uint8  LinearGreenFieldPos;
    uint8  LinearBlueMaskSize;
    uint8  LinearBlueFieldPos;
    uint8  LinearReservedMaskSize;
    uint8  LinearReservedFieldPos;
    uint32 MaxPixelClock;

    uint8  Reserved04[190];

} __attribute__ ((packed)) VBEModeInfo;

VBEModeInfo *Screen = (VBEModeInfo*)VBE_MODE_INFO_MEMORY_ADDRESS;


void ClearScreen(void)
{
   uint32* video_memory = (uint32*)Screen->FrameBuffer;

    for(uint32 i = 0; i < Screen->ResolutionX * Screen->ResolutionY; i++)
    {
        video_memory[i] = 0x001144CC;
    }    
    
};