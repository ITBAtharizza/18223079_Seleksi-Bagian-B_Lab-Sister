#ifndef TERMINALTEXT_H
#define TERMINALTEXT_H

#include <stdint.h>
char fgetc();
void fputc(char c, uint8_t color);
void terminal_puts(char* s, uint32_t len, uint8_t color);
void terminal_readline(char* buf, uint32_t size);

#endif
