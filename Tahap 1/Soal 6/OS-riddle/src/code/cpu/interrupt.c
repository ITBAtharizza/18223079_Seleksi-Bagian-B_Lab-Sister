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

void syscall(struct InterruptFrame frame) {
    // implementasi bagian ini
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
