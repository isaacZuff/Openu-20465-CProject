/* Helper functions to process and analyze code */
#ifndef _CODE_H
#define _CODE_H
#include "table.h"
#include "globals.h"

/**
 * Detects the opcode and the funct of a command by it's name
 * @param cmd The command name (string)
 * @param opcode_out The opcode value destination
 * @param funct_out The funct value destination
 */
void get_opcode_func(char* cmd, opcode *opcode_out, funct *funct_out);

/**
 * Returns the addressing type of an operand
 * @param operand The operand's string
 * @return The addressing type of the operand
 */
addressing_type get_addressing_type(char *operand);

/**
 * Validates and Builds a code word by the opcode, funct, operand count and operand strings
 * @param line_opcode The current opcode
 * @param line_funct The current funct
 * @param op_count The operands count
 * @param operands a 2-cell array of pointers to first and second operands.
 * @param opcode_encode struct of the opcode word OUTPUT
 * @param operand_encode struct of the operand word OUTPUT
 * @return Number of words to add(L from step 13 first pass) else 0
 */
int encode_opcode_wards(line_descriptor line, opcode line_opcode, funct line_funct, int op_count, char *operands[2],
                        opcode_word** opcode_encode, operand_word** operand_encode);

/**
 * Returns the register enum value by it's name
 * @param name The name of the register
 * @return The enum value of the register if found. otherwise, returns NONE_REGISTER
 */
reg get_regular_register_by_name(char *name);

/**
 * Returns the index register enum value by it's name
 * @param name register name
 * @return The enum value of the register if found. otherwise, returns NONE_REGISTER
 */
reg get_index_register_by_name(char *name);


/**
 * Register by name + index/register handling
 * @param name register name
 * @param addr_type addressing type
 * @return enum value of register if found else NONE_REGISTER
 */
reg get_register_by_name_and_addressing(char *name, addressing_type addr_type);

/**
 * Builds a data word by the operand's addressing type, value and whether the symbol (if it is one) is external.
 * @param addressing The addressing type of the value
 * @param data The value
 * @param external_symbol If the symbol is a label, and it's external
 * @return ABSOLUTE pointer to the constructed data word for the data by the specified properties.
 */
operand_data_word * encode_operand_data(addressing_type addressing, int data, bool external_symbol);

/**
 * Separates the operands from a certain index, puts each operand into the operands_out array,
 * and puts the found operand count in operand count argument
 * @param line The command text
 * @param i The index to start analyzing from
 * @param operands_out At least a 2-cell buffer of strings for the extracted operand strings
 * @param operand_count The operands_out of the detected operands count
 * @return Whether analyzing succeeded
 */
bool analyze_operands(line_descriptor line, int i, char **operands_out, int *operand_count);

#endif