#include "kill.h"
#include "util.h"
#include "user-shell.h"






void kill(char* arg){
    if(arg == NULL){
        put_chars("kill: argumen kurang\n", BIOS_RED);
        return;
    }

    if(!is_number(arg)) {
        put_chars("kill: masukkan pid bukan string \n", BIOS_RED);
        return;
    }

    int arg_as_uint32 = my_atoi(arg);
    struct ProcessInfo list_process[PROCESS_COUNT_MAX];
    int total;
    syscall(17, (uint32_t)&list_process, (uint32_t)&total, 0);

    bool found = false; 
    int index_process;
    for(int i = 0; i < total; i++){
        if(list_process[i].pid == (uint32_t)arg_as_uint32){
            found = true;
            index_process = i;
            break;
        }
    }

    if (arg_as_uint32 < 1) {
        put_chars("kill: pid harus lebih dari 1\n", BIOS_RED);
        return;
    }

    if(found){
        int retval;
        syscall(KILL, (uint32_t)arg_as_uint32, (uint32_t)&retval, 0);
        syscall(CLEAR, 0, 0, 0);
        if(retval == 0){
            put_chars(list_process[index_process].name, BIOS_RED);
            put_char('\n');
            put_chars("Berhasil di-terminate", BIOS_RED);
            put_char('\n');
        }
    }else{
        put_chars("exec: kill apa desu?\n", BIOS_RED);
    }
}