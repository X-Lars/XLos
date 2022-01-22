;----------------------------------------------------------------------------------------------------
; (M)ASTER (B)OOT (R)ECORD                                                           [0x7C00][0x7DFF]
;----------------------------------------------------------------------------------------------------
; -> DL = BIOS Boot Drive ID
;----------------------------------------------------------------------------------------------------
; <- [0x1000]  File Table
; <- [0x1500]  BIOS Boot Drive ID
; <- [0x6000]  Font
; <- [0x7E00]  Boot Loader
; <- [0x50000] Kernel
;----------------------------------------------------------------------------------------------------
; - Loads the file table into memory [0x1000]
; - Loads the font into memory [0x6000]
; - Loads the second stage boot loader into memory [0x7E00]
; - Loads the kernel into memory [0x50000]
; - Executes the second stage boot loader
;----------------------------------------------------------------------------------------------------

[ORG 0x7C00]
[BITS 16]

;----------------------------------------------------------------------------------------------------
;                                                                            (M)ASTER (B)OOT (R)ECORD

MBR:

    mov [0x1500], DL                            ; [0x1500] = BIOS Boot Drive ID

    xor AX, AX
    mov BX, AX
    mov CX, AX
    mov DX, AX
    mov DS, AX
    mov ES, AX
    mov FS, AX
    mov GS, AX
    mov SS, AX

;----------------------------------------------------------------------------------------------------
;                                                                                     LOAD FILE TABLE

MBRFileTable:

    mov BX, 0x1000                              ; INT 0x13, [ES:BX] = [0x0:1000], File Table Memory

    mov [0x1501], byte 3                        ; [0x1501] = [Retry Count]

    mov CX, 0x0002                              ; INT 0x13, CH = 0x00, Cylinder # | CL = 0x02, Sector #
    mov DH, 0x00                                ; INT 0x13, DH = 0x00, Head #
    mov DL, [0x1500]                            ; INT 0x13, DL = Drive #

    .Retry:
        
        cmp [0x1501], byte 0                        ; [Retry Count] == 0
        je MBRError                                 ; Error

        mov AH, 0x00                                ; INT 0x13, AH = 0x00, Reset Disk System
        int 0x13                                    ; Reset Disk System

        jc MBRError                                 ; CF == 1 ? Error : Continue

        dec byte [0x1501]                           ; [Retry Count] -= 1

        mov AX, 0x0201                              ; INT 0x13, AH = 0x02, Read Disk Sectors | AL = 0x02, # Sectors
        int 0x13                                    ; Read Disk Sectors

    jc .Retry                                   ; CF == 1 ? Retry : Continue

;                                                                                     LOAD FILE TABLE
;----------------------------------------------------------------------------------------------------
;----------------------------------------------------------------------------------------------------
;                                                                                          LOAD FILES
   
    mov SI, BOOT_BOOTLOADER                     ; SI = Boot Loader File Name
    call MBRFileTableEntry                      ; SI = Boot Loader File Table Entry
    mov BX, 0x7E00                              ; BX = Boot Loader Address
    call MBRLoadFile                            ; Load Boot Loader
    
    mov SI, BOOT_FONT                           ; SI = Font File Name
    call MBRFileTableEntry                      ; SI = Font File Table Entry
    mov BX, 0x6000                              ; BX = Font Address
    call MBRLoadFile                            ; Load Font
    
    mov SI, BOOT_KERNEL                         ; SI = Kernel File Name
    call MBRFileTableEntry                      ; SI = Kernel File Table Entry
    mov BX, 0x5000                              ; BX = Kernel Address
    mov ES, BX                                  ; ES = 0x5000
    xor BX, BX                                  ; [ES:BX] = [0x5000:0]
    call MBRLoadFile                            ; Load Kernel

;                                                                                          LOAD FILES
;----------------------------------------------------------------------------------------------------
;----------------------------------------------------------------------------------------------------
;                                                                                 EXECUTE BOOT LOADER

    jmp 0x0000:0x7E00                           ; Execute Boot Loader

