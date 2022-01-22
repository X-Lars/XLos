;----------------------------------------------------------------------------------------------------
; BOOT LOADER                                                                        [0x7E00][0x81FF]
;----------------------------------------------------------------------------------------------------
; -> [0x1000]  File Table
; -> [0x1500]  BIOS Boot Drive ID
; -> [0x6000]  Font
; -> [0x50000] Kernel
;----------------------------------------------------------------------------------------------------
; <- [0x1000]  File Table
; <- [0x1500]  BIOS Boot Drive ID
; <- [0x2000]  Memory Map                                                            [0x2000][0x3FFF]
; <- [0x6000]  Font
; <- [0x9000]  VBE Mode Info
; <- [0x50000] Kernel
; <- [0x90000] Stack
;----------------------------------------------------------------------------------------------------
; - Initializes The Memory Map
; - Initializes (V)esa (B)IOS (E)xtensions
; - Initializes (G)lobal (D)escriptor (T)able
; - Enters 32 bit protected mode
; - Initializes the kernel & stack [0x90000]
; - Initializes the video memory [0x9000]
; - Executes the kernel
;----------------------------------------------------------------------------------------------------

[ORG 0x7E00]
[BITS 16]

BootLoader:

;----------------------------------------------------------------------------------------------------
;                                                                                     INIT MEMORY MAP

InitializeMemoryMap:

    xor EAX, EAX                                ; Reset EAX
    mov ES, AX                                  ; INT 0x15, ES = Buffer Segment

    mov EAX, 0xE820                             ; INT 0x15, EAX = 0xE820, Get System Memory Map
    xor EBX, EBX                                ; INT 0x15, EBX = 0x0000, Continuation
    mov ECX, 24                                 ; INT 0x15, ECX = 0x0018, Buffer Size
    mov EDX, "PAMS"                             ; INT 0x15, EDX = "SMAP", Little Endian
    mov DI, 0x2004                              ; INT 0x15, DI  = 0x2004, Buffer Segment Offset
    mov [ES:DI+20], DWORD 1                     ; Force Valid ACPI Entry
    xor BP, BP                                  ; BP = Memory Entry Counter

    int 0x15                                    ; Get System Memory Map
    jc .MemoryError                             ; ERROR, Unsupported Function

    cmp EAX, "PAMS"                             ; EAX != "SMAP"
    jne .MemoryError                            ; ERROR, Failed

    test EBX, EBX                               ; EBX == 0
    jz .MemoryError                             ; ERROR, No Entries
    jmp .ValidateEntry

    .NextEntry:

        mov EAX, 0xE820                             ; INT 0x15, EAX = 0xE820, Get System Memory Map
        mov ECX, 24                                 ; INT 0x15, ECX = 0x0018, Buffer Size
        mov EDX, "PAMS"                             ; INT 0x15, EDX = "SMAP", Little Endian
        int 0x15                                    ; Get System Memory Map

    .ValidateEntry:

        jcxz .SkipEntry                             ; ECX == 0 ? Skip : Continue

        mov ECX, [ES:DI+8]                          ; ECX  = Memory Region Length, Lower 32 Bits
        or ECX, [ES:DI+12]                          ; ECX |= Memory Region Length, Upper 32 Bits
        
        jz .SkipEntry                               ; Skip Entry

    .ValidEntry:

        inc BP                                      ; BP += 1, Memory Entry Counter
        add DI, 24                                  ; DI += Memory Map Entry Size

    .SkipEntry:

        test EBX, EBX                               ; EBX == 0
        jz .Complete                                ; Done

        jmp .NextEntry                              ; Next Entry

    .MemoryError:

        stc                                         ; Set Carry Flag
        cli                                         ; Clear Interrupts
        hlt                                         ; Halt CPU

    .Complete:
        mov [0x2000], BP                            ; [0x2000] = Memory Entry Count
        clc                                         ; Clear Carry Flag

;                                                                                     INIT MEMORY MAP
;----------------------------------------------------------------------------------------------------
;----------------------------------------------------------------------------------------------------
;                                                                                            INIT VBE

