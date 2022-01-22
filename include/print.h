#pragma once
#include "print.h"

static void DrawBitmapCharacter(uint32*, uint8);

#define FONT_ADDRESS 0x6000
#define FONT_WIDTH 8
#define FONT_HEIGHT 16
#define TEXT_COLOR 0x00EEEEEE
#define BACKGROUND 0x001144CC

void PrintCharacter(uint8 character, uint16* cursorX, uint16* cursorY)
{
    uint32* frameBuffer = (uint32*)Screen->FrameBuffer;

    uint32 x = (*cursorX * FONT_WIDTH);
    uint32 y = (*cursorY * FONT_HEIGHT * Screen->ResolutionX);

    uint8* font;

    frameBuffer += (x + y);

    DrawBitmapCharacter(frameBuffer, character);

    (*cursorX)++;

    return;
}



void PrintString(uint8* string, uint16* cursorX, uint16* cursorY)
{
    uint32* frameBuffer = (uint32*)Screen->FrameBuffer;

    uint32 x = (*cursorX * FONT_WIDTH);
    uint32 y = (*cursorY * FONT_HEIGHT * Screen->ResolutionX);

    uint8* font;
    uint8* input = string;

    frameBuffer += (x + y);

    while (*input != '\0')
    {
        if(*input == 0x0A)
        {
            (*cursorY)++;

            frameBuffer += (Screen->ResolutionX * FONT_HEIGHT);
            input++;
            continue;
        }
        else if (*input == 0x0D)
        {
            frameBuffer -= (*cursorX * FONT_WIDTH);
            *cursorX = 0;
            input++;
            continue;
        }

        DrawBitmapCharacter(frameBuffer, *input);

        (*cursorX)++;
        frameBuffer += FONT_WIDTH;
        input++;
    }

    return;
}
void PrintHexadecimal(uint32 hex, uint16* cursorX, uint16* cursorY)
{
    uint8 hexString[80];
    uint8 *asciiNumbers = "0123456789ABCDEF";
    uint8 nibble;
    uint8 i = 0, j, t;
    uint8 padding = 0;

    if(hex == 0)
    {
        PrintString("0x00", cursorX, cursorY);
        return;
    }

    if(hex < 0x10) padding = 1;

    while (hex > 0)
    {
        nibble = (uint8)hex & 0x0F;
        nibble = asciiNumbers[nibble];
        hexString[i] = nibble;
        hex >>= 4;
        i++;
    }

    if(padding) hexString[i++] = '0';

    hexString[i++] = 'x';
    hexString[i++] = '0';
    hexString[i] = '\0';

    i--;

    for(j = 0; j < i; j++, i--)
    {
        t = hexString[j];
        hexString[j] = hexString[i];
        hexString[i] = t;
    }

    PrintString(hexString, cursorX, cursorY);
}
void PrintDecimal(int32 number, uint16* cursorX, uint16* cursorY)
{
    uint8 decString[80];
    uint8 i = 0, j = 0, t = 0;
    bool negative = false;

    if(number == 0) decString[i++] = '0';
    else if(number < 0)
    {
        negative = true;
        number = -number; // Remove negative sign
    }

    while (number > 0)
    {
        decString[i] = (number % 10) + '0';
        number /= 10;
        i++;
    }

    if(negative) decString[i++] = '-';

    decString[i] = '\0';
    i--; // Skip NULL 
    for(j = 0; j < i; j++, i--)
    {
        t = decString[j];
        decString[j] = decString[i];
        decString[i] = t;
    }

    PrintString(decString, cursorX, cursorY);
    
}

static void DrawBitmapCharacter(uint32* frameBuffer, uint8 character)
{
    uint8* font = (uint8*)(FONT_ADDRESS + ((character * FONT_HEIGHT) - FONT_HEIGHT));

    for(uint8 bitY = 0; bitY < FONT_HEIGHT; bitY++)
    {
        for(int8 bitX = FONT_WIDTH - 1; bitX >= 0; bitX--)
        {
            *frameBuffer = (font[bitY] & (1 << bitX)) ? TEXT_COLOR : BACKGROUND;
            frameBuffer++;
        }

        frameBuffer += (Screen->ResolutionX - FONT_WIDTH);
    }

    frameBuffer -= (Screen->ResolutionX * FONT_HEIGHT);

    return;
}