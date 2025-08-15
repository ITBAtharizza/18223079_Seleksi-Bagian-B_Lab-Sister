#include "user-shell.h"
#include "util.h"
#include "cp.h"

void cp(char* src, char* dest, uint32_t curr_pos){
    if(src == NULL|| dest == NULL){
        put_chars("cp: Argumen kurang", BIOS_RED); 
        put_char('\n'); 
        return;
    }
    uint32_t search_source_number = state.current_directory;
    uint32_t search_target_number = state.current_directory; 

    char directories_src[10][12]; 
    char directories_target[10][12]; 

    bool src_status = false; 
    bool target_status = false;    
    bool read_status = false;
    int num_dir_src; 
    int num_dir_target; 

    extract_dir(src, directories_src, &num_dir_src); 
    extract_dir(dest, directories_target, &num_dir_target);

    char target[12] = "\0\0\0\0\0\0\0\0\0\0\0\0";
    memcpy(target, directories_src[num_dir_src - 1], 12); 
    // put_chars(directories_src[num_dir_src - 1]);
    char name[8]; 
    char ext[3]; 

    extract_filename(target, name); 
    extract_file_extension(target, ext);


    bool file;
    if(strlen(ext) != 0){
        file = true; 
    }else file = false;

    int i = 0; 
    if(num_dir_src == 1 && (strlen(ext) != 0 || strlen(name) != 0)){
        src_status = true;
    }
    if(num_dir_src > 1){
        while (i < num_dir_src - 1) {
            updateDirectoryTable(search_source_number);  

            int entry_index = findEntryName(directories_src[i]);  
            if (entry_index == -1 || state.curr_dir.table[entry_index].attribute != ATTR_SUBDIRECTORY) {
                put_chars("Invalid directory path", BIOS_RED);
                put_char('\n');
                return;
            }

            // Update the search_directory_number to the found directory
            search_source_number = (uint32_t)((state.curr_dir.table[entry_index].cluster_high >> 16) | state.curr_dir.table[entry_index].cluster_low);
            put_char('\n');
            i++;
        }
        src_status = true;
    }

    i = 0; 
    while (i < num_dir_target) {
            updateDirectoryTable(search_target_number);  

            int entry_index = findEntryName(directories_target[i]);  
            if (entry_index == -1 || state.curr_dir.table[entry_index].attribute != ATTR_SUBDIRECTORY) {
                put_chars("Invalid directory path", BIOS_RED);
                put_char('\n');
                return;
            }

            // Update the search_directory_number to the found directory
            search_target_number = (uint32_t)((state.curr_dir.table[entry_index].cluster_high >> 16) | state.curr_dir.table[entry_index].cluster_low);
            put_char('\n');
            i++;
        }
        target_status = true;

    struct ClusterBuffer cl           = {0};
    int retcode_read;
    int size; 
    if(file){
        size = CLUSTER_SIZE; 
    }else size = 0;

    struct FAT32DriverRequest req_read = {
        .buf = &cl,
        .name = "\0\0\0\0\0\0\0\0",
        .ext = "\0\0\0",
	    .parent_cluster_number = search_source_number,
        .buffer_size = size,
    };
    memcpy(req_read.name, name, 8); 
    if(file){
        memcpy(req_read.ext, ext, 3);
        syscall(READ, (uint32_t)&req_read, (uint32_t)&retcode_read ,0);
        if(retcode_read != 0){
            put_chars("Pembacaan file gagal", BIOS_RED); 
            put_char('\n');
        }else{
            read_status = true;
        }
    }else{
        syscall(READ_DIRECTORY, (uint32_t)&req_read, (uint32_t)&retcode_read ,0);
        if(retcode_read != 0){
            put_chars("Pembacaan folder gagal", BIOS_RED); 
            put_char('\n');
        }else{
            read_status = true;
        } 
    }

    if(target_status && src_status && read_status){
        int ret;
        struct FAT32DriverRequest req_write = {
            .buf = &cl,
            .name = "\0\0\0\0\0\0\0\0",
            .ext = "\0\0\0",
            .parent_cluster_number = search_target_number,
            .buffer_size = size, 
        };
        memcpy(req_write.name, name, 8); 
        if(file){
            memcpy(req_write.ext, ext, 3);
        }
        syscall(WRITE, (uint32_t)&req_write, (uint32_t)&ret, 0);
        if(ret == 0){
            put_chars("cp: File/Folder berhasil di-copy", BIOS_LIGHT_GREEN);
            put_char('\n'); 
        }else{
            put_chars("cp: File/Folder gagal di-copy", BIOS_RED);
            put_char('\n'); 
        }
    }else if(!target_status){
        put_chars("cp: Ini bukan folder atau folder tidak ada", BIOS_RED); 
        put_char('\n'); 
    } else if(!src_status){
        put_chars("cp: file atau folder yang dipilih salah", BIOS_RED);
        put_char('\n');
    } else{
        put_chars("cp: salah semua lmao", BIOS_RED);
        put_char('\n');
    }

    updateDirectoryTable(curr_pos);

}