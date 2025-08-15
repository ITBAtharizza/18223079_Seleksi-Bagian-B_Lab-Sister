#include "../../header/cpu/portio.h"
#include "../../header/cpu/interrupt.h"
#include "../../header/driver/keyboard.h"
#include "../../header/text/framebuffer.h"
#include "../../header/filesystem/fat32.h"
#include "../../header/cpu/gdt.h"
#include "../../header/text/terminaltext.h"
#include "../../header/process/scheduler.h"
#include "../../header/stdlib/string.h"

void activate_keyboard_interrupt(void) {
    out(PIC1_DATA, in(PIC1_DATA) & ~(1 << IRQ_KEYBOARD));
}

void io_wait(void) {
    out(0x80, 0);
}

void pic_ack(uint8_t irq) {
    if (irq >= 8) out(PIC2_COMMAND, PIC_ACK);
    out(PIC1_COMMAND, PIC_ACK);
}

void pic_remap(void) {
    out(PIC1_COMMAND, ICW1_INIT | ICW1_ICW4); 
    io_wait();
    out(PIC2_COMMAND, ICW1_INIT | ICW1_ICW4);
    io_wait();
    out(PIC1_DATA, PIC1_OFFSET); 
    io_wait();
    out(PIC2_DATA, PIC2_OFFSET); 
    io_wait();
    out(PIC1_DATA, 0b0100); 
    io_wait();
    out(PIC2_DATA, 0b0010); 
    io_wait();

    out(PIC1_DATA, ICW4_8086);
    io_wait();
    out(PIC2_DATA, ICW4_8086);
    io_wait();

    out(PIC1_DATA, PIC_DISABLE_ALL_MASK);
    out(PIC2_DATA, PIC_DISABLE_ALL_MASK);
}

// Syscall handler for clearing the framebuffer.
void syscall_clear_screen(void) {
    framebuffer_clear();
    framebuffer_state.cur_col = 0;
    framebuffer_state.cur_row = 0;
}

// Syscall handler for reading the RTC.
void syscall_get_time(uint32_t ebx, uint32_t ecx, uint32_t edx) {
    uint8_t hour, minute, second;
    read_rtc(&hour, &minute, &second);
    
    // Return time values to user-space via pointers
    *((uint8_t*)ebx) = hour;
    *((uint8_t*)ecx) = minute;
    *((uint8_t*)edx) = second;
}


void syscall(struct InterruptFrame frame) {
    // Extract syscall number and arguments from registers for clarity
    uint32_t syscall_num = frame.cpu.general.eax;
    uint32_t arg1        = frame.cpu.general.ebx;
    uint32_t arg2        = frame.cpu.general.ecx;
    uint32_t arg3        = frame.cpu.general.edx;

    switch (syscall_num) {
        case 0:  *((int8_t*)arg2) = read(*(struct FAT32DriverRequest*)arg1); break;
        case 1:  *((int8_t*)arg2) = read_directory(*(struct FAT32DriverRequest*)arg1); break;
        case 2:  *((int8_t*)arg2) = write(*(struct FAT32DriverRequest*)arg1); break;
        case 3:  *((int8_t*)arg2) = delete(*(struct FAT32DriverRequest*)arg1); break;
        case 4:  get_keyboard_buffer((char*)arg1, (int32_t*)arg2); break;
        case 5:  putchar((char)arg1, arg2); break;
        case 6:  puts((char*)arg1, arg2, arg3); break;
        case 7:  keyboard_state_activate(); break;
        case 8:  *((uint32_t*)arg2) = move_to_child_directory(*(struct FAT32DriverRequest*)arg1); break;
        case 9:  *((uint32_t*)arg2) = move_to_parent_directory(*(struct FAT32DriverRequest*)arg1); break;
        case 10: list_dir_content((char*)arg1, arg2); break;
        case 11: print((char*)arg1, arg2); break;
        case 12: print_path_to_dir((char*)arg1, arg2, (char*)arg3); break;
        case 13: syscall_clear_screen(); break;
        case 14: process_destroy(arg1); break;
        case 15: process_create_user_process(*(struct FAT32DriverRequest*)arg1); break;
        case 16: ps((char*)arg1); break;
        case 17: syscall_get_time(arg1, arg2, arg3); break;
        case 18: *((int8_t*)arg2) = move_dir(*(struct FAT32DriverRequest*)arg1, *(struct FAT32DriverRequest*)arg2); break;
    }
}

void main_interrupt_handler(struct InterruptFrame frame) {
    switch (frame.int_number) {
        case IRQ_KEYBOARD + PIC1_OFFSET:
            keyboard_isr();
            break;
        case SYSCALL_CALL: 
            syscall(frame);
            break;
        case IRQ_TIMER + PIC1_OFFSET:
            struct Context ctx;
            ctx.cpu = frame.cpu;
            ctx.eflags = frame.int_stack.eflags;
            ctx.eip = frame.int_stack.eip;
            ctx.page_directory_virtual_addr = process_get_current_running_pcb_pointer()->context.page_directory_virtual_addr;
            scheduler_save_context_to_current_running_pcb(ctx);
            pic_ack(IRQ_TIMER);
            scheduler_switch_to_next_process();
            break;
        case 0xe: 
            uint32_t current_process = process_get_current_running_pcb_pointer()->metadata.pid;
            pic_ack(IRQ_TIMER); 
            scheduler_switch_to_next_process();
            process_destroy(current_process);
            break;
        case 0xd:    
            uint32_t current_process_to_kill = process_get_current_running_pcb_pointer()->metadata.pid;
            pic_ack(IRQ_TIMER); 
            scheduler_switch_to_next_process();
            process_destroy(current_process_to_kill);
            break;
    }
}

struct TSSEntry _interrupt_tss_entry = {
    .ss0  = GDT_KERNEL_DATA_SEGMENT_SELECTOR,
};

void set_tss_kernel_current_stack(void) {
    uint32_t stack_ptr;
    __asm__ volatile ("mov %%ebp, %0": "=r"(stack_ptr) : /* <Empty> */);
    _interrupt_tss_entry.esp0 = stack_ptr + 8; 
}
