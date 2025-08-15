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
    return 5 + (cluster - ROOT_CLUSTER_NUMBER) * CLUSTER_BLOCK_COUNT;
}

void init_directory_table(struct FAT32DirectoryTable *dir_table, char *name, uint32_t parent_dir_cluster, uint32_t self_cluster_number) {
    uint32_t i;
    for (i = 0; i < TOTAL_DIRECTORY_ENTRY; i++) {
        dir_table->table[i].user_attribute = 0x0;
    }

    // Current directory entry '.'
    copyStringWithLength(dir_table->table[0].name, ".       ", 8);
    copyStringWithLength(dir_table->table[0].ext, "   ", 3);
    dir_table->table[0].attribute = ATTR_SUBDIRECTORY;
    dir_table->table[0].user_attribute = UATTR_NOT_EMPTY;
    dir_table->table[0].cluster_low = (uint16_t) (self_cluster_number & 0xFFFF);
    dir_table->table[0].cluster_high = (uint16_t) (self_cluster_number >> 16);

    // Parent directory entry '..'
    copyStringWithLength(dir_table->table[1].name, "..      ", 8);
    copyStringWithLength(dir_table->table[1].ext, "   ", 3);
    dir_table->table[1].attribute = ATTR_SUBDIRECTORY;
    dir_table->table[1].user_attribute = UATTR_NOT_EMPTY;
    dir_table->table[1].cluster_low = (uint16_t) (parent_dir_cluster & 0xFFFF);
    dir_table->table[1].cluster_high = (uint16_t) (parent_dir_cluster >> 16);
}

void read_clusters(void *ptr, uint32_t cluster_number, uint8_t cluster_count){
    read_blocks(ptr, cluster_to_lba(cluster_number), cluster_count * CLUSTER_BLOCK_COUNT);
}

void write_clusters(const void *ptr, uint32_t cluster_number, uint8_t cluster_count){
    write_blocks(ptr, cluster_to_lba(cluster_number), cluster_count * CLUSTER_BLOCK_COUNT);
}

void create_fat32(void){
    write_blocks(fs_signature, BOOT_SECTOR, 1);

    uint32_t i;
    for(i = 0; i < CLUSTER_MAP_SIZE; i++){
        fat32_driver_state.fat_table.cluster_map[i] = FAT32_FAT_EMPTY_ENTRY;
    }
    fat32_driver_state.fat_table.cluster_map[0] = CLUSTER_0_VALUE;
    fat32_driver_state.fat_table.cluster_map[1] = CLUSTER_1_VALUE;
    fat32_driver_state.fat_table.cluster_map[FAT_CLUSTER_NUMBER] = FAT32_FAT_END_OF_FILE;
    fat32_driver_state.fat_table.cluster_map[ROOT_CLUSTER_NUMBER] = FAT32_FAT_END_OF_FILE;
    write_clusters(&fat32_driver_state.fat_table, FAT_CLUSTER_NUMBER, 1);

    struct FAT32DirectoryTable root_dir_table;
    init_directory_table(&root_dir_table, "        ", ROOT_CLUSTER_NUMBER, ROOT_CLUSTER_NUMBER);
    write_clusters(&root_dir_table, ROOT_CLUSTER_NUMBER, 1);
}

bool is_empty_storage(void){
    struct BlockBuffer boot_sector_buf;
    read_blocks(&boot_sector_buf, BOOT_SECTOR, 1);
    return memcmp(&boot_sector_buf, fs_signature, BLOCK_SIZE) != 0;
}

void initialize_filesystem_fat32(void){
    if(is_empty_storage()){
        create_fat32();
    }
    read_clusters(&fat32_driver_state.fat_table, FAT_CLUSTER_NUMBER, 1);
    read_clusters(&fat32_driver_state.dir_table_buf, ROOT_CLUSTER_NUMBER, 1);
}

bool get_dir_table_from_cluster(uint32_t cluster, struct FAT32DirectoryTable *dir_entry){
    if (cluster >= CLUSTER_MAP_SIZE) {
        return false;
    }
    read_clusters(dir_entry, cluster, 1);
    return true;
}

int8_t read_directory(struct FAT32DriverRequest *request){
    struct FAT32DirectoryTable *dir_table = &fat32_driver_state.dir_table_buf; 
    bool isParentValid = get_dir_table_from_cluster(request->parent_cluster_number, dir_table);
    if (!isParentValid){
        return -1;
    }

    uint32_t count = 0;
    uint32_t i;
    struct FAT32DirectoryEntry *dest_buf = (struct FAT32DirectoryEntry *)request->buf;

    for (i = 0; i < TOTAL_DIRECTORY_ENTRY; i++){
        if (dir_table->table[i].user_attribute == UATTR_NOT_EMPTY) {
            if (count < request->buffer_size) { // request->buffer_size here refers to the max number of entries to copy
                memcpy(&dest_buf[count], &dir_table->table[i], sizeof(struct FAT32DirectoryEntry));
                count++;
            } else {
                // Buffer is full, but continue iterating to count total entries
            }
        }
    }
    return count;
}

