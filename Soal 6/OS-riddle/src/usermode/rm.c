#include <stdint.h>
#include "./rm.h"
#include "./user-shell.h"
#include "./util.h"


void remove(char* target, uint32_t curr_pos){
    uint32_t search_directory_number = state.current_directory;
    if (target == NULL) {
        return;  
    }

    char directories[10][12]; 
    int num_dir; 

    extract_dir(target, directories, &num_dir);
    char true_target[12] = "\0\0\0\0\0\0\0\0\0\0\0\0"; 
    memcpy(true_target, directories[num_dir - 1], 12);


    char name[8]; 
    char ext[3]; 
    extract_filename(true_target, name);
    extract_file_extension(true_target, ext);


    int i = 0;
    if(num_dir > 1){
        while (i < num_dir - 1) {
            updateDirectoryTable(search_directory_number);  

            int entry_index = findEntryName(directories[i]);  
            if (entry_index == -1 || state.curr_dir.table[entry_index].attribute != ATTR_SUBDIRECTORY) {
                put_chars("Invalid directory path", BIOS_RED);
                put_char('\n');
                return;
            }

            // Update the search_directory_number to the found directory
            search_directory_number = (uint32_t)((state.curr_dir.table[entry_index].cluster_high >> 16) | state.curr_dir.table[entry_index].cluster_low);
            put_char('\n');
            i++;
        }
    } updateDirectoryTable(curr_pos);

    struct ClusterBuffer cl = {0}; 
    struct FAT32DriverRequest req = {
        .buf = &cl,
        .name = "\0\0\0\0\0\0\0\0",
        .ext = "\0\0\0",
	    .parent_cluster_number = search_directory_number,
        .buffer_size = 0,
    };
    memcpy(req.name, name, 8);
    memcpy(req.ext, ext, 3);
    
    // syscall(PUT_CHARS, (uint32_t)req.name, strlen(req.name), 0);
    // syscall(PUT_CHARS, (uint32_t)req.ext, strlen(req.ext), 0);
    int8_t ret;
    syscall(DELETE, (uint32_t)&req, (uint32_t)&ret, 0); 
    if (ret == 0){
        put_chars("rm: berhasil\n", BIOS_LIGHT_GREEN);
    }
    else if (ret == 1){
        put_chars("rm: entry tidak ditemukan\n", BIOS_RED);
    }
    else if (ret == 2){
        put_chars("rm: folder yang akan dihapus tidak kosong\n", BIOS_RED);
    }  
    else{
        put_chars("rm : nt (unknown error)\n", BIOS_RED);
    } 

}
