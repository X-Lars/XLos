;----------------------------------------------------------------------------------------------------
; KERNEL
;----------------------------------------------------------------------------------------------------
; -> [0x1000]  File Table
; -> [0x1500]  BIOS Boot Drive ID
; -> [0x2000]  Memory Map
; -> [0x6000]  Bitmap Font
; -> [0x9000]  VBE Mode Info
; -> [0x50000] Kernel
; -> [0x90000] Stack
;----------------------------------------------------------------------------------------------------
; - 
;----------------------------------------------------------------------------------------------------

[ORG 0x50000]
[BITS 32]

Kernel:

    call ClearScreen

    

    xor EAX, EAX
    cpuid

    .NextBX:
    push dword EBX
    push dword SHELL_CURSOR_X
    push dword SHELL_CURSOR_Y
    call PrintCharacter
    shr EBX, 8
    cmp EBX, 0
    jne .NextBX

    mov EBX, EDX
    .NextDX:
    push dword EBX
    push dword SHELL_CURSOR_X
    push dword SHELL_CURSOR_Y
    call PrintCharacter
    shr EBX, 8
    cmp EBX, 0
    jne .NextDX

    mov EBX, ECX
    .NextCX:
    push dword EBX
    push dword SHELL_CURSOR_X
    push dword SHELL_CURSOR_Y
    call PrintCharacter
    shr EBX, 8
    cmp EBX, 0
    jne .NextCX

Shell:

ShellInput:

    call GetKey

    push dword EAX
    push dword SHELL_CURSOR_X
    push dword SHELL_CURSOR_Y
    call PrintCharacter

    jmp ShellInput

KernelEnd:
    
    cli                                         ; Clear Interrupts
    hlt                                         ; Halt CPU
;                                                                                              KERNEL
;----------------------------------------------------------------------------------------------------

%include "graphics.inc"
%include "keyboard.inc"
%include "print.inc"

SHELL_CURSOR_X: dw 0x0000
SHELL_CURSOR_Y: dw 0x0000

;----------------------------------------------------------------------------------------------------
; SECTOR
;----------------------------------------------------------------------------------------------------

    times 1024-($-$$) db 0x00
;                                                                                              SECTOR
;----------------------------------------------------------------------------------------------------