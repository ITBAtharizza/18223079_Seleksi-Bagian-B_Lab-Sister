# Compiler & linker
ASM           = nasm
LIN           = ld
CC            = gcc
ISO           = genisoimage
DISK_NAME     = storage

# Directory
SOURCE_FOLDER = src
SOURCE_FOLDER_CODE = src/code
SOURCE_FOLDER_STDLIB = src/stdlib
SOURCE_FOLDER_KERNEL = src/kernel
SOURCE_FOLDER_ASM = src/asm
SOURCE_FOLDER_USER = src/usermode
SOURCE_FOLDER_CLOCK = src/clock
OUTPUT_FOLDER = bin
ISO_NAME      = OS2024

# Flags
WARNING_CFLAG = -Wall -Wextra -Werror
DEBUG_CFLAG   = -fshort-wchar -g
STRIP_CFLAG   = -nostdlib -fno-stack-protector -nostartfiles -nodefaultlibs -ffreestanding
CFLAGS        = $(DEBUG_CFLAG) $(WARNING_CFLAG) $(STRIP_CFLAG) -m32 -c -I$(SOURCE_FOLDER)
AFLAGS        = -f elf32 -g -F dwarf
LFLAGS        = -T $(SOURCE_FOLDER)/linker.ld -melf_i386
ISOFLAGS      = -R -b boot/grub/grub1 -no-emul-boot -boot-load-size 4 -A os -input-charset utf8 -quiet -boot-info-table -o bin/OS2024.iso bin/iso

