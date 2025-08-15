#include "header/process/scheduler.h"
#include "header/cpu/portio.h"
#include "stdint.h"
#include "header/stdlib/string.h"
#include "header/process/process.h"

extern struct process_state process_manager_state;
extern int32_t current_running_process_index;

void activate_timer_interrupt(void) {
    __asm__ volatile("cli");
    // Configure PIT firing interval
    uint32_t pit_timer_counter_to_fire = PIT_TIMER_COUNTER;
    out(PIT_COMMAND_REGISTER_PIO, PIT_COMMAND_VALUE);
    out(PIT_CHANNEL_0_DATA_PIO, (uint8_t)(pit_timer_counter_to_fire & 0xFF));
    out(PIT_CHANNEL_0_DATA_PIO, (uint8_t)((pit_timer_counter_to_fire >> 8) & 0xFF));

    // Enable timer interrupt
    out(PIC1_DATA, in(PIC1_DATA) & ~(1 << IRQ_TIMER));
}

void scheduler_init(void) {
    activate_timer_interrupt();
    process_manager_state.active_process_count = 1;
}

void scheduler_save_context_to_current_running_pcb(struct Context ctx){
    struct ProcessControlBlock* current_pcb = process_get_current_running_pcb_pointer();
    if (current_pcb != NULL) current_pcb->context = ctx;
}

void scheduler_switch_to_next_process(void){
    // Find the index of the next process to run
    int32_t next_index = get_next_process_index();

    // Get the PCB of the next process
    struct ProcessControlBlock* next_process = &(_process_list[next_index]);
    
    // Update the global index for the currently running process
    current_running_process_index = next_index;

    // Switch to the next process's page directory for memory management
    paging_use_page_directory(next_process->context.page_directory_virtual_addr);
    
    // Perform the context switch to the next process
    process_context_switch(next_process->context);
}