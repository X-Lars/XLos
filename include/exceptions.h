#pragma once

#include "../include/graphics.h"
#include "../include/print.h"

__attribute__ ((interrupt)) void DivideZeroHandler(INTFrame *frame)
{
    uint16 x = 0;
    uint16 y = 0;

    PrintString("Divide by zero exception", &x, &y);

    frame->EIP++;
}