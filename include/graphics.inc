
;----------------------------------------------------------------------------------------------------
; CLEAR SCREEN
;----------------------------------------------------------------------------------------------------
; -> [0x9000] VBE Mode Info
;----------------------------------------------------------------------------------------------------
; - Clears the screen
;----------------------------------------------------------------------------------------------------

VBE_MODE_INFO equ 0x9000                        ; VBE Mode Info

[BITS 32]

ClearScreen:

    push EAX                                    ; Store EAX
    push ECX                                    ; Store ECX
    push EDI                                    ; Store EDI

    xor EAX, EAX                                ; Reset EAX
    xor ECX, ECX                                ; Reset ECX

    mov AX, [VBE_MODE_INFO+18]                  ; EAX = [VBE + 18], X Resolution
    mov CX, [VBE_MODE_INFO+20]                  ; ECX = [VBE + 20], Y Resolution

    imul ECX, EAX                               ; ECX = # Pixels
    mov EDI, [VBE_MODE_INFO+40]                 ; EDI = [VBE + 40], Frame Buffer
    mov EAX, 0x00CC1144                         ; EAX = Pixel Color ARGB
    rep stosd                                   ; EDI[ECX] = EAX | ECX--

    pop EDI                                     ; Restore EDI
    pop ECX                                     ; Restore ECX
    pop EAX                                     ; Restore EAX

    ret                                         ; Return
;                                                                                        CLEAR SCREEN
;----------------------------------------------------------------------------------------------------