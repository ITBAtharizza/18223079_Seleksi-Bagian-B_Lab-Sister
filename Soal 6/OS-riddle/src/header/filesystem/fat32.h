#ifndef _FAT32_H
#define _FAT32_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "../driver/disk.h"

#define BOOT_SECTOR           0
#define CLUSTER_BLOCK_COUNT   4
#define CLUSTER_SIZE          (BLOCK_SIZE*CLUSTER_BLOCK_COUNT)
#define CLUSTER_MAP_SIZE      512

#define CLUSTER_0_VALUE       0x0FFFFFF0
#define CLUSTER_1_VALUE       0x0FFFFFFF

#define FAT32_FAT_END_OF_FILE 0x0FFFFFFF
#define FAT32_FAT_EMPTY_ENTRY 0x00000000

#define FAT_CLUSTER_NUMBER    1
#define ROOT_CLUSTER_NUMBER   2

#define ATTR_SUBDIRECTORY     0b00010000
#define UATTR_NOT_EMPTY       0b10101010

#define TOTAL_DIRECTORY_ENTRY (int)(CLUSTER_SIZE/sizeof(struct FAT32DirectoryEntry))

extern const uint8_t fs_signature[BLOCK_SIZE];

struct ClusterBuffer {
    uint8_t buf[CLUSTER_SIZE];
} __attribute__((packed));

struct GiantClusterBuffer {
    uint8_t buf[350 * CLUSTER_SIZE];
} __attribute__((packed));

struct FAT32FileAllocationTable {
    uint32_t cluster_map[CLUSTER_MAP_SIZE];
} __attribute__((packed));

struct FAT32DirectoryEntry {
    char     name[8];
    char     ext[3];
    uint8_t  attribute;
    uint8_t  user_attribute;

    bool     undelete;
    uint16_t create_time;
    uint16_t create_date;
    uint16_t access_date;
    uint16_t cluster_high;
    
    uint16_t modified_time;
    uint16_t modified_date;
    uint16_t cluster_low;
    uint32_t filesize;
} __attribute__((packed));

struct FAT32DirectoryTable {
    struct FAT32DirectoryEntry table[CLUSTER_SIZE / sizeof(struct FAT32DirectoryEntry)];
} __attribute__((packed));

struct FAT32DriverState {
    struct FAT32FileAllocationTable fat_table;
    struct FAT32DirectoryTable      dir_table_buf;
    struct ClusterBuffer            cluster_buf;
} __attribute__((packed));

struct FAT32DriverRequest {
    void     *buf;
    char      name[8];
    char      ext[3];
    uint32_t  parent_cluster_number;
    uint32_t  buffer_size;
    uint32_t  self_cluster_number;
} __attribute__((packed));

uint32_t cluster_to_lba(uint32_t cluster);

void init_directory_table(struct FAT32DirectoryTable *dir_table, char *name, uint32_t parent_dir_cluster, uint32_t self_cluster_number);

bool is_empty_storage(void);

void create_fat32(void);

void initialize_filesystem_fat32(void);

void write_clusters(const void *ptr, uint32_t cluster_number, uint8_t cluster_count);

void read_clusters(void *ptr, uint32_t cluster_number, uint8_t cluster_count);

int8_t read_directory(struct FAT32DriverRequest request);

int8_t read(struct FAT32DriverRequest request);

int8_t write(struct FAT32DriverRequest request);

int8_t delete(struct FAT32DriverRequest request);

bool cmp_string_with_fixed_length(const char *a, const char *b, int l);

#endif