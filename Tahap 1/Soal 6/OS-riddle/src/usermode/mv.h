#ifndef _MV_H_
#define _MV_H_

/*	if arg is a valid directory, move file to that directory
	else if arg is an invalid directory (i.e. has "/" in arg but folder not found), cancel mv
	else rename file to arg
*/	
#include <stdint.h>

void mv(char* arg1, char* arg2, uint32_t curr_pos);

#endif