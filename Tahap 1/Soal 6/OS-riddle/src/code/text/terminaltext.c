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
