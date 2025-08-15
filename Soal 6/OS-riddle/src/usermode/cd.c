#include "cd.h"
#include "user-shell.h"
#include "util.h"


void cd(char* dir) {
    uint32_t search_directory_number = state.current_directory;
    // put_chars(dir);
    if (dir == NULL) {
        put_chars("cd: Argumen kurang", BIOS_RED); 
        put_char('\n');
        put_chars("cd: Contoh penggunaan: cd folder1/folder2", BIOS_LIGHT_GREEN); 
        put_char('\n');
        return;  // No directory specified
    }

    char directories[10][12]; 
    int num_dir; 

    extract_dir(dir, directories, &num_dir);
    int i = 0;

    while (i < num_dir) {
        updateDirectoryTable(search_directory_number);  


        int entry_index = findEntryName(directories[i]);  
        if (entry_index == -1 || state.curr_dir.table[entry_index].attribute != ATTR_SUBDIRECTORY) {
                put_chars("cp: Invalid directory path", BIOS_RED);
                put_char('\n');
            return;


        }

        // Update the search_directory_number to the found directory
        search_directory_number = (uint32_t)((state.curr_dir.table[entry_index].cluster_high >> 16) | state.curr_dir.table[entry_index].cluster_low);
        i++;
    }

    // Update the current directory in the shell state
    state.current_directory = search_directory_number;
    updateDirectoryTable(search_directory_number);

    put_char('\n');
}
