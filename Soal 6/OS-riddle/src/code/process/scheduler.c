#include "header/process/scheduler.h"
#include "header/cpu/portio.h"
#include "stdint.h"
#include "header/stdlib/string.h"
#include "header/cpu/interrupt.h" // For IRQ_TIMER and PIC_OFFSET

#define PIT_CHANNEL_0_DATA_PORT 0x40
#define PIT_COMMAND_PORT        0x43

#define PIT_MODE_SQUARE_WAVE    0x36
#define PIT_FREQUENCY           1193180 // Base frequency of PIT
#define SCHEDULER_FREQUENCY     100     // 100 Hz
#define DEFAULT_TIME_SLICE_TICKS (PIT_FREQUENCY / SCHEDULER_FREQUENCY / SCHEDULER_FREQUENCY) // Adjust based on desired time slice duration

uint32_t _current_running_process_idx;
uint32_t time_slice_ticks_left;

void activate_timer_interrupt(void) {
    uint16_t divisor = PIT_FREQUENCY / SCHEDULER_FREQUENCY;
    out(PIT_COMMAND_PORT, PIT_MODE_SQUARE_WAVE);
    io_wait();
    out(PIT_CHANNEL_0_DATA_PORT, (uint8_t)(divisor & 0xFF));
    io_wait();
    out(PIT_CHANNEL_0_DATA_PORT, (uint8_t)((divisor >> 8) & 0xFF));
    io_wait();
    out(PIC1_DATA, in(PIC1_DATA) & ~(1 << IRQ_TIMER)); // Enable timer interrupt
}

void scheduler_init(void) {
    for (int i = 0; i < PROCESS_COUNT_MAX; ++i) {
        _process_list[i].metadata.state = Inactive;
    }
    _current_running_process_idx = 0;
    _process_list[_current_running_process_idx].metadata.state = Running;
    time_slice_ticks_left = DEFAULT_TIME_SLICE_TICKS;
}

void scheduler_save_context_to_current_running_pcb(struct Context ctx){
    _process_list[_current_running_process_idx].context = ctx;
}

void scheduler_switch_to_next_process(void){
    uint32_t next_process_idx = (_current_running_process_idx + 1) % PROCESS_COUNT_MAX;
    while (_process_list[next_process_idx].metadata.state == Inactive) {
        next_process_idx = (next_process_idx + 1) % PROCESS_COUNT_MAX;
    }

    _process_list[_current_running_process_idx].metadata.state = Waiting; // Mark current as waiting
    _current_running_process_idx = next_process_idx;
    _process_list[_current_running_process_idx].metadata.state = Running; // Mark next as running

    time_slice_ticks_left = DEFAULT_TIME_SLICE_TICKS;

    // Load context of the next process and switch
    process_context_switch(_process_list[_current_running_process_idx].context);
}