#include "header/process/process.h"
#include "header/memory/paging.h"
#include "header/stdlib/string.h"
#include "header/cpu/gdt.h"

struct process_state process_manager_state = {
    .active_process_count = 0,
    .last_pid = 0,
};

struct ProcessControlBlock _process_list[PROCESS_COUNT_MAX];

uint32_t process_generate_new_pid(void){
    for(int i = 0; i< PROCESS_COUNT_MAX; i++){
        if(_process_list[i].metadata.state == Inactive){
            return i;
        }
    }
    return process_manager_state.last_pid++;
}

uint32_t get_current_pid(){
    return process_manager_state.last_pid;
}

struct ProcessControlBlock* process_get_current_running_pcb_pointer(void) {
    for(int i = 0; i < PROCESS_COUNT_MAX; i++){
        if(_process_list[i].metadata.state == Running){
            return &_process_list[i];
        }
    }
    
    return NULL;
}

char* getStateString(enum ProcessState state) {
    switch (state) {
        case Inactive: return "Inactive";
        case Running: return "Running";
        case Waiting: return "Waiting";
        default: return "Unknown";
    }
}

int32_t process_list_get_inactive_index(){
    for(int i = 0; i < PROCESS_COUNT_MAX; i++){
        if(_process_list[i].metadata.state == Inactive) return i;
    }
    return -1;
}

int32_t process_create_user_process(struct FAT32DriverRequest request) {
    int32_t retcode = PROCESS_CREATE_SUCCESS; 
    if (process_manager_state.active_process_count >= PROCESS_COUNT_MAX) {
        retcode = PROCESS_CREATE_FAIL_MAX_PROCESS_EXCEEDED;
        goto exit_cleanup;
    }

    if ((uint32_t) request.buf >= KERNEL_VIRTUAL_ADDRESS_BASE) {
        retcode = PROCESS_CREATE_FAIL_INVALID_ENTRYPOINT;
        goto exit_cleanup;
    }

    int32_t p_index = process_list_get_inactive_index();
    if (p_index == -1) {
        retcode = PROCESS_CREATE_FAIL_MAX_PROCESS_EXCEEDED;
        goto exit_cleanup;
    }
    uint32_t page_frame_count_needed = (request.buffer_size + PAGE_FRAME_SIZE +  PAGE_FRAME_SIZE)/PAGE_FRAME_SIZE;
    if (!paging_allocate_check(page_frame_count_needed) || page_frame_count_needed > PROCESS_PAGE_FRAME_COUNT_MAX) {
        retcode = PROCESS_CREATE_FAIL_NOT_ENOUGH_MEMORY;
        goto exit_cleanup;
    }

    struct ProcessControlBlock *new_pcb = &(_process_list[p_index]);
    memcpy(new_pcb->metadata.process_name, request.name, 8);
    new_pcb->metadata.pid = process_generate_new_pid();
    new_pcb->metadata.state = Waiting;

    struct PageDirectory *page_directory = paging_create_new_page_directory();
    if (page_directory == NULL) {
        retcode = PROCESS_CREATE_FAIL_NOT_ENOUGH_MEMORY;
        goto exit_cleanup;
    }

    new_pcb->context.page_directory_virtual_addr = page_directory;

    struct PageDirectory *current_page = paging_get_current_page_directory_addr();

    void *program_base_address = request.buf;
    if(!paging_allocate_user_page_frame(page_directory, program_base_address)) {
        retcode = PROCESS_CREATE_FAIL_NOT_ENOUGH_MEMORY;
        goto exit_cleanup;
    }
    
    paging_use_page_directory(page_directory);

    int8_t result_read = read(request);
    if(result_read != 0){
        memset(new_pcb, 0, sizeof(struct ProcessControlBlock));
        paging_use_page_directory(current_page);
        retcode = PROCESS_CREATE_FAIL_FS_READ_FAILURE;
        goto exit_cleanup;
    }
    paging_use_page_directory(current_page);

    new_pcb->memory.virtual_addr_used[0] = program_base_address;
    new_pcb->memory.page_frame_used_count++;

    struct CPURegister *cpu = &(new_pcb->context.cpu);
    cpu->index.edi = 0;
    cpu->index.esi = 0; 

    cpu->stack.ebp = 0; 
    cpu->stack.esp = 0x400000-4;

    cpu->general.ebx = 0; 
    cpu->general.edx = 0; 
    cpu->general.ecx = 0; 
    cpu->general.eax = 0; 

    uint32_t segment_val = GDT_USER_DATA_SEGMENT_SELECTOR | 0x3;
    cpu->segment.gs = segment_val;
    cpu->segment.fs = segment_val;
    cpu->segment.es = segment_val; 
    cpu->segment.ds = segment_val;

    new_pcb->context.eflags |= CPU_EFLAGS_BASE_FLAG | CPU_EFLAGS_FLAG_INTERRUPT_ENABLE; 
    new_pcb->context.eip = 0; 

   process_manager_state.active_process_count++;

exit_cleanup:
    return retcode;
}

bool process_destroy(uint32_t pid){
    for(int i = 0; i < PROCESS_COUNT_MAX; i++){
        if(_process_list[i].metadata.pid == pid){
            memset(&_process_list[i], 0, sizeof(struct ProcessControlBlock));
            _process_list[i].metadata.state = Inactive;
            process_manager_state.active_process_count--;
            return true;
        }
    }
    return false;
}