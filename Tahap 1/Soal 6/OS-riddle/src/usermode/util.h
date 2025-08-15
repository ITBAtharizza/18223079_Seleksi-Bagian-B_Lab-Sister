#ifndef UTIL_H
#define UTIL_H
#include "user-shell.h"


void clearBuf(void *pointer, size_t n);

void print_curr_dir(struct ShellState s);

void print_curr_dir_helper(char* path_str, uint32_t current_dir);

void put_char(char buf);

void put_chars(char* buf, uint8_t color);

int my_atoi(char* str);

bool is_number(const char* str);

int findEntryName(char* name);


void extract_dir(char* dir, char directories[][12], int* numDirs);

void extract_filename(const char *filename, char *basename);

void extract_file_extension(const char *filename, char *extension);

// void extractDirectories(char* dir, char directories[][8], int* numDirs);
#endif