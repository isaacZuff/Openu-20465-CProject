/* Second pass line processing related functions */
#ifndef _SECOND_PASS_H
#define _SECOND_PASS_H
#include "globals.h"
#include "symbol_table.h"

/**
 * Processes a single line in the second pass
 * @param line The line string
 * @param ic  pointer to instruction counter
 * @param code_img Code image
 * @param symbol_table The symbol table
 * @return Whether operation succeeded
 */
bool process_line_second_pass(line_descriptor line, long *ic, machine_word **code_img, table *symbol_table);

/***
  * populate the missing values in the code image
  * @param line Proceesed line
  * @param ic pointer to ic counter
  * @param code_img code image array
  * @param symbol_table symbol_table pointer
  * @return
  */
bool add_symbol_to_machine_code(line_descriptor line, long *ic, machine_word **code_img, table *symbol_table);

#endif