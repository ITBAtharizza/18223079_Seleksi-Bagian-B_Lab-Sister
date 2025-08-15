#ifndef _CMOS_H 
#define _CMOS_H

#define CURRENT_YEAR        2025

extern unsigned char second;
extern unsigned char minute;
extern unsigned char hour;
extern unsigned char day;
extern unsigned char month;
extern unsigned int year;

void out_byte(int port, int value);
int in_byte(int port);
 
int get_update_in_progress_flag();
 
unsigned char get_RTC_register(int reg);

void read_rtc(unsigned char *hour_ptr, unsigned char *minute_ptr, unsigned char *second_ptr);

#endif 