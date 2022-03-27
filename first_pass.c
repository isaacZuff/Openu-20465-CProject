/* Contains major function that are related to the first pass */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "globals.h"
#include "code.h"
#include "utils.h"
#include "instructions.h"
#include "first_pass.h"


/**
 * Processes a single code line in the first pass.
 * Adds the code build binary structure to the code_img,
 * encodes immediately-addresses operands and leaves required data word that use labels NULL.
 * @param line The code line to process
 * @param i Where to start processing the line from
 * @param ic ABSOLUTE pointer to the current instruction counter
 * @param code_img The code image array
 * @return Whether succeeded or notssss
 */
static bool process_code(line_descriptor line, int i, long *ic, machine_word **code_img);

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
bool process_line_fpass(line_descriptor line, long *IC, long *DC, machine_word **code_img, long *data_img,
                        table *symbol_table) {
	int i, j;
	char symbol[MAX_LABEL_LENGTH];
    bool is_symbol = FALSE;
	instruction instruction;

	i = 0;

    /* Step 2 Readline till non-whitespace char*/
	SKIP_TO_NEXT_NON_WHITESPACE(line.content, i) /* Move to next non-whitespace char */
    /* Detect if we are in a comment or an empty line */
	if (!line.content[i] || line.content[i] == '\n' || line.content[i] == EOF || line.content[i] == ';')
		return TRUE;

    /* Step 3-5 Symbol handling */
	/* Process label(if exists)*/
	if (find_and_validate_label(line, symbol)) {
		return FALSE; /* Stop line processing if label is invalid */
	}


	if (symbol[0] != '\0') {
        /* Its a symbol, lets move the index to label content section
         * index of ':' + mandatory space after*/
        is_symbol = TRUE;
        i = index_of_char(line.content,':') + 1;
	}

	SKIP_TO_NEXT_NON_WHITESPACE(line.content, i) /* Move to next non-whitespace char */

	if (line.content[i] == '\n') return TRUE; /* Empty label => skip */

	/* Check whether symbol is already defined in the relevant tables */
	if (is_symbol && find_by_types(*symbol_table, symbol, 3, EXTERNAL_SYMBOL, DATA_SYMBOL, CODE_SYMBOL) != NULL) {
        fprintf_error_specific(line, "Symbol %s is already defined.", symbol);
		return FALSE;
	}

	/* Check if it's an instruction (starting with '.') */
	instruction = parse_instruction_from_index(line, &i);

	if (instruction == ERROR_INST) { /* Syntax error found */
		return FALSE;
	}

	SKIP_TO_NEXT_NON_WHITESPACE(line.content, i) /* Move to next non-whitespace char */

	/* is it's an instruction */
	if (instruction != NONE_INST) {
		/* if .string or .data, and symbol defined, put it into the symbol table */
		if ((instruction == DATA_INST || instruction == STRING_INST) && is_symbol)
			/* is data or string, add DC with the symbol to the table as data */
			add_table_item(symbol_table, symbol, *DC, DATA_SYMBOL);

		/* if its a string instruction, encode into data image buffer and increase dc as needed. */
		if (instruction == STRING_INST)
			return process_string_instruction(line, i, data_img, DC);
			/* if .data, do same but parse numbers. */
		else if (instruction == DATA_INST)
			return process_data_instruction(line, i, data_img, DC);
			/* if .extern, add to externals symbol table */
		else if (instruction == EXTERN_INST) {
			SKIP_TO_NEXT_NON_WHITESPACE(line.content, i)
			/* if external symbol detected, start analyzing from its deceleration end */
			for (j = 0; line.content[i] && line.content[i] != '\n' && line.content[i] != '\t' && line.content[i] != ' ' && line.content[i] != EOF; i++, j++) {
				symbol[j] = line.content[i];
			}
			symbol[j] = '\0';
			/* If invalid external label name, it's an error */
			if (!is_valid_label_name(symbol)) {
                fprintf_error_specific(line, "[ERROR] Invalid external label name: %s", symbol);
				return FALSE;
			}
			add_table_item(symbol_table, symbol, 0, EXTERNAL_SYMBOL); /* Extern value is defaulted to 0 */
		}
			/* if entry and symbol defined, print error */
		else if (instruction == ENTRY_INST && is_symbol ) {
            fprintf_error_specific(line, "[ERROR] Defining a label to an entry instruction is not allowed.");
			return FALSE;
		}
		/* .entry is handled in second pass! */
	} /* end if (instruction != NONE) */
		/* not instruction=>it's a command! */
	else {
		/* if symbol defined, add it to the table */
		if (is_symbol)
			add_table_item(symbol_table, symbol, *IC, CODE_SYMBOL);
		/* Analyze code */
		return process_code(line, i, IC, code_img);
	}
	return TRUE;
}

/**
 * Allocates and builds the data inside the additional code word by the given operand,
 * Only in the first pass
 * @param code_img The current code image
 * @param ic The current instruction counter
 * @param operand The operand to check
 */
