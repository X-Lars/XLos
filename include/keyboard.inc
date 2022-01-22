;----------------------------------------------------------------------------------------------------
; KEYBOARD
;----------------------------------------------------------------------------------------------------
; - Waits for a key press
;----------------------------------------------------------------------------------------------------
; <- AL = ASCII Character
;----------------------------------------------------------------------------------------------------
; %include "keyboard/keyboard.inc" ; AL = GetKey()
;----------------------------------------------------------------------------------------------------
[BITS 32]                                       ; Generate 32 bit code

GetKey:

    push EBP                                    ; Store EBP
    mov EBP, ESP                                ; Set base pointer

    push EBX                                    ; Store EBX                                  

    xor EAX, EAX                                ; EAX = 0

    .KeyboardStatus:

        in AL, 0x64		                        ; AL = Port 0x64, Keyboard Status Flags
        bt AX, 1                                ; Bit 1 == 1, Input Buffer Empty
	    jc .KeyboardStatus

    GetScanCode:    

        mov AL, [KEYBOARD_KEY]                  ; AL = Key
        mov [KEYBOARD_KEY_LAST], AL             ; LastKey = AL

        in AL, 0x60                             ; AL = Port 0x60, Keyboard Scancode

        mov [KEYBOARD_KEY], AL                  ; Key = Scancode
        cmp AL, [KEYBOARD_KEY_LAST]             ; Key == LastKey
        je GetScanCode


    cmp AL, 0x2A                                ; AL == Left Shift Pressed
    je KeyboardShiftPressed

    cmp AL, 0x36                                ; AL == Right Shift Pressed
    je KeyboardShiftPressed

    cmp AL, 0xAA                                ; AL == Left Shift Released
    je KeyboardShiftReleased

    cmp AL, 0xB6                                ; AL == Right Shift Released
    je KeyboardShiftReleased

    bt AX, 7                                    ; Bit 7 = 1, Break Code
    jc GetScanCode

    mov EBX, KEYBOARD_SCANCODES                 ; BX = Scancodes
    xlatb                                       ; AL = Lookup AL[DS:BX]

    cmp byte [KEYBOARD_KEY_SHIFT], 1
    jne GetKeyEnd

    cmp AL, '0'
    jl CheckSpecial
    cmp AL, '9'
    jg CheckAlpha

    sub AL, 0x30
    mov EBX, KEYBOARD_SHIFT_SCANCODES
    xlatb
    jmp GetKeyEnd

    CheckAlpha:
        
        cmp AL, 'a'
        jl CheckSpecial
        cmp AL, 'z'
        jg CheckSpecial

        sub AL, 0x20
        jmp GetKeyEnd

    CheckSpecial:

        mov BX, '_'
        cmp AL, '-'
        cmove AX, BX

        mov BX, '+'
        cmp AL, '='
        cmove AX, BX

        mov BX, '}'
        cmp AL, '['
        cmove AX, BX

        mov BX, '}'
        cmp AL, ']'
        cmove AX, BX

        mov BX, '?'
        cmp AL, '/'
        cmove AX, BX

        mov BX, '|'
        cmp AL, '\'
        cmove AX, BX

        mov BX, '<'
        cmp AL, ','
        cmove AX, BX

        mov BX, '>'
        cmp AL, '.'
        cmove AX, BX

        mov BX, '~'
        cmp AL, '`'
        cmove AX, BX

        mov BX, ':'
        cmp AL, ';'
        cmove AX, BX

        mov BX, '"'                               ;";
        cmp AL, "'"
        cmove AX, BX

        jmp GetKeyEnd


GetKeyEnd:

    ; TODO: Return shift & ctrl status

    pop EBX                                     ; Restore EBX
    
    mov ESP, EBP                                ; Deallocate memory
    pop EBP                                     ; Restore EBP

    ret                                         ; Return

KeyboardShiftPressed:

    mov byte [KEYBOARD_KEY_SHIFT], 1
    jmp GetScanCode

KeyboardShiftReleased:

    mov byte [KEYBOARD_KEY_SHIFT], 0
    jmp GetScanCode

;                                                                                             GET KEY
;----------------------------------------------------------------------------------------------------

;----------------------------------------------------------------------------------------------------
; VARIABLES
;----------------------------------------------------------------------------------------------------

    KEYBOARD_KEY: db 0
    KEYBOARD_KEY_LAST: db 0
    KEYBOARD_KEY_SHIFT: db 0
    KEYBOARD_KEY_CTRL: db 0
    KEYBOARD_SHIFT_SCANCODES: db ')', '!', '@', '#', '$', '%', '^', '&', '*', '('
    KEYBOARD_SCANCODES: db 0, 0x1B, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', 0x08,\
                               0x09, 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', 0x0D, 0,\
                                      'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', "'", '`', 0, '\',\
                                       'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0,\
                                    '*', 0, ' ', 0 ; F1, F2, F3, F4, F5, F6, F7, F8, F9, F10, NumLock, ScrollLock
                                    ; 7/Home, 8/Up, 9/PgUp, -, 4/Left, 5, 6/Right, +, 1/End, 2/Down, 3/PgDn, 0/Ins, ./Del
                                    ; Alt-SysRq, ???, (Windows?), F11, F12
    ; TODO: Implement Keys
    ; L-Shift  -> '`'
    ; R-Shift  -> '/'
    ; L-Alt    -> '*'
    ; CapsLock -> ' '

;                                                                                           VARIABLES
;----------------------------------------------------------------------------------------------------
