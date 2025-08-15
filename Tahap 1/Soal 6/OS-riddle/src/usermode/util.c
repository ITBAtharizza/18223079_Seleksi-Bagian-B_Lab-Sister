#include "util.h"

void clearBuf(void *pointer, size_t n) {
    uint8_t *ptr       = (uint8_t*) pointer;
    for (size_t i = 0; i < n; i++) {
        ptr[i] = 0x00;
    }
}

int my_atoi(char* str){
    int res = 0;
    for (int i = 0; str[i] != '\0'; ++i)
        res = res * 10 + str[i] - '0';
 
    return res;
}

bool is_number(const char* str){
    int i = 0;
    while (str[i] == ' ' || str[i] == '\t' || str[i] == '\n' || str[i] == '\v' || str[i] == '\f' || str[i] == '\r'){
        i++;
    }
    if (str[i] == '-' || str[i] == '+'){
        i++;
    }
    if (str[i] < '0' || str[i] > '9'){
        return false;
    }
    while (str[i] != '\0'){
        if (str[i] < '0' || str[i] > '9') {
            return false;
        }
        i++;
    }
    return true;
}

int findEntryName(char* name) {
    int result = -1;

    int i = 1;
    bool found = false;
    while (i < TOTAL_DIRECTORY_ENTRY && !found) {
        if (memcmp(state.curr_dir.table[i].name, name, 8) == 0 && 
            state.curr_dir.table[i].user_attribute == UATTR_NOT_EMPTY) {
            result = i;
            found = true;
        }
        else {
            i++;
        }
    }
    return result;
}


void extract_dir(char* dir, char directories[][12], int* numDirs) {
    if (dir == NULL || directories == NULL || numDirs == NULL) {
        return;  // Invalid input parameters
    }

    int len = strlen(dir);
    if (len == 0) {
        *numDirs = 0;
        return;
    }

    int index = 0;
    int start = 0;
    int end = 0;

    // Iterate through the input directory path
    while (end <= len) {
        if (dir[end] == '/' || dir[end] == '\0') {
            // Calculate the length of the directory name
            int dirLen = end - start;

            // Copy the directory name into the directories array
            if (dirLen > 0 && index < 10) {
                memcpy(directories[index], &dir[start], dirLen);
                directories[index][dirLen] = '\0';  // Null-terminate the string
                index++;
            }

            start = end + 1;  // Move start to the next character after '/'
        }
        end++;
    }

    *numDirs = index;  // Set the number of extracted directories
}

void print_curr_dir(struct ShellState s){
    print_curr_dir_helper(s.path_to_print, s.current_directory);
}

void print_curr_dir_helper(char* path_str, uint32_t current_dir) {
    int pathlen = 0;
    int nodecount = 0;
    char nodeIndex [10][64];

    clearBuf(path_str, 128);
    for (int i = 0; i < 10; i++) {
        clearBuf(nodeIndex[i], 64);
    }

    if (current_dir == ROOT_CLUSTER_NUMBER) {
        path_str[pathlen++] = '/';
        put_chars(path_str, BIOS_LIGHT_BLUE);
        return;
    }
    
    uint32_t parent = current_dir;
    path_str[pathlen++] = '/';
    while (parent != ROOT_CLUSTER_NUMBER) {
        updateDirectoryTable(parent);
        parent = (uint32_t) ((state.curr_dir.table[0].cluster_high >> 16) | state.curr_dir.table[0].cluster_low);
        memcpy(nodeIndex[nodecount], state.curr_dir.table[0].name, strlen(state.curr_dir.table[0].name));
        nodecount++;
    }
    updateDirectoryTable(current_dir);

    for (int i = nodecount - 1; i >= 0; i--) {
        for (size_t j = 0; j < strlen(nodeIndex[i]); j++) {
            path_str[pathlen++] = nodeIndex[i][j];
        } 
        
        if (i > 0) {
            path_str[pathlen++] = '/';
        }
    }

    put_chars(path_str, BIOS_LIGHT_BLUE);
}

void put_char(char buf){
    syscall(PUT_CHAR,(uint32_t)buf, 0, 0);
}

void put_chars(char* buf, uint8_t color){
    syscall(PUT_CHARS, (uint32_t)buf, strlen(buf), color);
}

void extract_filename(const char *filename, char *basename) {
    int j;
    int len = strlen(filename);
    for (j = 0; j < len; j++) {
        if(filename[j] != '.') basename[j] = filename[j];
        else break;
    }
    basename[j] = '\0';
}

void extract_file_extension(const char *filename, char *extension) {
    int i, j;
    int len = strlen(filename);
    int dot_found = -1;

    for (i = len - 1; i >= 0; i--) {
        if (filename[i] == '.') {
            dot_found = i;
            break;
        }
    }

    int z = 0;
    if (dot_found != -1) {
        for (j = i + 1; j < len; j++) {
            extension[z++] = filename[j];
        }
    } else {
        extension[0] = '\0';
    }
}

// void extractDirectories(char* dir, char directories[][8], int* numDirs) {
//     if (dir == NULL || directories == NULL || numDirs == NULL) {
//         return;  // Invalid input parameters
//     }

//     int len = strlen(dir);
//     if (len == 0) {
//         *numDirs = 0;
//         return;
//     }

//     int index = 0;
//     int start = 0;
//     int end = 0;

//     // Iterate through the input directory path
//     while (end <= len) {
//         if (dir[end] == '/' || dir[end] == '\0') {
//             // Calculate the length of the directory name
//             int dirLen = end - start;

//             // Copy the directory name into the directories array
//             if (dirLen > 0 && index < 10) {
//                 memcpy(directories[index], &dir[start], dirLen);
//                 directories[index][dirLen] = '\0';  // Null-terminate the string
//                 index++;
//             }

//             start = end + 1;  // Move start to the next character after '/'
//         }
//         end++;
//     }

//     *numDirs = index;  // Set the number of extracted directories
// }
