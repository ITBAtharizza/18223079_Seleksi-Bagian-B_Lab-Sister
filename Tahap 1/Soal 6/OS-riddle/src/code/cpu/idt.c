#include "../../header/cpu/gdt.h"
#include "../../header/cpu/idt.h"
#include <stdint.h>

struct InterruptDescriptorTable interrupt_descriptor_table;

struct IDTR _idt_idtr = {
    .size = sizeof(interrupt_descriptor_table), 
    .address = &interrupt_descriptor_table
};

void set_interrupt_gate(
    uint8_t  int_vector, 
    void     *handler_address, 
    uint16_t gdt_seg_selector, 
    uint8_t  privilege
){
    struct IDTGate *idt_int_gate = &interrupt_descriptor_table.table[int_vector];
    uint32_t addrHandler = (uint32_t) handler_address;
    idt_int_gate->offset_low = (uint16_t)(addrHandler & 0xFFFF);
    idt_int_gate->segment = gdt_seg_selector;
    idt_int_gate->zero_interrupt = INTERRUPT_GATE_R_BIT_1;
    idt_int_gate->gate_type_interrupt = INTERRUPT_GATE_TYPE_AND_SIZE; 
    idt_int_gate->descriptor_privillege_level= privilege;
    idt_int_gate->segment_present_interrupt = 1; 
    idt_int_gate->offset_high = (uint16_t)((addrHandler >> 16) & 0xFFFF);
}

void initialize_idt(void) {
    for(int i = 0; i < ISR_STUB_TABLE_LIMIT; i++){
        int privilege = 0; 
        if(i == 0x30) privilege = 0x3;
        set_interrupt_gate(i, isr_stub_table[i], GDT_KERNEL_CODE_SEGMENT_SELECTOR, privilege);
    }
    __asm__ volatile("lidt %0" : : "m"(_idt_idtr));
    __asm__ volatile("sti");
}


