#ifndef _PROCESS_H
#define _PROCESS_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "header/cpu/interrupt.h"
#include "header/memory/paging.h"
#include "header/filesystem/fat32.h"

#define PROCESS_NAME_LENGTH_MAX          32
#define PROCESS_PAGE_FRAME_COUNT_MAX     8
#define PROCESS_COUNT_MAX                16

#define KERNEL_RESERVED_PAGE_FRAME_COUNT 4
#define KERNEL_VIRTUAL_ADDRESS_BASE      0xC0000000

#define CPU_EFLAGS_BASE_FLAG               0x2
#define CPU_EFLAGS_FLAG_CARRY              0x1
#define CPU_EFLAGS_FLAG_PARITY             0x4
#define CPU_EFLAGS_FLAG_AUX_CARRY          0x10
#define CPU_EFLAGS_FLAG_ZERO               0x40
#define CPU_EFLAGS_FLAG_SIGN               0x80
#define CPU_EFLAGS_FLAG_TRAP               0x100
#define CPU_EFLAGS_FLAG_INTERRUPT_ENABLE   0x200
#define CPU_EFLAGS_FLAG_DIRECTION          0x400
#define CPU_EFLAGS_FLAG_OVERFLOW           0x800
#define CPU_EFLAGS_FLAG_IO_PRIVILEGE       0x3000
#define CPU_EFLAGS_FLAG_NESTED_TASK        0x4000
#define CPU_EFLAGS_FLAG_MODE               0x8000
#define CPU_EFLAGS_FLAG_RESUME             0x10000
#define CPU_EFLAGS_FLAG_VIRTUAL_8086       0x20000
#define CPU_EFLAGS_FLAG_ALIGNMENT_CHECK    0x40000
#define CPU_EFLAGS_FLAG_VINTERRUPT_FLAG    0x80000
#define CPU_EFLAGS_FLAG_VINTERRUPT_PENDING 0x100000
#define CPU_EFLAGS_FLAG_CPUID_INSTRUCTION  0x200000
#define CPU_EFLAGS_FLAG_AES_SCHEDULE_LOAD  0x40000000
#define CPU_EFLAGS_FLAG_ALTER_INSTRUCTION  0x80000000

#define PROCESS_CREATE_SUCCESS                   0
#define PROCESS_CREATE_FAIL_MAX_PROCESS_EXCEEDED 1
#define PROCESS_CREATE_FAIL_INVALID_ENTRYPOINT   2
#define PROCESS_CREATE_FAIL_NOT_ENOUGH_MEMORY    3
#define PROCESS_CREATE_FAIL_FS_READ_FAILURE      4

enum ProcessState{
    Inactive, 
    Running, 
    Waiting,
};

struct ProcessInfo {
    char name[8];
    uint32_t pid;
    enum ProcessState state;  
};

extern struct process_state {
    uint32_t active_process_count;
    uint32_t last_pid;
} process_state;

struct Context {
    struct CPURegister cpu; 
    uint32_t eflags;
    uint32_t eip;
    struct PageDirectory *page_directory_virtual_addr; 
};

char* getStateString(enum ProcessState state);

struct ProcessControlBlock{
    struct {
        char process_name[8];
        uint32_t pid; 
        enum ProcessState state;
    } metadata;
    struct Context context;
    struct{
        void *virtual_addr_used[PROCESS_PAGE_FRAME_COUNT_MAX];
        uint32_t page_frame_used_count;
    } memory;
};

extern struct ProcessControlBlock _process_list[PROCESS_COUNT_MAX];

struct ProcessControlBlock* process_get_current_running_pcb_pointer(void);

int32_t process_create_user_process(struct FAT32DriverRequest *request);

bool process_destroy(uint32_t pid);

void process_get_all_info(struct ProcessInfo* processes, int* count);

#endif