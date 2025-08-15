#include "header/filesystem/fat32.h"
#include "header/stdlib/string.h"
#include "header/text/framebuffer.h"
#include <stdint.h>
#include <stdbool.h>

static struct FAT32DriverState fat32_driver_state = {0};

const uint8_t fs_signature[BLOCK_SIZE] = {
    'S', 'e', 'l', 'e', 'k', 's', 'i', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',  ' ',
    'L', 'a', 'b', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',  ' ',
    'S', 'i', 's', 't', 'e', 'r', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',  ' ',
    'M', 'a', 'd', 'e', ' ', 'w', 'i', 't', 'h', ' ', 't', 'e', 'a', 'r', ' ',  ' ',
    '-', '-', '-', '-', '-', '-', '-', '-', '-', '-', '-', '2', '0', '2', '5', '\n',
    [BLOCK_SIZE-2] = 'O',
    [BLOCK_SIZE-1] = 'k',
};

uint32_t cluster_to_lba(uint32_t cluster){
    return 0;
}

void init_directory_table(struct FAT32DirectoryTable *dir_table, char *name, uint32_t parent_dir_cluster) {
    // implementasi bagian ini
}

void read_clusters(void *ptr, uint32_t cluster_number, uint8_t cluster_count){
    // implementasi bagian ini
}

void write_clusters(const void *ptr, uint32_t cluster_number, uint8_t cluster_count){
    // implementasi bagian ini
}

void create_fat32(void){
    // implementasi bagian ini
}

bool is_empty_storage(void){
    // implementasi bagian ini
    return false;
}

void initialize_filesystem_fat32(void){
    // implementasi bagian ini
}

bool get_dir_table_from_cluster(uint32_t cluster, struct FAT32DirectoryTable *dir_entry){
    // implementasi bagian ini
    return false;
}

int8_t read_directory(struct FAT32DriverRequest request){
    // implementasi bagian ini
    return -1;
}

int8_t read(struct FAT32DriverRequest request){
    struct FAT32DirectoryTable *dir_table = &fat32_driver_state.dir_table_buf; 
    bool isParentValid = get_dir_table_from_cluster(request.parent_cluster_number, dir_table);
    if (!isParentValid){
        return -1;
    }
    bool found = false;
    int i;
    int idx;
    int j = 0;

    read_clusters(dir_table, request.parent_cluster_number, 1);
    for (i=0; i<TOTAL_DIRECTORY_ENTRY; i++){
        if (dir_table->table[i].user_attribute == UATTR_NOT_EMPTY
        && strcmp(dir_table->table[i].name, request.name, 8) == 0
        && strcmp(dir_table->table[i].ext, request.ext, 3) == 0){
            if(dir_table->table[i].attribute == ATTR_SUBDIRECTORY){
                return 1;
            }
            if(request.buffer_size < dir_table->table[i].filesize){
                return -1;
            }
            found = true;
            idx = i;
            break;
        }
    }

    if(!found) return 2;
    struct FAT32FileAllocationTable *fat_table = &fat32_driver_state.fat_table;
    uint32_t cluster_number = dir_table->table[idx].cluster_low + (((uint32_t)dir_table->table[idx].cluster_high) >> 16);
    while (cluster_number != FAT32_FAT_END_OF_FILE){
        read_clusters(request.buf+CLUSTER_SIZE*j, cluster_number, 1);
        cluster_number = fat_table->cluster_map[cluster_number];
        j++;
    }
    return 0;
}

