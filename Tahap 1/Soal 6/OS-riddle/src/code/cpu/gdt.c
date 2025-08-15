#include "../../header/cpu/gdt.h"
#include "../../header/cpu/interrupt.h"

static struct GlobalDescriptorTable global_descriptor_table = {
    // Implementasi bagian ini
    .table = {
        {0},
        {0},
        {0},
        {0},
        {0},
        {0},
        {0}
    }
};

struct GDTR _gdt_gdtr = {
    .size = sizeof(global_descriptor_table),
    .address = &global_descriptor_table
};

void gdt_install_tss(void) {
    // Implementasi bagian ini
}
