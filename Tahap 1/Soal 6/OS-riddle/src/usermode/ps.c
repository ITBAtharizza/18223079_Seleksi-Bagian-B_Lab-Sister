#include "ps.h"
#include "header/process/process.h"
#include "user-shell.h"
#include "util.h"

char* getStateString(enum ProcessState state) {
    switch (state) {
        case Inactive: return "Inactive";
        case Running: return "Running";
        case Waiting: return "Waiting";
        default: return "Unknown";
    }
}

void convert_and_print_pid(int pid) {
    char pid_str[2];

    if (pid < 0 || pid > 99) {
        return;
    }

    pid_str[0] = (pid / 10) + '0';  
    pid_str[1] = (pid % 10) + '0';  // 

    put_chars(pid_str, BIOS_LIGHT_GREEN);
}

void ps(){
    struct ProcessInfo process_info[PROCESS_COUNT_MAX]; 
    int total;
    syscall(17, (uint32_t)&process_info, (uint32_t)&total, 0);

    
    char *pid_info = "PID:";
    char *pid_name = "Name:";
    char *state_info = "State:";
    
    // Print gathered process information
    for (int i = 0; i < total; i++) {
        // Print PID info
        
        put_chars(pid_info, BIOS_LIGHT_GREEN);
        convert_and_print_pid((int)process_info[i].pid);
        put_char(' ');

        put_chars(pid_name, BIOS_LIGHT_GREEN);
        put_chars(process_info[i].name, BIOS_LIGHT_GREEN);

        put_char(' ');
        put_chars(state_info, BIOS_LIGHT_GREEN);
        char* state_str = getStateString(process_info[i].state);
        put_chars(state_str, BIOS_LIGHT_GREEN);
        put_char('\n');
    }
    put_char('\n');
}