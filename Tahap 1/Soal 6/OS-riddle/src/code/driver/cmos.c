#include "header/driver/cmos.h"

void out_byte(int port, int value) {
    // implementasi bagian ini
}

int in_byte(int port) {
    // implementasi bagian ini
    return 0;
}

enum {
      cmos_address = 0x70,
      cmos_data    = 0x71
};
 
int get_update_in_progress_flag() {
    // implementasi bagian ini
    return 0;
}
 
unsigned char get_RTC_register(int reg) {
    // implementasi bagian ini
    return 0;
}

void read_rtc(unsigned char *hour_ptr, unsigned char *minute_ptr, unsigned char *second_ptr) {
    // implementasi bagian ini
}
