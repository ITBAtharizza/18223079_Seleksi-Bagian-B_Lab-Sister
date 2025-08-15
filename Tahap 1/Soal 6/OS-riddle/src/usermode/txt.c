
#include "util.h"
#include "user-shell.h"
#include "txt.h"


void txt(char* arg){

    uint32_t target_dir = state.current_directory;
    int ret; 
    char buffer[200];
    struct FAT32DriverRequest req = {
        .buf = &buffer,
        .name = "\0\0\0\0\0\0\0\0", 
        .ext = "txt",
        .parent_cluster_number = target_dir,
        .buffer_size = 200, 
    };

    memcpy(req.name, arg, 8);
    syscall(WRITE, (uint32_t)&req, (uint32_t)&ret, 0);
    if(ret == 0){
        put_chars("txt: berhasil ditulis\n", BIOS_LIGHT_GREEN);
    }else{
        put_chars("txt: gagal ditulis\n", BIOS_RED);
    }
}