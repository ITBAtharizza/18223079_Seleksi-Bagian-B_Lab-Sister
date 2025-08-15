#ifndef _GDT_H
#define _GDT_H

#include <stdint.h>

#define GDT_MAX_ENTRY_COUNT 32
#define GDT_KERNEL_CODE_SEGMENT_SELECTOR 0x8
#define GDT_KERNEL_DATA_SEGMENT_SELECTOR 0x10
#define GDT_USER_CODE_SEGMENT_SELECTOR 0x18
#define GDT_USER_DATA_SEGMENT_SELECTOR 0x20
#define GDT_TSS_SELECTOR               0x28

extern struct GDTR _gdt_gdtr;

struct SegmentDescriptor {
    // First 32-bit (Bytes 0-3)
    uint16_t segment_low;  // This is the Limit (bits 0-15)
    uint16_t base_low;     // Base (bits 0-15)

    // Next 16-bit (Bytes 4-5)
    uint8_t base_mid;      // Base (bits 16-23)

    // This is the Access Byte (Byte 5)
    uint8_t type_bit   : 4; // Type fields (A, RW, DC, E)
    uint8_t non_system : 1; // S bit (1 for code/data)
    uint8_t dpl        : 2; // Descriptor Privilege Level
    uint8_t present    : 1; // Present bit

    // Next 16-bit (Bytes 6-7)
    // This byte contains high limit and flags
    uint8_t limit_high : 4; // This is segment_low's upper bits
    uint8_t avl        : 1; // Available bit
    uint8_t l_bit      : 1; // Long mode bit
    uint8_t db_bit     : 1; // D/B bit (size)
    uint8_t g_bit      : 1; // Granularity bit

    uint8_t base_high;     // Base (bits 24-31)

} __attribute__((packed));

struct GlobalDescriptorTable {
    struct SegmentDescriptor table[GDT_MAX_ENTRY_COUNT];
} __attribute__((packed));

struct GDTR {
    uint16_t                     size;
    struct GlobalDescriptorTable *address;
} __attribute__((packed));

void gdt_install_tss(void);

#endif