InitializeVBE:

    xor AX, AX                                  ; Reset ES = 0x0000
    mov ES, AX                                  ; INT 0x10, [ES:DI] = SVGA Info Buffer

    mov AH, 0x4F                                ; INT 0x10, AH = 0x4F, VESA BIOS | AL = 0x00, Get Info
    mov DI, VBE_INFO                            ; INT 0x10, DI = VBE Info Structure
    int 0x10                                    ; Get SVGA Information

    cmp AX, 0x4F                                ; AX != 0x004F
    jne InitializeVBEError                      ; ERROR: Unsupported Function
    
    mov AX, word [VBE_INFO.VIDEO_MODE_POINTER]  ; AX = VBE Mode Offset
    mov [VBE_OFFSET], AX                        ; VBEOffset = AX
    mov AX, word [VBE_INFO.VIDEO_MODE_POINTER+2]; AX = VBE Mode Segment
    mov [VBE_SEGMENT], AX                       ; VBESegment = AX

    mov FS, AX                                  ; FS = VBESegment
    mov SI, [VBE_OFFSET]                        ; SI = VBEOffset

    .ValidateMode:

        mov DX, [FS:SI]                             ; DX = [Mode:Offset]
        inc SI                                      ; Offset += 1 Byte
        inc SI                                      ; Offset += 1 Byte
        mov [VBE_OFFSET], SI                        ; Store Offset
        mov [VBE_MODE], DX                          ; Store Mode

        cmp DX, word 0xFFFF                         ; DX == Max Video Mode #
        je InitializeVBEError                       ; ERROR: End Of Video Modes
        
        mov AX, 0x4F01                              ; INT 0x10, AH = 0x4F, VESA BIOS | AL = 0x01, Get Mode Info
        mov CX, [VBE_MODE]                          ; INT 0x10, CX = Mode Bitfields
        mov DI, VBE_MODE_INFO                       ; INT 0x10, DI = VBE Mode Info Structure
        int 0x10                                    ; Get SVGA Mode Information

        cmp AX, 0x4F                                ; AX != 0x004F
        jne InitializeVBEError                      ; ERROR: Unsupported Function
   
        mov AX, [VBE_WIDTH]                         ; AX = Request Width
        cmp AX, [VBE_MODE_INFO.X_RESOLUTION]        ; Request Width != Mode Width
        jne .NextMode                               ; Next Mode

        mov AX, [VBE_HEIGHT]                        ; AX = Request Height
        cmp AX, [VBE_MODE_INFO.Y_RESOLUTION]        ; Request Height != Mode Height
        jne .NextMode                               ; Next Mode

        mov AX, [VBE_BPP]                           ; AX = Request Bitdepth
        cmp AL, [VBE_MODE_INFO.BITS_PER_PIXEL]      ; Request Bitdepth != Mode Bitdepth
        jne .NextMode                               ; Next Mode
        
        mov AX, 0x4F02                              ; INT 0x10, AH = 0x4F, VESA BIOS | AL = 0x02, Set Video Mode
        mov BX, [VBE_MODE]                          ; INT 0x10, BX = Video Mode
        or BX, 0x4000                               ; INT 0x10, BX = 0100000000000000, Bit 14: Enable Linear Frame Buffer
        xor DI, DI                                  ; INT 0x10, DI = CRTC Information Structure
        int 0x10                                    ; Set Video Mode

        cmp AX, 0x4F                                ; AX != 0x004F
        jne InitializeVBEError                      ; ERROR: Unsupported Function
        
        jmp InitializeGDT                           ; Initialize GDT

        .NextMode:

            mov AX, [VBE_SEGMENT]                       ; AX = VBESegment
            mov FS, AX                                  ; Restore Segment
            mov SI, [VBE_OFFSET]                        ; Restore Offset
            jmp .ValidateMode                           ; Validate Next Mode

    InitializeVBEError:

        cli                                         ; Clear Interrupts
        hlt                                         ; Halt CPU
;                                                                                            INIT VBE
;----------------------------------------------------------------------------------------------------
;----------------------------------------------------------------------------------------------------
;                                                                                            INIT GDT

InitializeGDT:

    cli                                         ; Clear Interrupts

    xor AX, AX                                  ; Set DS
    mov DS, AX                                  ; Reset [DS:GDT_DESCRIPTOR]

    lgdt [GDT_DESCRIPTOR]                       ; Load [GDT Descriptor]

    mov EAX, CR0                                ; Set CR0
    or  EAX, 1                                  ; Set bit 0
    mov CR0, EAX                                ; CR0 = EAX

;                                                                                            INIT GDT
;----------------------------------------------------------------------------------------------------
;----------------------------------------------------------------------------------------------------
;                                                                                ENTER PROTECTED MODE
    jmp CODE_SEGMENT:ProtectedMode              ; CS = GDT Code Segment
;                                                                                ENTER PROTECTED MODE
;----------------------------------------------------------------------------------------------------
;----------------------------------------------------------------------------------------------------
; PRINT HEX
;----------------------------------------------------------------------------------------------------
; -> DX Value
;----------------------------------------------------------------------------------------------------
; - Prints the value pointed to by DX as a hexadecimal string to the screen
;----------------------------------------------------------------------------------------------------

