/* Contains general-purposed functions, for both passes and many usages */
#ifndef _UTILS_H
#define _UTILS_H

#include "globals.h"


/** simple macro that forwards the line index to the next non-whitespace char */
#define SKIP_TO_NEXT_NON_WHITESPACE(string, index) for (;string[index] && (string[index] == '\t' || string[index] == ' '); (++index));

/**
 * Concatenates both string to a new allocated memory
 * @param first_str The first string
 * @param second_str The second string
 * @return ABSOLUTE pointer to the new, allocated string
 */
char *strcat_to_new(char *first_str, char* second_str);

/**
 * Finds the defined label in the code if exists, and saves it into the buffer.
 * If valid, symbol saved in symbol_buff
 * @param line The source line to find in
 * @param symbol_buff The buffer for copying the found label in
 * @return False if label not found or found and valid else True
 */
bool find_and_validate_label(line_descriptor line, char *symbol_buff);

/**
 * Finds the defined label in the code if exists, and saves it into the buffer.
 * If valid, symbol saved in symbol_buff
 * @param content The string for searching
 * @param symbol_buff The buffer for copying the found label in
 * @return False if label not found or found and valid else True
 */
bool find_instruction_label(char* content, char *symbol_buff);

/***
 * Takes the register out of the label
 * @param full_label
 * @return label without register and braces
 */
char *extract_index_addressing_label(char* full_label);

/***
 * Finds index of char in a given string
 * @param string string to search in
 * @param c char to find
 * @return index location int or -1 if not found
 */
int index_of_char(char* string, int c);

/**
 * Returns the instruction enum by the instruction's name, without the opening '.'
 * @param name The instruction name, without the '.'
 * @return The instruction enum if found, NONE_INST if not found.
 */
instruction find_instruction_by_name(char *name);

/**
 * Tests whether a string is a collection of digits only
 * @param string string number to check
 * @return True if its an integer else false
 */
bool is_integer(char* string);

/***
 * Malloc wrapper with error "handling"
 * @param size size to allocate in bytes
 * @return pointer to allocated memory on successful allocation
 */
void *better_malloc(long size);

/***
 * Checks for label validity
 * ABSOLUTE label is valid iff
 * Label isn't longer than MAX_LABEL_LENGTH
 * First char is alphabetic
 * All chars are alphanumeric
 * Label isn't a reserved word
 * @param name label name
 * @return True if label is valid and vice versa
 */
bool is_valid_label_name(char* name);

/**
 * Wrapper for isalnum ctype.h function that handles strings
 * @param string input string
 * @return True if alphanumeric else false
 */
bool is_alnum_str(char *string);

/*Returns TRUE if name is saved word*/
bool is_reserved_word(char *name);

/**
 * Prints a detailed error message, including file name and line number by the specified message,
 * formatted as specified in App. B of "The C Programming language" for printf.
 * @param line line_descriptor information object
 * @param message The error message
 * @param ... The arguments to format into the message
 * @return printf result of the message
 */
int fprintf_error_specific(line_descriptor line, char *message, ...);

/**
 * Prints a detailed error message
 * formatted as specified in App. B of "The C Programming language" for printf.
 * @param message The error message
 * @param ... The arguments to format into the message
 * @return printf result of the message
 */
int printf_error(char *message, ...);

/**
 * Frees all the dynamically-allocated memory for the code image.
 * @param code_image ABSOLUTE pointer to the code images buffer
 * @param fic The final instruction counter value
 */
void free_code_image(machine_word **code_image, long fic);

#endif