#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "helper.h"
#include "opcode_builder.h" /* for checking reserved words */

#define STDERR_FILE stdout /* we should print to stderr but w/e */

char *strcat_to_new(char *first_str, char* second_str) {
    /* first_str_len + second_str_len + string line terminator */
	char *new_string = (char *) better_malloc(strlen(first_str) + strlen(second_str) + 1);
	strcpy(new_string, first_str);
	strcat(new_string, second_str);
	return new_string;
}


/**
 * Finds the defined label in the code if exists, and saves it into the buffer.
 * If valid, symbol saved in symbol_buff
 * @param line The source line to find in
 * @param symbol_buff The buffer for copying the found label in
 * @return False if label not found or found and valid, True only if invalid
 */
bool find_and_validate_label(line_descriptor line, char *symbol_buff) {
	int j, i;
	i = j = 0;

	SKIP_TO_NEXT_NON_WHITESPACE(line.content, i) /* Move to next non-whitespace char */

    /* Advancing the j variable whether it's a whitespace chat or not is a trick
     * to make index finding relative to the line easier */
	for (; line.content[i] && line.content[i] != ':' && line.content[i] != EOF && line.content[i] != ' ' && i <= MAX_LINE_LENGTH; i++, j++) {
        symbol_buff[j] = line.content[i];
	}
    symbol_buff[j] = '\0'; /* End of string */

	if (line.content[i] == ':') {
        /* We encountered a label, check validity */
		if (!is_valid_label_name(symbol_buff)) {
			fprintf_error_specific(line, "[ERROR] Invalid label name");
            symbol_buff[0] = '\0';
			return TRUE; /* Symbol is invalid*/
		}
		return FALSE; /* Symbol is valid */
	}
    symbol_buff[0] = '\0';
	return FALSE; /* Symbol not found */
}

/***
 * Gets the first field in the line
 * @param line_string
 * @return Index of the first char after the field or -1 if not found
 */
int get_first_field(const char* line_string,char* field){
    int i=0,j=0;
    SKIP_TO_NEXT_NON_WHITESPACE(line_string,i);
    for (; line_string[i] && line_string[i] != '\n' && line_string[i] != EOF && line_string[i] != ' ' && i <= MAX_LINE_LENGTH; i++) {
        field[j] = line_string[i];
        j++;
    }
    field[j] = '\0';

    if(field[0] == '\0'){
        return -1;
    }
    return j;
}

/**
 * Finds the defined label in the code if exists, and saves it into the buffer.
 * If valid, symbol saved in symbol_buff
 * @param content The string for searching
 * @param symbol_buff The buffer for copying the found label in
 * @return False if label not found or found and valid else True
 */
bool find_instruction_label(char* content, char *symbol_buff) {
	int i=0, after_white_space;
	bool letter_found = FALSE;
	SKIP_TO_NEXT_NON_WHITESPACE(content, i) /* Move to next non-whitespace char */
	after_white_space=i;

	/* Advancing the j variable whether it's a whitespace chat or not is a trick
     * to make index finding relative to the line easier */
	for (; content[i] && content[i] != '\n' && content[i] != EOF && i <= MAX_LINE_LENGTH; i++) {
		symbol_buff[i] = content[i];
		letter_found = TRUE;
	}
	symbol_buff[i] = '\0'; /* End of string */

	SKIP_TO_NEXT_NON_WHITESPACE(content, i) /* Move to next non-whitespace char */

	if(letter_found && (content[i] == '\n' || content[i]==EOF)){
		return TRUE; /* Found symbol */
	}

	symbol_buff[0] = '\0';
	return FALSE; /* Symbol not found */
}

/***
 * Takes the register out of the label
 * @param full_label
 * @return label without register and braces
 */
char *extract_index_addressing_label(char* full_label){
    char *short_label = better_malloc(MAX_LABEL_LENGTH);
    int i, open_braces_index = index_of_char(full_label, '[');

    for(i=0; i<open_braces_index;i++) {
        short_label[i] = full_label[i];
    }
    short_label[i] = '\0';
    return short_label;
}

/***
 * Finds index of char in a given string
 * @param string string to search in
 * @param c char to find
 * @return index location int or -1 if not found
 */
