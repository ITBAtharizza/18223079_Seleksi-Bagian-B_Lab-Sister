#ifndef SYSCALL_H
#define SYSCALL_H

#include <stdint.h>
#include "header/filesystem/fat32.h"
#include "header/stdlib/string.h"
#include "header/driver/keyboard.h"
#include "header/process/process.h"

#define READ 0 
#define READ_DIRECTORY 1 
#define WRITE 2 
#define DELETE 3
#define PUT_CHAR 5 
#define PUT_CHARS 6 
#define ACTIVATE_KEYBOARD 7 
#define DEACTIVATE_KEYBOARD 8 
#define CLEAR 9
#define GET_PROMPT 10 
#define CHANGE_DIR 13
#define EXEC 15 
#define KILL 16 
#define LIST_PRINT 17
#define LIST 18

#define BIOS_LIGHT_GREEN 0b1010
#define BIOS_GREY        0b0111
#define BIOS_DARK_GREY   0b1000
#define BIOS_LIGHT_BLUE  0b1001
#define BIOS_RED         0b1100
#define BIOS_BROWN       0b0110
#define BIOS_WHITE       0b1111
#define BIOS_BLACK       0b0000


#define MAX_PROMPT 512 //gada perintah yang melebihi ini

struct ShellState {
	struct FAT32DirectoryTable curr_dir;
    uint32_t current_directory;
	char* current_directory_name;
	char prompt_val[MAX_PROMPT];
	int prompt_size;
	char path_to_print[2048];
};

extern struct ShellState state;

int findEntryName(char* name);

void syscall(uint32_t eax, uint32_t ebx, uint32_t ecx, uint32_t edx);

void updateDirectoryTable(uint32_t cluster_number);

#endif
