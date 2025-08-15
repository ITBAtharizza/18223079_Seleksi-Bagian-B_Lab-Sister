#include "util.h"
#include "user-shell.h"
#include "write.h" 


void writef(char* arg1, char* arg2){

    uint32_t target_dir = state.current_directory; 

    char name[8]; 
    char ext[3]; 

    extract_filename(arg1, name); 
    extract_file_extension(arg1, ext);

    int ret; 
    char buf[200];
    memcpy(buf, arg2, strlen(arg2));
    struct FAT32DriverRequest req = {
        .buf = &buf,
        .name = "\0\0\0\0\0\0\0\0", 
        .ext = "\0\0\0",
        .parent_cluster_number = target_dir,
        .buffer_size = 200, 
    };
    memcpy(req.name, name, 8); 
    memcpy(req.ext, ext, 3); 

    syscall(DELETE, (uint32_t)&req, (uint32_t)&ret, 0); 

    // If the length of arg2 is less than 200, fill the rest with null characters
    syscall(WRITE, (uint32_t)&req, (uint32_t)&ret, 0); 
    if(ret == 0){
        put_chars("write: berhasil dilakukan\n", BIOS_LIGHT_GREEN);
    }else{
        put_chars("write: gagal dilakukan\n", BIOS_RED);
    }
}