#otomisasi 
SRCS_CODE := $(wildcard $(SOURCE_FOLDER)/code/*)
SRCS_STDLIB := $(wildcard $(SOURCE_FOLDER)/stdlib/*.c)
SRCS_KERNEL := $(wildcard $(SOURCE_FOLDER)/kernel/*.c)
ASM_SRCS := $(wildcard $(SOURCE_FOLDER_ASM)/*.s)

OBJS_CODE := $(foreach dir,$(SRCS_CODE), $(wildcard $(dir)/*.c))

OBJS_STDLIB := $(patsubst $(SOURCE_FOLDER)/stdlib/%.c, $(OUTPUT_FOLDER)/%.o, $(SRCS_STDLIB))
OBJS_KERNEL := $(patsubst $(SOURCE_FOLDER)/kernel/%.c, $(OUTPUT_FOLDER)/%.o, $(SRCS_KERNEL))
ASM_OBJS := $(patsubst $(SOURCE_FOLDER_ASM)/%.s, $(OUTPUT_FOLDER)/%.o, $(ASM_SRCS))
OBJS := $(patsubst $(SOURCE_FOLDER)/code/%.c, $(OUTPUT_FOLDER)/%.o, $(OBJS_CODE))

$(shell mkdir -p $(dir $(OBJS)))

$(OUTPUT_FOLDER)/%.o: $(SOURCE_FOLDER)/code/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(OUTPUT_FOLDER)/%.o: $(SOURCE_FOLDER_STDLIB)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(OUTPUT_FOLDER)/%.o: $(SOURCE_FOLDER_KERNEL)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(OUTPUT_FOLDER)/%.o: $(SOURCE_FOLDER_ASM)/%.s
	$(ASM) $(AFLAGS) $< -o $@

#prerequisites
prereq = $(OBJS) $(OBJS_STDLIB) $(OBJS_KERNEL) $(ASM_OBJS)  
#main
run: all
	@qemu-system-i386 -s -S -drive file=$(OUTPUT_FOLDER)/storage.bin,format=raw,if=ide,index=0,media=disk -cdrom $(OUTPUT_FOLDER)/$(ISO_NAME).iso 
all: build
build: iso
clean:
	$(shell rm -rf  $(dir $(OBJS)))
	rm -rf $(OUTPUT_FOLDER)/*.o
	rm -rf *.o *.iso $(OUTPUT_FOLDER)/kernel
	rm -rf $(OUTPUT_FOLDER)/*.iso

t: $(prereq)
	echo $^

kernel: $(prereq)
	@$(LIN) $(LFLAGS) $^ -o $(OUTPUT_FOLDER)/kernel
	@echo Linking object files and generate elf32...
	@rm -f *.o

disk:
	@qemu-img create -f raw $(OUTPUT_FOLDER)/$(DISK_NAME).bin 4M

iso: kernel
	mkdir -p $(OUTPUT_FOLDER)/iso/boot/grub
	cp $(OUTPUT_FOLDER)/kernel     $(OUTPUT_FOLDER)/iso/boot/
	cp other/grub1                 $(OUTPUT_FOLDER)/iso/boot/grub/
	cp $(SOURCE_FOLDER)/menu.lst   $(OUTPUT_FOLDER)/iso/boot/grub/
	$(ISO) $(ISOFLAGS)
	rm -r $(OUTPUT_FOLDER)/iso/
	@echo succesfully linked files

inserter:
	@$(CC) -Wno-builtin-declaration-mismatch -g -I$(SOURCE_FOLDER) \
		$(SOURCE_FOLDER)/stdlib/string.c \
		$(SOURCE_FOLDER_CODE)/filesystem/fat32.c \
		$(SOURCE_FOLDER)/helper/external-inserter.c \
		-o $(OUTPUT_FOLDER)/inserter

user-shell:
	$(ASM) $(AFLAGS) $(SOURCE_FOLDER_USER)/crt0.s -o $(OUTPUT_FOLDER)/crt0.o
	$(CC) $(CFLAGS) -fno-pie -c $(SOURCE_FOLDER_USER)/user-shell.c -o $(OUTPUT_FOLDER)/user-shell.o
	$(CC) $(CFLAGS) -fno-pie -c $(SOURCE_FOLDER_USER)/mkdir.c -o $(OUTPUT_FOLDER)/mkdir.o
	$(CC) $(CFLAGS) -fno-pie -c $(SOURCE_FOLDER_USER)/ls.c -o $(OUTPUT_FOLDER)/ls.o
	$(CC) $(CFLAGS) -fno-pie -c $(SOURCE_FOLDER_USER)/cd.c -o $(OUTPUT_FOLDER)/cd.o
	$(CC) $(CFLAGS) -fno-pie -c $(SOURCE_FOLDER)/stdlib/string.c -o $(OUTPUT_FOLDER)/string.o
	$(CC) $(CFLAGS) -fno-pie -c $(SOURCE_FOLDER_USER)/rm.c -o $(OUTPUT_FOLDER)/rm.o
	$(CC) $(CFLAGS) -fno-pie -c $(SOURCE_FOLDER_USER)/util.c -o $(OUTPUT_FOLDER)/util.o 
	$(CC) $(CFLAGS) -fno-pie -c $(SOURCE_FOLDER_USER)/cat.c -o $(OUTPUT_FOLDER)/cat.o 
	$(CC) $(CFLAGS) -fno-pie -c $(SOURCE_FOLDER_USER)/cp.c -o $(OUTPUT_FOLDER)/cp.o 
	$(CC) $(CFLAGS) -fno-pie -c $(SOURCE_FOLDER_USER)/mv.c -o $(OUTPUT_FOLDER)/mv.o 
	$(CC) $(CFLAGS) -fno-pie -c $(SOURCE_FOLDER_USER)/find.c -o $(OUTPUT_FOLDER)/find.o 
	$(CC) $(CFLAGS) -fno-pie -c $(SOURCE_FOLDER_USER)/ps.c -o $(OUTPUT_FOLDER)/ps.o 
	$(CC) $(CFLAGS) -fno-pie -c $(SOURCE_FOLDER_USER)/exec.c -o $(OUTPUT_FOLDER)/exec.o 
	$(CC) $(CFLAGS) -fno-pie -c $(SOURCE_FOLDER_USER)/kill.c -o $(OUTPUT_FOLDER)/kill.o 
	$(CC) $(CFLAGS) -fno-pie -c $(SOURCE_FOLDER_USER)/txt.c -o $(OUTPUT_FOLDER)/txt.o 
	$(CC) $(CFLAGS) -fno-pie -c $(SOURCE_FOLDER_USER)/write.c -o $(OUTPUT_FOLDER)/write.o 
	$(LIN) -T $(SOURCE_FOLDER_USER)/user-linker.ld -melf_i386 --oformat=binary \
		$(OUTPUT_FOLDER)/crt0.o $(OUTPUT_FOLDER)/user-shell.o $(OUTPUT_FOLDER)/mkdir.o \
		$(OUTPUT_FOLDER)/ls.o $(OUTPUT_FOLDER)/rm.o $(OUTPUT_FOLDER)/cd.o $(OUTPUT_FOLDER)/string.o \
		$(OUTPUT_FOLDER)/util.o $(OUTPUT_FOLDER)/cat.o  $(OUTPUT_FOLDER)/cp.o $(OUTPUT_FOLDER)/mv.o $(OUTPUT_FOLDER)/find.o \
		$(OUTPUT_FOLDER)/ps.o $(OUTPUT_FOLDER)/exec.o $(OUTPUT_FOLDER)/kill.o $(OUTPUT_FOLDER)/txt.o \
		$(OUTPUT_FOLDER)/write.o \
		-o $(OUTPUT_FOLDER)/shell
	@echo Linking object shell object files and generating flat binary...
	$(LIN) -T $(SOURCE_FOLDER_USER)/user-linker.ld -melf_i386 --oformat=elf32-i386 \
		$(OUTPUT_FOLDER)/crt0.o $(OUTPUT_FOLDER)/user-shell.o $(OUTPUT_FOLDER)/mkdir.o \
		$(OUTPUT_FOLDER)/ls.o  $(OUTPUT_FOLDER)/rm.o $(OUTPUT_FOLDER)/cd.o $(OUTPUT_FOLDER)/string.o \
		$(OUTPUT_FOLDER)/util.o $(OUTPUT_FOLDER)/cat.o $(OUTPUT_FOLDER)/cp.o $(OUTPUT_FOLDER)/mv.o $(OUTPUT_FOLDER)/find.o \
		$(OUTPUT_FOLDER)/ps.o $(OUTPUT_FOLDER)/exec.o $(OUTPUT_FOLDER)/kill.o $(OUTPUT_FOLDER)/txt.o \
		$(OUTPUT_FOLDER)/write.o \
		-o $(OUTPUT_FOLDER)/shell_elf
	@echo Linking object shell object files and generating ELF32 for debugging...
	size --target=binary $(OUTPUT_FOLDER)/shell
	rm -f $(OUTPUT_FOLDER)/*.o



insert-shell: disk inserter user-shell
	@echo Inserting shell into root directory...
	@cd $(OUTPUT_FOLDER); ./inserter shell 2 $(DISK_NAME).bin

clock: 
	$(ASM) $(AFLAGS) $(SOURCE_FOLDER_USER)/crt0.s -o $(OUTPUT_FOLDER)/crt0.o
	$(CC) $(CFLAGS) -fno-pie -c $(SOURCE_FOLDER_CODE)/driver/cmos.c -o $(OUTPUT_FOLDER)/cmos.o
	$(CC) $(CFLAGS) -fno-pie -c $(SOURCE_FOLDER_CLOCK)/clock.c -o $(OUTPUT_FOLDER)/clock.o
	$(LIN) -T $(SOURCE_FOLDER_USER)/user-linker.ld -melf_i386 --oformat=binary \
		$(OUTPUT_FOLDER)/crt0.o $(OUTPUT_FOLDER)/clock.o $(OUTPUT_FOLDER)/cmos.o \
		-o $(OUTPUT_FOLDER)/clock
	@echo Linking object shell object files and generating flat binary...
	$(LIN) -T $(SOURCE_FOLDER_USER)/user-linker.ld -melf_i386 --oformat=elf32-i386 \
		$(OUTPUT_FOLDER)/crt0.o $(OUTPUT_FOLDER)/clock.o $(OUTPUT_FOLDER)/cmos.o \
		-o $(OUTPUT_FOLDER)/clock_elf
	rm -f $(OUTPUT_FOLDER)/*.o	

insert-clock: clock inserter
	@echo inserting clock into root directory.. 
	cd $(OUTPUT_FOLDER); ./inserter clock 2 $(DISK_NAME).bin



insert-frametes:
	@echo inserting frametes into root directory..
	@cd $(OUTPUT_FOLDER); ./inserter frametes 2 $(DISK_NAME).bin

restart: clean disk inserter insert-shell

.PHONY: all