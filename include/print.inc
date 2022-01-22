;----------------------------------------------------------------------------------------------------
; PRINT CHARACTER
;----------------------------------------------------------------------------------------------------
; - Prints a character to the screen at the specified offset
; - Updates the hardware cursor
;----------------------------------------------------------------------------------------------------
; -> BYTE Character
; -> WORD *CursorX
; -> WORD *CursorY
;----------------------------------------------------------------------------------------------------
; ASSUMES:
; [0x6000] Bitmap Font 8x16
; [0x9000] VBE Mode Info
;----------------------------------------------------------------------------------------------------

[BITS 32]

PrintCharacter:

    push EBP                                    ; Store EBP
    mov EBP, ESP                                ; Set EBP

    push EAX                                    ; Store EAX
    push EBX                                    ; Store EBX
    push ECX                                    ; Store ECX
    push EDX                                    ; Store EDX
    push EDI                                    ; Store EDI
    push ESI                                    ; Store ESI

    mov EDI, [0x9028]                           ; EDI = [VBE + 40], Frame Buffer
    xor EAX, EAX                                ; Reset EAX
    mov AX, [0x9010]                            ; EAX = [VBE + 16], Bytes Per Scanline

    mov EBX, [EBP+8]                            ; EBX = P3, [Cursor Y]
    mov BX, [EBX]                               ; BX = *[Cursor Y]
    and EBX, 0x0000FFFF                         ; Clear MSB
    shl EBX, 4                                  ; EBX *= 16

    imul EBX, EAX                               ; EBX *= Bytes Per Scanline (Screen Width * Pixel (4 Bytes) * Font Height (16))
    mov ESI, EBX                                ; ESI = Pixel Offset Y

    mov EBX, [EBP+12]                           ; EBX = P2, [Cursor X]
    mov BX, [EBX]                               ; BX = *[Cursor X]
    and EBX, 0x0000FFFF                         ; Clear MSB
    shl EBX, 5                                  ; EBX *= 32 (Font Width (8) * Pixel (4 Bytes))

    add ESI, EBX                                ; ESI = Pixel Offset Y + Pixel Offset X
    add EDI, ESI                                ; EDI = Frame Buffer + Pixel Offset

    xor EAX, EAX                                ; Reset EAX
    mov AL, [EBP+16]                            ; AL = P1, [Character]

    shl EAX, 4                                  ; EAX *= 16, Bitmap Font Offset                    
    sub EAX, 16                                 ; EAX -= 16, Bitmap Font Character Start
    add EAX, 0x6000                             ; EAX += Bitmap Font Address

    mov ECX, 16                                 ; ECX = 16, Pixels Y
    push ECX                                    ; Store ECX
    
    .BitmapLoop:

        mov ECX, 8                                  ; ECX = 8, Pixels X
        mov BL, [EAX]                               ; BL = Character Bit Row
        inc EAX                                     ; EAX += 1, Next Bit Row

        .BitLoop:

            mov EDX, ECX                                ; EDX = Bit Row #
            dec EDX                                     ; EDX -= 1

            bt BX, DX                                   ; Bittest BX[DX] == 1
            jc .DrawPixel                               ; Draw Pixel

            mov [EDI], dword 0x00CC1144                 ; Draw Background Color
            jmp .NextBit                                ; Next Bit

            .DrawPixel:

                mov [EDI], dword 0x00EEEEEE                 ; Draw Foreground Color

            .NextBit:

                add EDI, 4                                  ; Framebuffer += 4 (32 Bit Pixels)
                dec ECX                                     ; Pixels X -= 1
                jnz .BitLoop                                ; ECX != 0 ? Loop : Continue

        pop ECX                                     ; Restore ECX
        dec ECX                                     ; ECX -= 1
        jz .UpdateCursor                            ; ECX == 0 ? Update Cursor : Continue

        xor EDX, EDX                                ; Reset EDX
        mov DX, [0x9010]                            ; EDX = [VBE + 16], Bytes Per Scanline
        add EDI, EDX                                ; EDI = Next Scanline
        sub EDI, 32                                 ; EDI -= Font Width * 4 Byte Per Pixel
        push ECX                                    ; Store ECX
        jmp .BitmapLoop                             ; Next Bit Row


    .UpdateCursor:

        xor EDX, EDX                                ; Reset EDX
        mov DX, [0x9010]                            ; DX = [VBE + 16], Bytes Per Scanline
        imul EDX, 15                                ; EDX *= 15
        sub EDI, EDX                                ; EDI -= 15 Scanlines

        mov EBX, [EBP+12]                           ; EBX = P2, Cursor X
        inc word [EBX]                              ; Cursor X += 1

EndPrintCharacter:

    pop ESI                                     ; Restore ESI
    pop EDI                                     ; Restore EDI
    pop EDX                                     ; Restore EDX
    pop ECX                                     ; Restore ECX
    pop EBX                                     ; Restore EBX
    pop EAX                                     ; Restore EAX

    mov ESP, EBP                                ; Deallocate Memory
    pop EBP                                     ; Restore EBP

    ret 12                                      ; Return, ESP += 3 * DWORD

;                                                                                     PRINT CHARACTER
;----------------------------------------------------------------------------------------------------