int8_t read(struct FAT32DriverRequest *request){
    struct FAT32DirectoryTable *dir_table = &fat32_driver_state.dir_table_buf; 
    bool isParentValid = get_dir_table_from_cluster(request->parent_cluster_number, dir_table);
    if (!isParentValid){
        return -1;
    }
    bool found = false;
    int i;
    int idx;
    int j = 0;

    read_clusters(dir_table, request->parent_cluster_number, 1);
    for (i=0; i<TOTAL_DIRECTORY_ENTRY; i++){
        if (dir_table->table[i].user_attribute == UATTR_NOT_EMPTY
        && strcmp(dir_table->table[i].name, request->name, 8) == 0
        && strcmp(dir_table->table[i].ext, request->ext, 3) == 0){
            if(dir_table->table[i].attribute == ATTR_SUBDIRECTORY){
                return 1;
            }
            if(request->buffer_size < dir_table->table[i].filesize){
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
        read_clusters(request->buf+CLUSTER_SIZE*j, cluster_number, 1);
        cluster_number = fat_table->cluster_map[cluster_number];
        j++;
    }
    return 0;
}

int8_t write(struct FAT32DriverRequest *request){
    bool isFolder = (request->buffer_size == 0 && strlen(request->ext) == 0);
    struct FAT32DirectoryTable *dir_table = &fat32_driver_state.dir_table_buf; 
    bool isParentValid = get_dir_table_from_cluster(request->parent_cluster_number, dir_table);
    if (!isParentValid){
        return 2;
    }

    bool found = false;
    int i;

    for(i=2; i<TOTAL_DIRECTORY_ENTRY; i++){

        if( fat32_driver_state.dir_table_buf.table[i].user_attribute == UATTR_NOT_EMPTY
        && 
        memcmp(fat32_driver_state.dir_table_buf.table[i].name, request->name, 8) == 0
        && 
        (isFolder 
        || memcmp(fat32_driver_state.dir_table_buf.table[i].ext, request->ext, 3) == 0
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
    if(request->buffer_size == 0){filesize = CLUSTER_SIZE;}
    else filesize = request->buffer_size;

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
    dir_entry->user_attribute = UATTR_NOT_EMPTY;
    dir_entry->filesize = filesize;
    copyStringWithLength(dir_entry->name, request->name, 8);
    copyStringWithLength(dir_entry->ext, request->ext, 3);
    if (isFolder) {
        dir_entry->attribute = ATTR_SUBDIRECTORY;
    } else {
        dir_entry->attribute = 0;
    }
    dir_entry->cluster_low = (uint16_t)(empty_clusters[0] & 0xFFFF);
    dir_entry->cluster_high = (uint16_t)(empty_clusters[0] >> 16);

    // Update FAT table
    for (int k = 0; k < alloc_cluster; k++) {
        if (k == alloc_cluster - 1) {
            fat32_driver_state.fat_table.cluster_map[empty_clusters[k]] = FAT32_FAT_END_OF_FILE;
        } else {
            fat32_driver_state.fat_table.cluster_map[empty_clusters[k]] = empty_clusters[k+1];
        }
        if(!isFolder){
            write_clusters(request->buf + CLUSTER_SIZE * k, empty_clusters[k], 1);
        }
    }
    write_clusters(&fat32_driver_state.fat_table, FAT_CLUSTER_NUMBER, 1);
    write_clusters(dir_table, request->parent_cluster_number, 1);

    return 0;
}

int8_t delete(struct FAT32DriverRequest *request){
    struct FAT32DirectoryTable *dir_table = &fat32_driver_state.dir_table_buf;
    bool isParentValid = get_dir_table_from_cluster(request->parent_cluster_number, dir_table);
    if (!isParentValid){
        return -1;
    }

    bool found = false;
    bool isFolder = false;
    uint32_t rc;
    for(rc = 2; rc<TOTAL_DIRECTORY_ENTRY; rc++){
        if(dir_table->table[rc].user_attribute == UATTR_NOT_EMPTY
        && memcmp(dir_table->table[rc].name, request->name, 8) == 0
        && (memcmp(dir_table->table[rc].ext, request->ext, 3) == 0)
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
            get_dir_table_from_cluster(request->parent_cluster_number, dir_table);
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
        write_clusters(dir_table, request->parent_cluster_number, 1);
    }

    return 0;
}