PrintHex:

    MOV CX, 4                                   ;

    .PrintHexLoop:

        mov AX, DX                                  ; DX = 
        and AL, 0x0F                                ; AL = 0x0F
        mov BX, HEX_ASCII                           ; BX =
        xlatb                                       ; AL = [DS:BX + AL]

        mov BX, CX                                  ;
        mov [HEX_STRING+BX+1], AL                   ; Store Hex Character
        ror DX, 4                                   ; DX = Next Character

    loop .PrintHexLoop

    mov SI, HEX_STRING                          ;
    mov CX, 6                                   ;
;                                                                                           PRINT HEX
;----------------------------------------------------------------------------------------------------
;----------------------------------------------------------------------------------------------------
; PRINT STRING
;----------------------------------------------------------------------------------------------------
; -> SI String Address
; -> CX String Length
;----------------------------------------------------------------------------------------------------
; - Prints the string pointed to by SI to the screen
;----------------------------------------------------------------------------------------------------

PrintString:

    mov AH, 0x0E                                ; INT 0x10, AH = 0x0E, Teletype Output

    .PrintLoop:

        lodsb                                       ; INT 0x10, AL = [DS:SI], Character
        int 0x10                                    ; Teletype Ouput

    loop .PrintLoop

    ret                                         ; Return
;                                                                                        PRINT STRING
;----------------------------------------------------------------------------------------------------

;----------------------------------------------------------------------------------------------------
;                                                                                         INIT KERNEL
    [BITS 32]

ProtectedMode:

    mov AX, DATA_SEGMENT                        ; AX = GDT Data Segment
    mov DS, AX                                  ; DS = GDT Data Segment
    mov ES, AX                                  ; ES = GDT Data Segment
    mov FS, AX                                  ; FS = GDT Data Segment
    mov GS, AX                                  ; GS = GDT Data Segment
    mov SS, AX                                  ; SS = GDT Data Segment

    mov ESP, 0x90000                            ; SP = 0x90000
    mov EBP, ESP                                ; BP = 0x90000

    mov ESI, VBE_MODE_INFO                      ; ESI = VBEModeInfo
    mov EDI, 0x9000                             ; EDI = 0x9000
    mov ECX, 64                                 ; ECX = 64, (VBEModeInfo: 256 Byte / DWORD: 4 Byte)
    rep movsd                                   ; EDI[ECX] = ESI[ECX] | ECX--

;                                                                                         INIT KERNEL
;----------------------------------------------------------------------------------------------------
;----------------------------------------------------------------------------------------------------
;                                                                                      EXECUTE KERNEL

    jmp CODE_SEGMENT:0x50000                    ; Execute Kernel
;                                                                                      EXECUTE KERNEL
;----------------------------------------------------------------------------------------------------

BootLoaderEnd:

    cli                                         ; Clear Interrupts
    hlt                                         ; Halt CPU
;                                                                                         BOOT LOADER
;----------------------------------------------------------------------------------------------------

;----------------------------------------------------------------------------------------------------
; (G)LOBAL (D)ESCRIPTOR (T)ABLE
;----------------------------------------------------------------------------------------------------

    GDT_BEGIN:
    GDT_NULL_DESCRIPTOR:
        dd 0x00000000                           ; NULL Descriptor
        dd 0x00000000                           ; NULL Descriptor
    GDT_CODE_SEGMENT:                           ; GDT Offset 0x08
        dw 0xFFFF                               ; [15.. 0] Limit
        dw 0x0000                               ; [31..16] Base
        db 0x00                                 ; [39..32] Base
        db 10011010b                            ; [47..40] Access
        db 11001111b                            ; [55..52] Flags  | [51..48] Limit
        db 0x00                                 ; [63..56] Base
    GDT_DATA_SEGMENT:                           ; GDT Offset 0x10
        dw 0xFFFF                               ; [15.. 0] Limit
        dw 0x0000                               ; [31..16] Base
        db 0x00                                 ; [39..32] Base
        db 10010010b                            ; [47..40] Access
        db 11001111b                            ; [55..52] Flags  | [51..48] Limit
        db 0x00                                 ; [63..56] Base
    GDT_END:

    GDT_DESCRIPTOR:
        dw GDT_END - GDT_BEGIN - 1              ; GDT Size
        dd GDT_BEGIN                            ; GDT Memory address

    CODE_SEGMENT equ GDT_CODE_SEGMENT - GDT_BEGIN
    DATA_SEGMENT equ GDT_DATA_SEGMENT - GDT_BEGIN

;                                                                       (G)LOBAL (D)ESCRIPTOR (T)ABLE
;----------------------------------------------------------------------------------------------------