static void encode_addressing_additional_words(machine_word **code_img, long *ic, char *operand);

/**
 * Processes a single code line in the first pass.
 * Adds the code build binary structure to the code_img,
 * encodes immediately-addresses operands and leaves required data word that use labels NULL.
 * @param line The code line to process
 * @param i Where to start processing the line from
 * @param ic ABSOLUTE pointer to the current instruction counter
 * @param code_img The code image array
 * @return Whether succeeded or notssss
 */
static bool process_code(line_descriptor line, int i, long *ic, machine_word **code_img) {
	char operation[8]; /* stores the string of the current code instruction */
	char *operands[2]; /* 2 strings, each for operand */
    long start_ic;
    int j, operand_count,status;
	opcode curr_opcode; /* the current opcode and funct values */
	funct curr_funct;
	machine_word *opcode_machine_word, *operand_machine_word;
    opcode_word* opcode_word_temp = NULL;
    operand_word*  operand_word_temp = NULL;
	/* Skip white chars */
	SKIP_TO_NEXT_NON_WHITESPACE(line.content, i)

	/* Until white char, end of line, or too big instruction, copy it: */
	for (j = 0; line.content[i] && line.content[i] != '\t' && line.content[i] != ' ' && line.content[i] != '\n' && line.content[i] != EOF && j < 6; i++, j++) {
		operation[j] = line.content[i];
	}
	operation[j] = '\0'; /* End of string */
	/* Get opcode & funct by command name into curr_opcode/curr_funct */
	get_opcode_func(operation, &curr_opcode, &curr_funct);
	/* If invalid operation (opcode is DEFAULT_OP=-1), print and skip processing the line. */
	if (curr_opcode == DEFAULT_OP) {
        fprintf_error_specific(line, "Unrecognized instruction: %s.", operation);
		return FALSE; /* an error occurred */
	}

	/* Separate operands and get their count */
	if (!analyze_operands(line, i, operands, &operand_count))  {
		return FALSE;
	}

	/* Build code word struct to store in code image array */
	if ((status = encode_opdcode_wards(line, curr_opcode, curr_funct, operand_count, operands, &opcode_word_temp,
                                       &operand_word_temp)) == 0) {
		/* Release allocated memory for operands */
		if (operands[0]) {
			free(operands[0]);
			if (operands[1]) {
				free(operands[1]);
			}
		}
		return FALSE;
	}
    status; /* nope */

    /* IC before encoding of opcode+operands */
    start_ic = *ic;
	/* allocate memory for a new word in the code image, and put the code word into it */
	opcode_machine_word = (machine_word *) better_malloc(sizeof(machine_word));
	(opcode_machine_word->word).opcode = opcode_word_temp;
	code_img[(*ic) - IC_INIT_VALUE] = opcode_machine_word; /* IC initialized to 100 but we shouldn't skip the first cells  */

    if(operand_word_temp != NULL) {
        (*ic)++;
        operand_machine_word = (machine_word *) better_malloc(sizeof(machine_word));
        operand_machine_word->is_operand = TRUE;
        operand_machine_word->length=0;
        (operand_machine_word->word).operand = operand_word_temp;
        code_img[(*ic) - IC_INIT_VALUE] = operand_machine_word;
    }

	/* Build extra information code word if possible, free pointers with no need */
	if (operand_count--) { /* Its true unless operand == 0 we subtract to handle the case of 1 operand */
        encode_addressing_additional_words(code_img, ic, operands[0]);
		free(operands[0]);
		if (operand_count) {
            encode_addressing_additional_words(code_img, ic, operands[1]);
			free(operands[1]);
		}
	}

	(*ic)++; /* increase ic to point the next cell */

	/* Add the final length (of code word + data words) to the code word struct: */
	code_img[start_ic - IC_INIT_VALUE]->length = (*ic) - start_ic;

	return TRUE; /* No errors */
}

static void encode_addressing_additional_words(machine_word **code_img, long *ic, char *operand) {
	addressing_type operand_addressing = get_addressing_type(operand);
	/* Register includes no additional info words */
	if (operand_addressing != REGISTER_ADDR && operand_addressing != NONE_ADDR) {
		(*ic)++; /* We add one word to the ic */
		if (operand_addressing == IMMEDIATE_ADDR) {
			char *ptr;
			machine_word *immediate_addr_word;
			/* skip the first char because immediate addressing specifies it equals # */
			int value = strtol(operand + 1, &ptr, 10);
            immediate_addr_word = (machine_word *) better_malloc(sizeof(machine_word));
            immediate_addr_word->length = 0;
			(immediate_addr_word->word).data2 = encode_operand_data(IMMEDIATE_ADDR, value, FALSE);

			code_img[(*ic) - IC_INIT_VALUE] = immediate_addr_word;
		} else{
            (*ic)++; /* Direct/Index 2 more words */
        }
	}
}