int index_of_char(char* string, int c) {
    char *index_pointer;
    index_pointer = strchr(string, c);
	if(index_pointer == NULL){
		return -1;
	}
    return (int) (index_pointer - string);
}


struct instruction_lookup_item {
	char *name;
	instruction value;
};

static struct instruction_lookup_item
		instructions_lookup_table[] = {
		{"string", STRING_INST},
		{"data",   DATA_INST},
		{"entry",  ENTRY_INST},
		{"extern", EXTERN_INST},
		{NULL, NONE_INST}
};

instruction get_instruction_by_name(char *instruction_str) {
	struct instruction_lookup_item *curr_item;
	for (curr_item = instructions_lookup_table; curr_item->name != NULL; curr_item++) {
		if (strcmp(curr_item->name, instruction_str) == 0) {
			return curr_item->value;
		}
	}
	return NONE_INST;
}

bool is_integer(char *string) {
	int i = 0;
	if (string[0] == '-' || string[0] == '+') string++; /* if string starts with +/-, it's OK */
	for (; string[i]; i++) { /* Just make sure that everything is a digit until the end */
		if (!isdigit(string[i])) {
			return FALSE;
		}
	}
	return i > 0; /* if i==0 then it was an empty string! */
}
/***
 * Malloc wrapper with error "handling"
 * @param size size to allocate in bytes
 * @return pointer to allocated memory on successful allocation
 */
void *better_malloc(long size) {
	void *ptr = malloc(size);
	if (ptr == NULL) {
		printf("[ERROR] Malloc failed exiting the program.");
		exit(1);
	}
	return ptr;
}

/***
 * Checks for label validity
 * ABSOLUTE label is valid iff
 * Label isn't longer than MAX_LABEL_LENGTH
 * First char is alphabetic
 * All chars are alphanumeric
 * Label isn't a reserved word
 * @param name label name
 * @return True if label is valid else false
 */
bool is_valid_label_name(char *name) {
	return name[0] && strlen(name) <= MAX_LABEL_LENGTH && isalpha(name[0]) && is_alnum_str(name + 1) &&
           !is_reserved_word(name);
}

/**
 * Wrapper for isalnum ctype.h function that handles strings
 * @param string input string
 * @return True if alphanumeric else false
 */
bool is_alnum_str(char *string) {
	int i;
    /* enumerate string characters and feeds them to isalnum func  */
	for (i = 0; string[i]; i++) {
		if (!isalnum(string[i])) return FALSE;
	}
	return TRUE;
}

bool is_reserved_word(char *name) {
	int fun, opc;
	/* check if register or command */
	get_opcode_func(name, &opc, (funct *) &fun);
	if (opc != DEFAULT_OP || get_regular_register_by_name(name) != NONE_REGISTER ||
            get_instruction_by_name(name) != NONE_INST) return TRUE;

	return FALSE;
}

int fprintf_error_specific(line_descriptor line, char *message, ...) { /* Prints the errors into a file, defined above as macro */
	int result;
	va_list args;
	fprintf(STDERR_FILE, "Error In %s:%ld: ", line.full_file_name, line.line_number);

	va_start(args, message);
	result = vfprintf(STDERR_FILE, message, args);
	va_end(args);

	fprintf(STDERR_FILE, "\n");
	return result;
}

int printf_error(char *message, ...) { /* Prints the errors into a file, defined above as macro */
    int result;
    va_list args; /* for formatting */
    /* Print file+line */

    /* use vprintf to call printf from variable argument function (from stdio.h) with message + format */
    va_start(args, message);
    result = vfprintf(STDERR_FILE, message, args);
    va_end(args);

    fprintf(STDERR_FILE, "\n");
    return result;
}

void free_code_image(machine_word **code_image, long icf) {
	long i;
	for (i = 0; i < icf; i++) {
		machine_word *curr_word = code_image[i];
		if (curr_word != NULL) {
			/* free code/data word */
			if (curr_word->length > 0) {
				free(curr_word->word.opcode);
			} else if(curr_word->is_operand== TRUE){
                free(curr_word->word.operand);
            } else {
				free(curr_word->word.data2);
			}
			/* free the pointer to the union */
			free(curr_word);
			code_image[i] = NULL;
		}
	}
}