;----------------------------------------------------------------------------------------------------
; (V)ESA (B)IOS (E)XTENSIONS
;----------------------------------------------------------------------------------------------------

    VBE_INFO:
        .VBE_SIGNATURE:                 db "VBE2"
        .VBE_VERSION:                   dw 0x0000
        .OEM_STRING_POINTER:            dd 0x00000000
        .CAPABILITIES:                  dd 0x00000000
        .VIDEO_MODE_POINTER:            dd 0x00000000
        .TOTAL_MEMORY:                  dw 0x0000
        .OEM_SOFTWARE_REV:              dw 0x0000
        .OEM_VENDOR_NAME_POINTER:       dd 0x00000000
        .OEM_PRODUCT_NAME_POINTER:      dd 0x00000000
        .OEM_PRODUCT_REVISION_POINTER:  dd 0x00000000
        .RESERVED: times 222            db 0x00
        .OEM_DATA: times 256            db 0x00

    VBE_MODE_INFO:
        .MODE_ATTRIBUTES:               dw 0x0000
        .WINDOW_A_ATTRIBUTES:           db 0x00
        .WINDOW_B_ATTRIBUTES:           db 0x00
        .WINDOW_GRANULARITY:            dw 0x0000
        .WINDOW_SIZE:                   dw 0x0000
        .WINDOW_A_SEGMENT:              dw 0x0000
        .WINDOW_B_SEGMENT:              dw 0x0000
        .WINDOW_FUNCTION_POINTER:       dd 0x00000000
        .BYTES_PER_SCANLINE:            dw 0x0000       ; 0x10 | 16

        .X_RESOLUTION:                  dw 0x0000       ; 0x12 | 18
        .Y_RESOLUTION:                  dw 0x0000       ; 0x14 | 20
        .X_CHARSIZE:                    db 0x00
        .Y_CHARSIZE:                    db 0x00
        .NUMBER_OF_PLANES:              db 0x00
        .BITS_PER_PIXEL:                db 0x00
        .NUMBER_OF_BANKS:               db 0x00
        .MEMORY_MODEL:                  db 0x00
        .BANK_SIZE:                     db 0x00
        .NUMBER_OF_IMAGE_PAGES:         db 0x00
        .RESERVED1:                     db 0x00

        .RED_MASK_SIZE:                 db 0x00
        .RED_FIELD_POS:                 db 0x00
        .GREEN_MASK_SIZE:               db 0x00
        .GREEN_FIELD_POS:               db 0x00
        .BLUE_MASK_SIZE:                db 0x00
        .BLUE_FIELD_POS:                db 0x00
        .RESERVED_MASK_SIZE:            db 0x00
        .RESERVED_FIELD_POS:            db 0x00
        .DIRECT_COLOR_MODE_INFO:        db 0x00

        .FRAME_BUFFER:                  dd 0x00000000   ; 0x28 | 40
        .RESERVED2:                     dd 0x00000000
        .RESERVED3:                     dw 0x0000

        .LINEAR_BYTES_PER_SCAN_LINE:    dw 0x0000
        .BANK_NUMER_OF_IMAGE_PAGES:     db 0x00
        .LINEAR_NUMBER_OF_IMAGE_PAGES:  db 0X00
        .LINEAR_RED_MASK_SIZE:          db 0x00
        .LINEAR_RED_FIELD_POS:          db 0x00
        .LINEAR_GREEN_MASK_SIZE:        db 0x00
        .LINEAR_GREEN_FIELD_POS:        db 0x00
        .LINEAR_BLUE_MASK_SIZE:         db 0x00
        .LINEAR_BLUE_FIELD_POS:         db 0x00
        .LINEAR_RESERVED_MASK_SIZE:     db 0x00
        .LINEAR_RESERVED_FIELD_POS:     db 0x00
        .MAX_PIXEL_CLOCK:               dd 0x00000000

        .RESERVED4: times 190           db 0x00
;                                                                          (V)ESA (B)IOS (E)XTENSIONS
;----------------------------------------------------------------------------------------------------

;----------------------------------------------------------------------------------------------------
; DATA
;----------------------------------------------------------------------------------------------------

    HEX_STRING: db "0x0000"
    HEX_ASCII: db "0123456789ABCDEF"

    VBE_WIDTH: dw 1024
    VBE_HEIGHT: dw 768
    VBE_BPP: db 32
    VBE_OFFSET: dw 0
    VBE_SEGMENT: dw 0
    VBE_MODE: dw 0
;                                                                                                DATA
;----------------------------------------------------------------------------------------------------

;----------------------------------------------------------------------------------------------------
; SECTOR
;----------------------------------------------------------------------------------------------------

    times 2048-($-$$) db 0x00
;                                                                                              SECTOR
;----------------------------------------------------------------------------------------------------