int8_t write(struct FAT32DriverRequest request){
    bool isFolder = (request.buffer_size == 0 && strlen(request.ext) == 0);
    struct FAT32DirectoryTable *dir_table = &fat32_driver_state.dir_table_buf; 
    bool isParentValid = get_dir_table_from_cluster(request.parent_cluster_number, dir_table);
    if (!isParentValid){
        return 2;
    }

    bool found = false;
    int i;

    for(i=2; i<TOTAL_DIRECTORY_ENTRY; i++){

        if( fat32_driver_state.dir_table_buf.table[i].user_attribute == UATTR_NOT_EMPTY
        && 
        memcmp(fat32_driver_state.dir_table_buf.table[i].name, request.name, 8) == 0
        && 
        (isFolder 
        || memcmp(fat32_driver_state.dir_table_buf.table[i].ext, request.ext, 3) == 0
        )
        ){
            found = true;
            break;
        }
    }
    if (found) return 1;

    int idx_empty_entry = -1;
    for(i = 0; i < TOTAL_DIRECTORY_ENTRY; i++){
        if(fat32_driver_state.dir_table_buf.table[i].user_attribute != UATTR_NOT_EMPTY){
            idx_empty_entry = i;
            break;
        }
    }

    if(idx_empty_entry == -1){
        return -1;
    }

    uint32_t filesize;
    if(request.buffer_size == 0){filesize = CLUSTER_SIZE;}
    else filesize = request.buffer_size;

    int alloc_cluster = (filesize + CLUSTER_SIZE - 1) / CLUSTER_SIZE;
    uint32_t empty_clusters[alloc_cluster];

    int curr_cluster = 0; 
    int empty_cluster = 0; 
    while(empty_cluster < alloc_cluster && curr_cluster < CLUSTER_MAP_SIZE){
        uint32_t is_cluster_empty = fat32_driver_state.fat_table.cluster_map[curr_cluster];
        if(is_cluster_empty == FAT32_FAT_EMPTY_ENTRY){
            empty_clusters[empty_cluster++] = curr_cluster;
        }
        curr_cluster++;
    }   
    if(empty_cluster < alloc_cluster){
        return -1;
    }

    struct FAT32DirectoryEntry *dir_entry = &fat32_driver_state.dir_table_buf.table[idx_empty_entry];
    dir_entry->user_attribute = 0x0;
    dir_entry->filesize = filesize; 
    if(!isFolder){
        dir_entry->attribute = 0;
    }
    else dir_entry->attribute = ATTR_SUBDIRECTORY;
    dir_entry->user_attribute = UATTR_NOT_EMPTY;
    dir_entry->cluster_low = (uint16_t) empty_clusters[0] & 0xFFFF;
    dir_entry->cluster_high = (uint16_t) (empty_clusters[0] >> 16);
    copyStringWithLength(dir_entry->name, request.name, 8);
    if(!isFolder){
        memcpy(dir_entry->ext, request.ext, 3);
    }
    write_clusters(&fat32_driver_state.dir_table_buf, request.parent_cluster_number, 1);

    struct FAT32DirectoryTable new_table;
    void *ptr = request.buf;
    if(isFolder){
        init_directory_table(&new_table, request.name, request.parent_cluster_number);
        ptr = (void *)&new_table;
    }
     
    for (int i = 0; i < alloc_cluster; i++) {
        uint32_t cluster = empty_clusters[i];   
        uint32_t next_cluster;
        if (i + 1 == alloc_cluster) {
            struct ClusterBuffer buffer;
            next_cluster = FAT32_FAT_END_OF_FILE;
            int current_size = filesize % CLUSTER_SIZE;
            if (current_size != 0) {
                memcpy(&buffer, ptr, current_size);
                memset(&buffer.buf[current_size], 0x00, CLUSTER_SIZE - current_size);
                ptr = (void *)&buffer;
            }
        }
        else{
            next_cluster = empty_clusters[i + 1];
        }
        write_clusters(ptr, cluster, 1);
        ptr += CLUSTER_SIZE;
        fat32_driver_state.fat_table.cluster_map[cluster] = next_cluster;
    }
    write_clusters(fat32_driver_state.fat_table.cluster_map, FAT_CLUSTER_NUMBER, 1);

    return 0;
}

int8_t delete(struct FAT32DriverRequest request){
    struct FAT32DirectoryTable *dir_table = &fat32_driver_state.dir_table_buf;
    bool isParentValid = get_dir_table_from_cluster(request.parent_cluster_number, dir_table);
    if (!isParentValid){
        return -1;
    }

    bool found = false;
    bool isFolder = false;
    uint32_t rc;
    for(rc = 2; rc<TOTAL_DIRECTORY_ENTRY; rc++){
        if(dir_table->table[rc].user_attribute == UATTR_NOT_EMPTY
        && memcmp(dir_table->table[rc].name, request.name, 8) == 0
        && (memcmp(dir_table->table[rc].ext, request.ext, 3) == 0)
        ){
            found = true;
            isFolder = dir_table->table[rc].attribute == ATTR_SUBDIRECTORY;
            break;
        }
    }

    if (!found) return 1;
    else{
        if (isFolder){
            bool isKosong = true;
            uint32_t folderRC = dir_table->table[rc].cluster_low + (((uint32_t)dir_table->table[rc].cluster_high) >> 16);
            get_dir_table_from_cluster(folderRC, dir_table);

            uint32_t i;
            for(i = 2; i<TOTAL_DIRECTORY_ENTRY; i++){
                if(dir_table->table[i].user_attribute == UATTR_NOT_EMPTY
                ){
                    isKosong = false;
                    break;
                }
            }
            get_dir_table_from_cluster(request.parent_cluster_number, dir_table);
            if (!isKosong) return 2;
        }

        struct FAT32FileAllocationTable *fat_table = &fat32_driver_state.fat_table;

        dir_table->table[rc].user_attribute = !UATTR_NOT_EMPTY;
        for (int i=0; i<8; i++){
            dir_table->table[rc].name[i] = '\0';     
        }  
        for (int i=0; i<3; i++){
            dir_table->table[rc].ext[i] = '\0';     
        }  

        struct ClusterBuffer emptyBuffer = {0};
        uint32_t cluster_number = dir_table->table[rc].cluster_low + (((uint32_t)dir_table->table[rc].cluster_high) >> 16);
        uint32_t prev;
        while (fat_table->cluster_map[cluster_number] != FAT32_FAT_END_OF_FILE){
            write_clusters(&emptyBuffer, cluster_number, 1);
            prev = cluster_number;
            cluster_number = fat_table->cluster_map[cluster_number];
            fat_table->cluster_map[prev] = FAT32_FAT_EMPTY_ENTRY;
        }
        write_clusters(&emptyBuffer, cluster_number, 1);
        fat_table->cluster_map[cluster_number] = FAT32_FAT_EMPTY_ENTRY;
        
        write_clusters(fat_table, FAT_CLUSTER_NUMBER, 1);
        write_clusters(dir_table, request.parent_cluster_number, 1);
    }

    return 0;
}

bool cmp_string_with_fixed_length(const char *a, const char *b, int l) {
    return false;
}