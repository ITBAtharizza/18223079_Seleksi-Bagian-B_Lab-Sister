#include "../../header/cpu/gdt.h"
#include "../../header/cpu/interrupt.h"

static struct GlobalDescriptorTable global_descriptor_table = {
    .table = {
        // [0]: Null Descriptor
        {0},

        // [1]: Kernel Code Descriptor (Ring 0)
        {
            .segment_low  = 0xFFFF,
            .base_low     = 0,
            .base_mid     = 0,
            .type_bit     = 0xA, // Executable, Readable
            .non_system   = 1,
            .dpl          = 0,   // DPL Ring 0
            .present      = 1,
            .limit_high   = 0xF,
            .avl          = 0,
            .l_bit        = 0,
            .db_bit       = 1,   // 32-bit segment
            .g_bit        = 1,   // 4 KiB pages
            .base_high    = 0
        },

        // [2]: Kernel Data Descriptor (Ring 0)
        {
            .segment_low  = 0xFFFF,
            .base_low     = 0,
            .base_mid     = 0,
            .type_bit     = 0x2, // Writable
            .non_system   = 1,
            .dpl          = 0,   // DPL Ring 0
            .present      = 1,
            .limit_high   = 0xF,
            .avl          = 0,
            .l_bit        = 0,
            .db_bit       = 1,   // 32-bit segment
            .g_bit        = 1,   // 4 KiB pages
            .base_high    = 0
        },

        // [3]: User Code Descriptor (Ring 3)
        {
            .segment_low  = 0xFFFF,
            .base_low     = 0,
            .base_mid     = 0,
            .type_bit     = 0xA, // Executable, Readable
            .non_system   = 1,
            .dpl          = 3,   // DPL Ring 3
            .present      = 1,
            .limit_high   = 0xF,
            .avl          = 0,
            .l_bit        = 0,
            .db_bit       = 1,   // 32-bit segment
            .g_bit        = 1,   // 4 KiB pages
            .base_high    = 0
        },

        // [4]: User Data Descriptor (Ring 3)
        {
            .segment_low  = 0xFFFF,
            .base_low     = 0,
            .base_mid     = 0,
            .type_bit     = 0x2, // Writable
            .non_system   = 1,
            .dpl          = 3,   // DPL Ring 3
            .present      = 1,
            .limit_high   = 0xF,
            .avl          = 0,
            .l_bit        = 0,
            .db_bit       = 1,   // 32-bit segment
            .g_bit        = 1,   // 4 KiB pages
            .base_high    = 0
        },

        // [5]: TSS Descriptor
        {
            .segment_low  = sizeof(struct TSSEntry),
            .base_low     = 0,
            .base_mid     = 0,
            .type_bit     = 0x9, // TSS (available 32-bit)
            .non_system   = 0,
            .dpl          = 0,
            .present      = 1,
            .limit_high   = (sizeof(struct TSSEntry) & (0xF << 16)) >> 16,
            .avl          = 0,
            .l_bit        = 0,
            .db_bit       = 1,
            .g_bit        = 0,
            .base_high    = 0
        },

        // Unused entries
        {0}
    }
};

// GDTR pointing to the table
struct GDTR _gdt_gdtr = {
    .size    = sizeof(global_descriptor_table) - 1,
    .address = &global_descriptor_table
};

// Function to set the base address of the TSS descriptor at runtime
void gdt_install_tss(void) {
    uint32_t base = (uint32_t) &_interrupt_tss_entry;
    global_descriptor_table.table[5].base_high = (base & (0xFF << 24)) >> 24;
    global_descriptor_table.table[5].base_mid  = (base & (0xFF << 16)) >> 16;
    global_descriptor_table.table[5].base_low  = base & 0xFFFF;
}