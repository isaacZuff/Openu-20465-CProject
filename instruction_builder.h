/* Helper functions for instruction analyzing */
#ifndef _INSTRUCTION_BUILDER_H
#define _INSTRUCTION_BUILDER_H
#include "globals.h"

/**
 * Returns the first instruction detected from the index in the string.
 * @param line The source string.
 * @param index The index to start looking from.
 * @return instruction_type indicates the detected instruction.
 */
instruction parse_instruction_from_index(line_descriptor line, int *index);

/**
 * Processes a .string instruction from index of source line.
 * @param line The source line
 * @param index The index
 * @param data_img The current data image
 * @param dc The current data counter
 * @return Whether succeeded
 */
bool process_string_instruction(line_descriptor line, int index, long *data_img, long *dc);

/**
 * Processes a .data instruction from index of source line.
 * @param line The source line
 * @param index The index
 * @param data_img The data image
 * @param dc The current data counter
 * @return Whether succeeded
 */
bool process_data_instruction(line_descriptor line, int index, long *data_img, long *dc);

#endif