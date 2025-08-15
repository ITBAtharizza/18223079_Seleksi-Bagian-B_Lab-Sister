#ifndef _FIND_H
#define _FIND_H

#include "./user-shell.h"
#include "./util.h"

#define IDX_UNDEF -1
#define CAPACITY 100

/* Definisi elemen dan address */
typedef uint32_t find_cluster_number;
typedef find_cluster_number ElType;
typedef struct {
	ElType buffer[CAPACITY]; 
	int idxHead;
	int idxTail;
} Queue;

//Mencari file/folder dengan nama yang sama diseluruh file system
void find(char* arg);

//Process FAT32 secara BFS, menggunakan tipe data Queue
void process_find(Queue *q, char* findName);

#endif