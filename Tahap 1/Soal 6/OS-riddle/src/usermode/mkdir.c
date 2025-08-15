
#include "./mkdir.h"
#include "./util.h"
#include "./user-shell.h"

struct ClusterBuffer cl = {0};
void mkdir(char* dir) {
    if(dir != NULL){
        struct FAT32DriverRequest req = {
            .name = "\0\0\0\0\0\0\0\0",
            .ext = "\0\0\0",
            .buf = &cl,
            .buffer_size = 0,
            .parent_cluster_number = state.current_directory,
        };
        memcpy(req.name, dir, 8);
        int8_t ret;
        syscall(WRITE, (uint32_t)&req, (uint32_t)&ret, 0);
        if(ret == 1){
            put_chars("mkdir: folder sudah ada", BIOS_RED);
            put_char('\n');
        }else if(ret == -1){
            put_chars("mkdir: directory penuh", BIOS_RED );
            put_char('\n');
        }else if(ret == 0){
            put_chars("Folder berhasil dibuat", BIOS_LIGHT_GREEN);
            put_char('\n');
        }
    } else{
        put_chars("mkdir: Argumen kurang", BIOS_RED);
        put_char('\n');
        put_chars("mkdir: mkdir <nama_folder>", BIOS_LIGHT_GREEN);
        put_char('\n');
    }
}