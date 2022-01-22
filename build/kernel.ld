/* [SECTIONS]   Defines the  memory layout of the output file. */
/* [.]          Location counter, if no address specified, the address is set from the current value of the location counter. */
/* [*(.____)]   Wildcard, selects all (.____) sections in all input files. */
/* [.text]      Defines the code section. */
/* [.data]      Defines initialized data. */
/* [.bss]       Defines uninitialized data. */
/* [.rodata]    Defines constant data. */

SECTIONS
{
    .text 0x50000:
    {
        *(kernel_entry);
        *(.text);
    }

    .rodata :
    {
        *(.rodata*);
    }
   

    .bss :
    {
        *(.bss*);
    }
}