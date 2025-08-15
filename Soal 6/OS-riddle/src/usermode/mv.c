#include "mv.h"
#include "user-shell.h"
#include "util.h"


void rename(char* target_name, char* target_ext, char directories_src[][12], int directories_src_num ,char directories_dest[][12], uint32_t curr_pos){
    uint32_t search_source_number = state.current_directory;


    int i = 0;
    if(directories_src_num > 1){
        while (i < directories_src_num - 1) {
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
    } updateDirectoryTable(curr_pos);

    char what_to_rename[12] = "\0\0\0\0\0\0\0\0\0\0\0\0"; 
    char filename_rename[8] = "\0\0\0\0\0\0\0\0"; 
    char fileext_rename[3] = "\0\0\0";

    memcpy(what_to_rename, directories_dest[0], 12); 
    extract_filename(what_to_rename, filename_rename); 
    extract_file_extension(what_to_rename, fileext_rename);

    struct ClusterBuffer cl = {0};
    int retcode;
    if(strlen(fileext_rename) != 0){
        /**
         * ini kasus untuk merubah nama file
        */
        struct FAT32DriverRequest request = {
            .buf = &cl,
            .name = "\0\0\0\0\0\0\0",
            .ext = "\0\0\0",
            .parent_cluster_number = search_source_number,
            .buffer_size = CLUSTER_SIZE,
        };
        memcpy(request.name, target_name, 8); 
        memcpy(request.ext, target_ext, 3); 
        syscall(READ, (uint32_t)&request, (uint32_t)&retcode, 0x0);

        memcpy(request.name, filename_rename, 8); 
        memcpy(request.ext, fileext_rename, 3); 
        syscall(WRITE, (uint32_t)&request, (uint32_t)&retcode, 0x0);

        memcpy(request.name, target_name, 8); 
        memcpy(request.ext, target_ext, 3);
        syscall(DELETE, (uint32_t)&request, (uint32_t)&retcode, 0x0);

        if(retcode == 0){
            put_chars("File berhasil di-rename", BIOS_LIGHT_GREEN);
            put_char('\n');
        } 
    }
    else{ 
        /**
         * ini kasus buat renaming folder;
        */
       struct ClusterBuffer cf = {0};
        struct FAT32DriverRequest request = {
            .buf = &cf,
            .name = "\0\0\0\0\0\0\0",
            .parent_cluster_number = search_source_number,
            .buffer_size = 0,
        };
        memcpy(request.name, target_name, 8); 
        syscall(READ_DIRECTORY, (uint32_t)&request, (uint32_t)&retcode, 0x0);
        memcpy(request.name, target_name, 8); 
        syscall(DELETE, (uint32_t)&request, (uint32_t)&retcode, 0x0);
        memcpy(request.name, filename_rename, 8); 
        syscall(WRITE, (uint32_t)&request, (uint32_t)&retcode, 0x0);

        if(retcode == 0){
            put_chars("Folder berhasil di-rename", BIOS_LIGHT_GREEN);
            put_char('\n');
        } else if (retcode == 1){
            memcpy(request.name, target_name, 8); 
            syscall(WRITE, (uint32_t)&request, (uint32_t)&retcode, 0x0);
            put_chars("Folder sudah ada nama yang sama", BIOS_RED);
            put_char('\n');

        }
    }
}

void move(char directories_src[10][12], int num_dir_src , char directories_dest[10][12], int num_dir_dest, uint32_t curr_pos){
    uint32_t search_source_number = state.current_directory;
    uint32_t search_target_number = state.current_directory; 

    bool src_status = false; 
    bool target_status = false;    
    bool read_status = false;


    char target[12] = "\0\0\0\0\0\0\0\0\0\0\0\0";
    memcpy(target, directories_src[num_dir_src - 1], 12); 
    // put_chars(directories_src[num_dir_src - 1],);
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
    while (i < num_dir_dest) {
        updateDirectoryTable(search_target_number);  

        int entry_index = findEntryName(directories_dest[i]);  
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

    int retcode;
    int size; 
    if(file){
        size = CLUSTER_SIZE;  //request untuk 
    }else size = 0;

    struct ClusterBuffer cl           = {0};
    struct FAT32DriverRequest req = {
        .buf = &cl,
        .name = "\0\0\0\0\0\0\0\0",
        .ext = "\0\0\0",
	    .parent_cluster_number = search_source_number,
        .buffer_size = size,
    };
    memcpy(req.name, name, 8);
    if(file){
        memcpy(req.ext, ext, 3);
        syscall(READ, (uint32_t)&req, (uint32_t)&retcode ,0);
        if(retcode != 0){
            put_chars("Pembacaan file gagal", BIOS_RED); 
            put_char('\n');
        }else{
            read_status = true;
        }
    } else{
        syscall(READ_DIRECTORY, (uint32_t)&req, (uint32_t)&retcode ,0);
        if(retcode != 0){
            put_chars("Pembacaan folder gagal", BIOS_RED); 
            put_char('\n');
        }else{
            read_status = true;
        } 
    }


    if(target_status && src_status && read_status){
        req.parent_cluster_number = search_target_number;
        memcpy(req.name, name, 8); 
        if(file){
            memcpy(req.ext, ext, 3);
        }
        syscall(WRITE, (uint32_t)&req, (uint32_t)&retcode, 0);
        if(retcode == 0){
            put_chars("File/Folder berhasil di-move", BIOS_LIGHT_GREEN);
            put_char('\n'); 
            req.parent_cluster_number = search_source_number;
            syscall(DELETE, (uint32_t)&req, (uint32_t)&retcode, 0);
        }else{
            put_chars("File/Folder gagal di-copy", BIOS_RED);
            put_char('\n'); 
        }
    }else if(!target_status){
        put_chars("Ini bukan folder atau folder tidak ada", BIOS_RED); 
        put_char('\n'); 
    } else if(!src_status){
        put_chars("file atau folder yang dipilih salah", BIOS_RED);
        put_char('\n');
    } else{
        put_chars("salah semua lmao", BIOS_RED);
        put_char('\n');
    }

    updateDirectoryTable(curr_pos);
}


void mv(char* arg1, char* arg2, uint32_t curr_pos){
    if(arg1 == NULL ||  arg2){
        put_chars("mv: Argumen kurang", BIOS_RED);
        put_char('\n');
        return;
    }
    char directories_src[10][12]; 
    char directories_target[10][12]; 

    int num_dir_src; 
    int num_dir_target; 

    extract_dir(arg1, directories_src, &num_dir_src); 
    extract_dir(arg2, directories_target, &num_dir_target);

    char target[12] = "\0\0\0\0\0\0\0\0\0\0\0\0";

    memcpy(target, directories_src[num_dir_src - 1], 12); 

    char target_name[8]; 
    char target_ext[3]; 
    extract_filename(target, target_name); 
    extract_file_extension(target, target_ext);
    
    char rename_ext[3]; 
    extract_file_extension(directories_target[num_dir_target-1], rename_ext);

    bool file;
    if(strlen(target_ext) != 0){
        file = true; 
    }else file = false;

    bool renaming;
    if(file && (num_dir_target == 1) && strlen(rename_ext) != 0){
        renaming = true; //renaming file
    }
    if(!file && (num_dir_target == 1) && strlen(rename_ext) == 0){
        renaming = true; //renaming folderk
    }
    if((file || !file) && (num_dir_target != 1)){
        renaming = false; //this_is_MOVE
    }

    if((file && renaming) || (!file && renaming)){
        rename(target_name, target_ext, directories_src, num_dir_src ,directories_target, curr_pos);
    }

    if((file && !renaming) || (!file && !renaming)){
        move(directories_src, num_dir_src, directories_target, num_dir_target ,curr_pos);
    }
}