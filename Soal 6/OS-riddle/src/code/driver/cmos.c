#include "header/driver/cmos.h"
#include "header/cpu/portio.h"

void out_byte(int port, int value) {
    out((uint16_t)port, (uint8_t)value);
}

int in_byte(int port) {
    return (int)in((uint16_t)port);
}

enum {
      cmos_address = 0x70,
      cmos_data    = 0x71
};

int get_update_in_progress_flag() {
    out_byte(cmos_address, 0x0A);
    return in_byte(cmos_data) & 0x80;
}

unsigned char get_RTC_register(int reg) {
    out_byte(cmos_address, reg);
    return (unsigned char)in_byte(cmos_data);
}

void read_rtc(unsigned char *hour_ptr, unsigned char *minute_ptr, unsigned char *second_ptr) {
    unsigned char prev[3], curr[3], regB;
    int stable = 0;

    // Loop until two consecutive reads match to prevent rollover errors
    while (!stable) {
        // Wait for any update in progress to finish
        while (get_update_in_progress_flag());
        curr[0] = get_RTC_register(0x00); // seconds
        curr[1] = get_RTC_register(0x02); // minutes
        curr[2] = get_RTC_register(0x04); // hours

        // Wait again to ensure no tick happened during the read
        while (get_update_in_progress_flag());
        prev[0] = get_RTC_register(0x00);
        prev[1] = get_RTC_register(0x02);
        prev[2] = get_RTC_register(0x04);

        if ((prev[0] == curr[0]) && (prev[1] == curr[1]) && (prev[2] == curr[2])) {
            stable = 1;
        }
    }

    regB = get_RTC_register(0x0B);

    // If BCD mode is enabled, convert values to binary
    if (!(regB & 0x04)) {
        curr[0] = ((curr[0] >> 4) * 10) + (curr[0] & 0x0F);
        curr[1] = ((curr[1] >> 4) * 10) + (curr[1] & 0x0F);
        curr[2] = ((curr[2] >> 4) * 10) + (curr[2] & 0x0F);
    }

    // If 12-hour mode is enabled and the PM bit is set, convert to 24-hour format
    if (!(regB & 0x02) && (curr[2] & 0x80)) {
        curr[2] = ((curr[2] & 0x7F) + 12) % 24;
    }

    // Copy the final, correct values to the output pointers
    *second_ptr = curr[0];
    *minute_ptr = curr[1];
    *hour_ptr   = curr[2];
}