;                                                                                 EXECUTE BOOT LOADER
;----------------------------------------------------------------------------------------------------

MBRError:

    cli                                         ; Clear Interrupts
    hlt                                         ; Halt CPU

;                                                                            (M)ASTER (B)OOT (R)ECORD
;----------------------------------------------------------------------------------------------------

;----------------------------------------------------------------------------------------------------
; LOAD FILE
;----------------------------------------------------------------------------------------------------
; -> SI      = File Table Entry
; -> [ES:BX] = Memory Address
;----------------------------------------------------------------------------------------------------
; - Loads the specified file into memory
;----------------------------------------------------------------------------------------------------
MBRLoadFile:

    mov [0x1501], byte 3                        ; [0x1501] = [Retry Count]

    mov CH, 0x00                                ; INT 0x13, CH = 0x00, Cylinder #
    mov CL, [SI+14]                             ; INT 0x13, CL = Sector #
    mov DH, 0x00                                ; INT 0x13, DH = 0x00, Head #
    mov DL, [0x1500]                            ; INT 0x13, DL = Drive #

    .Retry:
        
        cmp [0x1501], byte 0                        ; [Retry Count] == 0
        je MBRError                                 ; Error

        mov AH, 0x00                                ; AH = 0x00, Reset Disk System
        int 0x13                                    ; Reset Disk System

        jc MBRError                                 ; CF == 1 ? Error : Continue

        dec byte [0x1501]                           ; [Retry Count] -= 1

        mov AH, 0x02                                ; INT 0x13, AH = 0x02, Read Disk Sectors
        mov AL, [SI+15]                             ; INT 0x13, AL = # Sectors
        int 0x13                                    ; Read Disk Sectors

    jc .Retry                                   ; CF == 1 ? Retry : Continue        

    ret                                         ; Return
;                                                                                           LOAD FILE
;----------------------------------------------------------------------------------------------------

;----------------------------------------------------------------------------------------------------
; FILE TABLE ENTRY
;----------------------------------------------------------------------------------------------------
; -> SI = File Name
;----------------------------------------------------------------------------------------------------
; <- SI = File Table Entry
;----------------------------------------------------------------------------------------------------
; - Returns the file table entry for the specified file name
;----------------------------------------------------------------------------------------------------
MBRFileTableEntry:

    mov DI, 0x1000                              ; DI = File Table Address

    .loop:

        mov CL, 10                                  ; CL = 10, File Name Length

        push DI                                     ; Store DI
        push SI                                     ; Store SI

        repe cmpsb                                  ; SI[CL] == DI[CL] | CL--
        je .return

        pop SI                                      ; Restore SI
        pop DI                                      ; Restore DI

        add DI, 16                                  ; DI += 16, Next Entry
        
    jmp .loop

    .return:

        pop SI                                      ; Restore SI
        pop DI                                      ; Restore DI

        mov SI, DI                                  ; SI = DI, File Table Address

        ret                                         ; Return

;                                                                                    FILE TABLE ENTRY
;----------------------------------------------------------------------------------------------------

;----------------------------------------------------------------------------------------------------
; DATA
;----------------------------------------------------------------------------------------------------

    BOOT_BOOTLOADER: db "bootloader"
    BOOT_FONT:       db "font      "
    BOOT_KERNEL:     db "kernel    "
;                                                                                                DATA
;----------------------------------------------------------------------------------------------------

;----------------------------------------------------------------------------------------------------
; SECTOR
;----------------------------------------------------------------------------------------------------
; - Pads the file to 510 bytes of the required 512 bytes
; - Appends the 2 byte signature 0x55 0xAA required by BIOS to identify the (M)aster (B)oot (R)ecord
;----------------------------------------------------------------------------------------------------

    times 510-($-$$) db 0x00
    dw 0xAA55
;                                                                                              SECTOR  
;----------------------------------------------------------------------------------------------------
