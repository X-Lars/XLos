# Rule Prefix
.RECIPEPREFIX = >

# Options
OPTION-C = true

# Compiler Constants
ASM-COMPILER = nasm
ASM-COMPILER-OPTIONS = -f bin -i $(INC-DIR)
C-COMPILER = gcc
C-COMPILER-OPTIONS = -m32 -c -fno-pie -ffreestanding -nostdlib -nodefaultlibs -O0 -mgeneral-regs-only
LINKER-OPTIONS = -m elf_i386 -T build/kernel.ld --oformat binary

# Directories
BIN-DIR = bin/
SRC-DIR = src/
INC-DIR = include/
# Files
ASM-FILES = bootloader font
C-FILES = kernel
TEMP-FILES = mbr filetable $(ASM-FILES) $(C-FILES)
BIN-FILES = $(TEMP-FILES:%=$(BIN-DIR)%.bin)

# Required
# BOOTLOADER-BINARIES = $(BOOTLOADER-FILES:%=$(BIN-DIR)%.bin)
# KERNEL-BINARIES = $(KERNEL-FILES:%=$(BIN-DIR)%.bin)
# KERNEL-OBJECTS = $(KERNEL-FILES:%=$(BIN-DIR)%.o)
# OS-OBJECTS = $(OS-FILES:%=$(BIN-DIR)%.os)


all: rebuild run

build: os

rebuild: clean os

os: filetable $(ASM-FILES) $(BIN-DIR)kernel.bin
> @echo "Building OS"
# Get size of file table
> @size=$$(($$(wc -c < $(BIN-DIR)filetable.bin)));\
> newsize=$$((size - $$((size % 512)) + 512));\
> dd if=/dev/zero of=$(BIN-DIR)filetable.bin bs=1 seek=$$size count=$$((newsize - size)) status=none
> @cat $(BIN-FILES) > $(BIN-DIR)temp.bin
> @dd if=/dev/zero of=$(BIN-DIR)os.bin bs=512 count=2880 status=none
> @dd if=$(BIN-DIR)temp.bin of=$(BIN-DIR)os.bin conv=notrunc status=none
> @rm -f $(BIN-DIR)*.tmp
> @rm -f $(BIN-DIR)*[!os].bin

ifeq ($(OPTION-C), true)

$(BIN-DIR)kernel.bin:
> @echo "Building Kernel (C Version)"
> @$(C-COMPILER) $(SRC-DIR)kernel.c -o $(BIN-DIR)kernel.o $(C-COMPILER-OPTIONS)
> @ld $(BIN-DIR)kernel.o -o $(BIN-DIR)kernel.bin $(LINKER-OPTIONS) 
> @size=$$(($$(wc -c < $(BIN-DIR)kernel.bin)));\
> newsize=$$((size - $$((size % 512)) + 512));\
> echo "$$size Byte ->" "$$newsize Byte = $$(printf '0x%02X' $$((newsize / 512))) Sectors";\
> dd if=/dev/zero of=$(BIN-DIR)kernel.bin bs=1 seek=$$size count=$$((newsize - size)) status=none;\
> build/filetable.sh kernel $$((newsize / 512))
> @rm -f $(BIN-DIR)*.o

else

$(BIN-DIR)kernel.bin:
> @echo "Building Kernel (Assembly Version)"
> @$(ASM-COMPILER) $(SRC-DIR)kernel.asm -o $(BIN-DIR)kernel.bin $(ASM-COMPILER-OPTIONS)
> @size=$$(($$(wc -c < $(BIN-DIR)kernel.bin)));\
> echo "$$size Byte = $$(printf '0x%02X' $$((size / 512))) Sector(s)";\
> build/filetable.sh kernel $$((size / 512))

endif

filetable:
> @echo "Generating File Table"
# Write: 1 (-n: No New Line)
> @echo -n 1 > bin/sector.tmp
# Compile MBR
> @$(ASM-COMPILER) $(SRC-DIR)mbr.asm -o $(BIN-DIR)mbr.bin $(ASM-COMPILER-OPTIONS)
# Write file table: MBR
> @build/filetable.sh mbr 1
# Write file table: File Table
> @build/filetable.sh filetable 1

clean:
> @echo "Cleaning Binaries"
> @rm -f bin/*[!os].*

run: os
> @echo "Running OS"
> @qemu-system-i386 -L /usr/bin -drive format=raw,file=bin/os.bin,if=ide,index=0,media=disk -rtc base=localtime,clock=host,driftfix=slew
# qemu-system-x86_64

debug: os
> @echo "Debugging OS"
> @bochs -q -f bochs.cfg

iso: clean os
> @echo "Creating ISO Image"
> @mkdir -p bin/iso
> @cp bin/os.bin bin/iso/
> @genisoimage -quiet -V "XLos" -input-charset iso8859-1 -o bin/os.iso -b os.bin bin/iso/ 1> /dev/null
> @rm -f bin/iso/*.*
> @rm -d bin/iso

$(ASM-FILES):
> @echo "Compiling Assemby" $@
> @$(ASM-COMPILER) $(SRC-DIR)$@.asm -o $(BIN-DIR)$@.bin $(ASM-COMPILER-OPTIONS)
> @size=$$(($$(wc -c < $(BIN-DIR)$@.bin)));\
> echo "$$size Byte = $$(printf '0x%02X' $$((size / 512))) Sector(s)";\
> build/filetable.sh $@ $$((size / 512))

# $(C-FILES):
# > @echo "Compiling C"
# > $(C-COMPILER) $(SRC-DIR)$@.c -o $(BIN-DIR)$@.o $(C-COMPILER-OPTIONS)
# > ld $(BIN-DIR)$@.o -o $(BIN-DIR)$@.bin $(LINKER-OPTIONS)
# > @size=$$(($$(wc -c < $(BIN-DIR)$@.bin)));\
# > newsize=$$((size - $$((size % 512)) + 512));\
# > echo "$$size Byte ->" "$$newsize Byte = $$(printf '0x%02X' $$((newsize / 512))) Sectors";\
# > dd if=/dev/zero of=$@ bs=1 seek=$$size count=$$((newsize - size)) status=none;\
# > build/filetable.sh $@ $$((newsize / 512))

# # *.asm -> *.bin
# $(BIN-DIR)%.bin: $(SRC-DIR)%.asm
# > @echo "Compiling" $@
# > @$(ASM-COMPILER) $< -o $@ $(ASM-COMPILER-OPTIONS)
# > @size=$$(($$(wc -c < $@)));\
# > echo "$$size Byte = $$(printf '0x%02X' $$((size / 512))) Sector(s)";\
# #> build/filetable.sh $(notdir $(basename $@)) $$((size / 512))

# # *.c -> *.o
# $(BIN-DIR)%.o: $(SRC-DIR)%.c
# > @echo "Compiling" $@
# > @$(C-COMPILER) $< -o $@ $(C-COMPILER-OPTIONS)
# > @size=$$(($$(wc -c < $@)));\
# > newsize=$$((size - $$((size % 512)) + 512));\
# > echo "$$size Byte ->" "$$newsize Byte = $$(printf '0x%02X' $$((newsize / 512))) Sectors";\
# > dd if=/dev/zero of=$@ bs=1 seek=$$size count=$$((newsize - size)) status=none;\
# #> build/filetable.sh $(notdir $(basename $@)) $$((newsize / 512))
