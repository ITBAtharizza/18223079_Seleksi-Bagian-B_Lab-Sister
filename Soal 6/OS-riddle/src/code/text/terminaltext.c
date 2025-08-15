#include "../../header/text/terminaltext.h"
#include "../../header/driver/keyboard.h"
#include "../../header/text/framebuffer.h"

void fputc(char c, uint8_t color){
    if(c == '\n'){
        framebuffer_newline();
    }else{
        framebuffer_put(c, color);
    }
}

void terminal_puts(char* s, uint32_t len, uint8_t color) {
    for (uint32_t i = 0; i < len; ++i) {
        fputc(s[i], color);
    }
}

void terminal_readline(char* buf, uint32_t size) {
    uint32_t i = 0;
    char c;
    while (i < size - 1) {
        get_keyboard_buffer(&c);
        if (c != '\0') {
            if (c == '\n') {
                fputc('\n', 0xF);
                break;
            } else if (c == 0x08) { // Backspace
                if (i > 0) {
                    i--;
                    fputc(0x08, 0xF); // Move cursor back
                    fputc(' ', 0xF); // Erase character
                    fputc(0x08, 0xF); // Move cursor back again
                }
            } else {
                buf[i++] = c;
                fputc(c, 0xF);
            }
        }
    }
    buf[i] = '\0';
}