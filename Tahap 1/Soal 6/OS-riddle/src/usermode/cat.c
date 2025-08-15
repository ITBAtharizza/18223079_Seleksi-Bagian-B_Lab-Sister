#include "user-shell.h"
#include "cat.h"
#include "util.h"


void cat(char* val, uint32_t curr_pos){
    if(val == NULL){
        put_chars("cat: Argumen kurang", BIOS_RED); 
        put_char('\n');
        put_chars("cat: cat <nama_file>", BIOS_LIGHT_BLUE);
        put_char('\n');
        return;
    }
    uint32_t search_directory_number = state.current_directory;

    char directories[10][12]; 
    int num_dir; 

    extract_dir(val, directories, &num_dir);
    char true_target[12] = "\0\0\0\0\0\0\0\0\0\0\0\0";
    memcpy(true_target, directories[num_dir - 1], 12);

    char filename[8] = {'\0','\0','\0','\0','\0','\0','\0','\0'};
    char fileext[3] = {'\0','\0','\0'};
    extract_filename(true_target, filename);
    extract_file_extension(true_target, fileext);

    int i = 0;
    if(num_dir > 1){
        while (i < num_dir - 1) {
            updateDirectoryTable(search_directory_number);  

            int entry_index = findEntryName(directories[i]);  
            if (entry_index == -1 || state.curr_dir.table[entry_index].attribute != ATTR_SUBDIRECTORY) {
                put_chars("cat: Invalid Directory", BIOS_RED); 
                put_char('\n');
                return;
            }

            // Update the search_directory_number to the found directory
            search_directory_number = (uint32_t)((state.curr_dir.table[entry_index].cluster_high >> 16) | state.curr_dir.table[entry_index].cluster_low);
            put_char('\n');
            i++;
        }
    } updateDirectoryTable(curr_pos);


    struct ClusterBuffer cl           = {0};
    struct FAT32DriverRequest request = {
        .buf = &cl,
        .name = "\0\0\0\0\0\0\0",
        .ext = "\0\0\0",
        .parent_cluster_number = search_directory_number,
        .buffer_size = 4 * CLUSTER_SIZE,
    };
    memcpy(&(request.name), filename, 8);
    memcpy(&(request.ext), fileext, 3);
    int32_t retcode;

    syscall(READ, (uint32_t) &request, (uint32_t) &retcode, 0x0);
    if(retcode == 0){
        put_chars((char *)&cl, BIOS_BROWN);
    }
    put_char('\n');


}