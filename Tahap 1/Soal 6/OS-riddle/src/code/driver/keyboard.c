#include "../../header/driver/keyboard.h"
#include "../../header/cpu/portio.h"
#include "../../header/cpu/interrupt.h"
#include "header/text/framebuffer.h"
#include <stdint.h>


struct KeyboardDriverState keyboard_state = {
    .read_extended_mode = false,
		.keyboard_input_on = false,
		.keyboard_buffer = '\0',
		.shift_pressed = false,
		.alt_pressed = false,
		.ctrl_pressed = false,
		.buffer_filled = false, 
    .caps_active = false
};
const char keyboard_scancode_1_to_ascii_map[256] = {
      0, 0x1B, '1', '2', '3', '4', '5', '6',  '7', '8', '9',  '0',  '-', '=', '\b', '\t',
    'q',  'w', 'e', 'r', 't', 'y', 'u', 'i',  'o', 'p', '[',  ']', '\n',   0,  'a',  's',
    'd',  'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',   0, '\\',  'z', 'x',  'c',  'v',
    'b',  'n', 'm', ',', '.', '/',   0, '*',    0, ' ',   0,    0,    0,   0,    0,    0,
      0,    0,   0,   0,   0,   0,   0,   0,    0,   0, '-',    0,    0,   0,  '+',    0,
      0,    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,    0,    0,   0,    0,    0,
      0,    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,    0,    0,   0,    0,    0,
      0,    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,    0,    0,   0,    0,    0,

      0,    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,    0,    0,   0,    0,    0,
      0,    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,    0,    0,   0,    0,    0,
      0,    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,    0,    0,   0,    0,    0,
      0,    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,    0,    0,   0,    0,    0,
      0,    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,    0,    0,   0,    0,    0,
      0,    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,    0,    0,   0,    0,    0,
      0,    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,    0,    0,   0,    0,    0,
      0,    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,    0,    0,   0,    0,    0,
};


const char keyboard_scancode_1_to_ascii_map_shift_pressed[256] = {
    0,   0x1B, '!',  '@', '#', '$', '%', '^', '&', '*', '(', ')', '_',
    '+', '\b', '\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P',
    '{', '}',  '\n', 0, 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L',
    ':', '"',  '~',  0,   '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<',
    '>', '?',  0,    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,    0,    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,    0,    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,    0,    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,    0,    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,    0,    0,   0,   0,   0,   0,   0,   0,   0,

    0,   0,    0,    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,    0,    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,    0,    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,    0,    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,    0,    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,    0,    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,    0,    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,    0,    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,    0,    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,    0,    0,   0,   0,   0,   0,   0,   0,   0,
};

const char keyboard_scancode_1_to_ascii_map_caps[256] = {
      0, 0x1B, '1', '2', '3', '4', '5', '6',  '7', '8', '9',  '0',  '-', '=', '\b', '\t',
      'Q',  'W', 'E', 'R', 'T', 'Y', 'U', 'I',  'O', 'P', '[',  ']', '\n',   0,  'A',  'S',
      'D',  'F', 'G', 'H', 'J', 'K', 'L', ';', '\'', '`',   0, '\\',  'Z', 'X',  'C',  'V',
      'B',  'N', 'M', ',', '.', '/',   0, '*',    0, ' ',   0,    0,    0,   0,    0,    0,
      0,    0,   0,   0,   0,   0,   0,   0,    0,   0, '-',    0,    0,   0,  '+',    0,
      0,    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,    0,    0,   0,    0,    0,
      0,    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,    0,    0,   0,    0,    0,
      0,    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,    0,    0,   0,    0,    0,

      0,    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,    0,    0,   0,    0,    0,
      0,    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,    0,    0,   0,    0,    0,
      0,    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,    0,    0,   0,    0,    0,
      0,    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,    0,    0,   0,    0,    0,
      0,    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,    0,    0,   0,    0,    0,
      0,    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,    0,    0,   0,    0,    0,
      0,    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,    0,    0,   0,    0,    0,
      0,    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,    0,    0,   0,    0,    0,
 
};

void keyboard_isr(void) {
  uint8_t scancode = in(KEYBOARD_DATA_PORT);
	pic_ack(IRQ_KEYBOARD + PIC1_OFFSET);

  if(!keyboard_state.keyboard_input_on) return;
  else{
    switch (scancode){
      case EXT_SCANCODE_LEFT_SHIFT + BUTTON_PRESSED:
      case EXT_SCANCODE_RIGHT_SHIFT + BUTTON_PRESSED: 
        keyboard_state.shift_pressed = true;
        break;
      case EXT_SCANCODE_ALT + BUTTON_PRESSED: 
        keyboard_state.alt_pressed = true;
        break; 
      
      case EXT_SCANCODE_CTRL + BUTTON_PRESSED: 
        keyboard_state.ctrl_pressed = true;
        break;
      case EXT_SCANCODE_CAPSLOCK + BUTTON_PRESSED:
        if (!keyboard_state.caps_active) {
            keyboard_state.caps_active = true;
        } else {
            keyboard_state.caps_active = false;
        }
        break;
      case EXT_SCANCODE_ALT + BUTTON_RELEASED: 
        keyboard_state.alt_pressed = false; 
        break;
      case EXT_SCANCODE_CTRL + BUTTON_RELEASED: 
        keyboard_state.ctrl_pressed = false;
        break;
      case EXT_SCANCODE_LEFT_SHIFT + BUTTON_RELEASED:
      case EXT_SCANCODE_RIGHT_SHIFT + BUTTON_RELEASED: 
        keyboard_state.shift_pressed = false;
        break;

      case (uint8_t)0xE048:
            framebuffer_scroll_up(); 
            break;

      case (uint8_t) 0xE050:
            framebuffer_scroll_down(); 
          break;

      default: {
        char c;
        if (keyboard_state.caps_active){
          c = keyboard_scancode_1_to_ascii_map_caps[scancode];
        }
        else{
          if (keyboard_state.shift_pressed){
            c = keyboard_scancode_1_to_ascii_map_shift_pressed[scancode];
          }
          else{
            c = keyboard_scancode_1_to_ascii_map[scancode];
          } 
        }
        
        if (scancode & BUTTON_RELEASED) return;
        keyboard_state.buffer_filled = true;
        keyboard_state.keyboard_buffer = c;

      }
    }
  }
}

void keyboard_state_activate(void){
	keyboard_state.keyboard_input_on = true;
}

void keyboard_state_deactivate(void){
	keyboard_state.keyboard_input_on = false;
}

void get_keyboard_buffer(char *buf){
	*buf = keyboard_state.keyboard_buffer;
  keyboard_state.keyboard_buffer = '\0';
}

