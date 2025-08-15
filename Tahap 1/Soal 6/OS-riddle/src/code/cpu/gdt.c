#include "../../header/cpu/gdt.h"
#include "../../header/cpu/interrupt.h"

static struct GlobalDescriptorTable global_descriptor_table = {
    .table = {
        // Entry 0: Null Descriptor
        [0] = {0},

        // Entry 1: Kernel Code Segment (Ring 0)
        [1] = {
            .segment_low = 0xFFFF,
            .base_low    = 0,
            .base_mid    = 0,
            .type_bit    = 0xA,
            .non_system  = 1,
            .dpl         = 0,
            .present     = 1,
            .limit_high  = 0xF,
            .avl         = 0,
            .l_bit       = 0,
            .db_bit      = 1,
            .g_bit       = 1,
            .base_high   = 0
        },

        // Entry 2: Kernel Data Segment (Ring 0)
        [2] = {
            .segment_low = 0xFFFF,
            .base_low    = 0,
            .base_mid    = 0,
            .type_bit    = 0x2,
            .non_system  = 1,
            .dpl         = 0,
            .present     = 1,
            .limit_high  = 0xF,
            .avl         = 0,
            .l_bit       = 0,
            .db_bit      = 1,
            .g_bit       = 1,
            .base_high   = 0
        },
    }
};

// Update the size for the smaller table
struct GDTR _gdt_gdtr = {
    .size    = (3 * sizeof(struct SegmentDescriptor)) - 1,
    .address = &global_descriptor_table
};

void gdt_install_tss(void) {
    // Implementasi bagian ini
}
