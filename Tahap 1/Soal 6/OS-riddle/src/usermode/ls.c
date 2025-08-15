#include "./ls.h"
#include "./user-shell.h"
#include "./util.h"


void ls()
{
    for (int i = 1; i < TOTAL_DIRECTORY_ENTRY; i++)
    {
    if (state.curr_dir.table[i].user_attribute == UATTR_NOT_EMPTY)
        {
            put_chars(state.curr_dir.table[i].name, BIOS_LIGHT_GREEN);
            if (state.curr_dir.table[i].attribute != ATTR_SUBDIRECTORY && strlen(state.curr_dir.table[i].ext) != 0)
            {
                put_chars(".", BIOS_LIGHT_BLUE );
                put_chars(state.curr_dir.table[i].ext, BIOS_LIGHT_GREEN);
            }
            if (i != TOTAL_DIRECTORY_ENTRY - 1) put_char(' ');
        }
    }
    put_char('\n');
}