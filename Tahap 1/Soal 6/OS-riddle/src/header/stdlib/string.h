#ifndef _STRING_H
#define _STRING_H

#include <stdint.h>
#include <stddef.h>

/**
 * C standard memset, check man memset or
 * https://man7.org/linux/man-pages/man3/memset.3.html for more details
 * 
 * @param s Pointer to memory area to set
 * @param c Constant byte value for filling memory area
 * @param n Memory area size in byte 
 * 
 * @return Pointer s
*/
void* memset(void *s, int c, size_t n);

/**
 * C standard memcpy, check man memcpy or
 * https://man7.org/linux/man-pages/man3/memcpy.3.html for more details
 * 
 * @param dest Starting location for memory area to set
 * @param src Pointer to source memory
 * @param n Memory area size in byte 
 * 
 * @return Pointer dest
*/
void* memcpy(void* restrict dest, const void* restrict src, size_t n);

/**
 * C standard memcmp, check man memcmp or
 * https://man7.org/linux/man-pages/man3/memcmp.3.html for more details
 * 
 * @param s1 Pointer to first memory area
 * @param s2 Pointer to second memory area
 * @param n Memory area size in byte 
 * 
 * @return Integer as error code, zero for equality, non-zero for inequality
*/
int memcmp(const void *s1, const void *s2, size_t n);

/**
 * C standard memmove, check man memmove or
 * https://man7.org/linux/man-pages/man3/memmove.3.html for more details
 * 
 * @param dest Pointer to destination memory
 * @param src Pointer to source memory
 * @param n Memory area size in byte 
 * 
 * @return Pointer dest
*/


void *memmove(void *dest, const void *src, size_t n);

int strcmp(char *str1, char *str2, int length);

int my_memcmp(const void *ptr1, const void *ptr2, size_t n);

void my_memset(void *ptr, int value, size_t num);

void *my_memcpy(void *dest, const void *src, size_t length);

void copyStringWithLength(char* destination, const char* source, int length);

bool cmp_string_with_fixed_length(const char *a, const char *b, int l);

char *my_strtok(char *str, char delim);

size_t strlen(const char *str);

#endif
