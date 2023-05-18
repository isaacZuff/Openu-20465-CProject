#ifndef _FIRST_PASS_H
#define _FIRST_PASS_H
#include "globals.h"

/**
 * Processes a single line in the first pass
 * @param line The line text
 * @param datas The data symbol table
 * @param codes The code symbol table
 * @param externals The externals symbol table
 * @param IC ABSOLUTE pointer to the current instruction counter
 * @param DC ABSOLUTE pointer to the current data counter
 * @param code_img The code image array
 * @param data_img The data image array
 * @return Whether succeeded.
 */
bool process_line_first_pass(line_descriptor line, long *IC, long *DC, machine_word **code_img, long *data_img,
                             table *symbol_table);

#endif