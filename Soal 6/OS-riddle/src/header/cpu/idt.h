#ifndef _IDT_H
#define _IDT_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#define IDT_MAX_ENTRY_COUNT    256
#define ISR_STUB_TABLE_LIMIT   64
#define INTERRUPT_GATE_R_BIT_1 0b000
#define INTERRUPT_GATE_R_BIT_2 0b0110
#define INTERRUPT_GATE_R_BIT_3 0b0

#define INTERRUPT_GATE_TYPE_AND_SIZE 0b01110

extern void *isr_stub_table[ISR_STUB_TABLE_LIMIT];

extern struct IDTR _idt_idtr;

struct IDTGate {
    uint16_t offset_low;
    uint16_t segment;  

    uint8_t : 5;
    uint8_t zero_interrupt: 3;
    uint8_t gate_type_interrupt: 5;
    uint8_t descriptor_privillege_level: 2;
    uint8_t segment_present_interrupt: 1;
    uint16_t offset_high;

} __attribute__((packed));

struct InterruptDescriptorTable {
    struct IDTGate table[IDT_MAX_ENTRY_COUNT];
} __attribute__((packed));

struct IDTR {
    uint16_t            size; 
    struct InterruptDescriptorTable *address;
} __attribute__((packed));

void set_interrupt_gate(uint8_t int_vector, void *handler_address, uint16_t gdt_seg_selector, uint8_t privilege);

void initialize_idt(void);

#endif