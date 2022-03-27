#include <stdio.h>
#include <stdlib.h>
#include "second_pass.h"
#include "code.h"
#include "utils.h"
#include "string.h"

int process_second_pass_operand_old(line_descriptor line, long *curr_ic, long *ic, char *operand, machine_word **code_img,
                                    table *symbol_table);

int process_second_pass_operand(line_descriptor line, long *curr_ic, char *operand, machine_word **code_img,
                                table *symbol_table);

/**
 * Processes a single line in the second pass
 * @param line The line string
 * @param ic ABSOLUTE pointer to instruction counter
 * @param code_img Code image
 * @param symbol_table The symbol table
 * @return Whether operation succeeded
 */
bool process_line_second_pass(line_descriptor line, long *ic, machine_word **code_img, table *symbol_table) {
	char symbol[MAX_LABEL_LENGTH];
	long i = 0;

    /* Step 1 Readline till non-whitespace char*/
	SKIP_TO_NEXT_NON_WHITESPACE(line.content, i) /* Move to next non-whitespace char */
    /* Detect if we are in a comment or an empty line */
    if (!line.content[i] || line.content[i] == '\n' || line.content[i] == EOF || line.content[i] == ';')
    {
        return TRUE;
    }
    find_and_validate_label(line,symbol);

    /* check if we got a label in the first ward */
    if (symbol[0] != '\0') {
        i = index_of_char(line.content,':') + 1;
	}
	SKIP_TO_NEXT_NON_WHITESPACE(line.content, i) /* At least one space here */

	if (line.content[i] == '.') { /* Identify instructions by the dot prefix */
		/*Step 4 Add entry to symbol table else skip*/
		if ( strncmp(".entry", line.content+i, 6) == 0) {
			i += 6; /* strlen(.entry) ==6  */
			SKIP_TO_NEXT_NON_WHITESPACE(line.content, i)

			/* Find the label */
			if (!find_instruction_label(line.content+i,symbol)) {
                fprintf_error_specific(line, "[ERROR] Cannot find label");
				return FALSE;
			}
            /* Insert only if the label doesn't exist already */
			if (find_by_types(*symbol_table, symbol, 1, ENTRY_SYMBOL) == NULL) {
				table_entry *entry;
				/* if symbol is not already defined in data or code section it's an error*/
				if ((entry = find_by_types(*symbol_table, symbol, 2, DATA_SYMBOL, CODE_SYMBOL)) == NULL) {
					/* Symbol can't be external and entry */
					if ((entry = find_by_types(*symbol_table, symbol, 1, EXTERNAL_SYMBOL)) != NULL) {
                        fprintf_error_specific(line, "[ERROR] Symbol can't be external and entry symbol name: %s",
                                               entry->key);
						return FALSE;
					}
					/* otherwise, print more general error */
                    fprintf_error_specific(line, "Cant find symbol in the data/code tables");
					return FALSE;
				}
				add_table_item(symbol_table, symbol, entry->value, ENTRY_SYMBOL);
			}
		}
		return TRUE;
	}
	return add_symbol_to_machine_code(line, ic, code_img, symbol_table);
}

/**
 * Find the symbol that need replacment in a code line, and replacing them by the address in the symbol table.
 * @param line The current code line that is being processed
 * @param ic ABSOLUTE pointer to the current instruction counter
 * @param code_img The machine code image array
 * @param data_table The data symbol table
 * @param code_table The code symbol table
 * @param ext_table The externals symbol table
 * @param ext_references ABSOLUTE pointer to the external symbols references table
 * @return whether succeeded
 */
bool add_symbol_to_machine_code(line_descriptor line, long *ic, machine_word **code_img, table *symbol_table) {
	char temp[80];
	char *operands[2];
	int i = 0, operand_count;
	bool isvalid = TRUE;
	long curr_ic = (*ic)+1; /* we need to change the values we left null inside an already built array so we'll work temp counter */
	/* Get the total word length of current code text line in code binary image */
	int length = code_img[(*ic) - IC_INIT_VALUE]->length;
	/* if the length is 1, then there's only the code word, no data. */
	if (length > 1) {
		/* Now, we need to skip command, and get the operands themselves: */
		SKIP_TO_NEXT_NON_WHITESPACE(line.content, i)
        find_and_validate_label(line, temp);
		if (temp[0] != '\0') { /* if symbol is defined */
			/* move i right after it's end */
			for (; line.content[i] && line.content[i] != '\n' && line.content[i] != EOF && line.content[i] != ' ' &&
			       line.content[i] != '\t'; i++);
			i++;
		}
		SKIP_TO_NEXT_NON_WHITESPACE(line.content, i)
		/* now skip command */
		for (; line.content[i] && line.content[i] != ' ' && line.content[i] != '\t' && line.content[i] != '\n' &&
		       line.content[i] != EOF; i++);
		SKIP_TO_NEXT_NON_WHITESPACE(line.content, i)
		/* now analyze operands We send NULL as string of command because no error will be printed, and that's the only usage for it there. */
        analyze_operands(line, i, operands, &operand_count);
		/* Process operands, if needed. if failed return failure. otherwise continue */
		if (operand_count--) {
			isvalid = process_second_pass_operand(line, &curr_ic, operands[0], code_img, symbol_table);
			free(operands[0]);
			if (!isvalid) {
                if(operand_count){
                    free(operands[1]);
                }
                return FALSE;
            }
			if (operand_count) {
				isvalid = process_second_pass_operand(line, &curr_ic, operands[1], code_img, symbol_table);
				free(operands[1]);
				if (!isvalid) return FALSE;
			}
		}
	}
	/* Make the current pass IC as the next line ic */
	(*ic) = (*ic) + length;
	return TRUE;
}

/**
 * Builds the additional data word for operand in the second pass, if needed.
 * @param curr_ic Current instruction pointer of source code line
 * @param ic Current instruction pointer of source code line start
 * @param operand The operand string
 * @param code_img The code image array
 * @param symbol_table The symbol table
 * @return Whether succeeded
 */
int process_second_pass_operand_old(line_descriptor line, long *curr_ic, long *ic, char *operand, machine_word **code_img,
                                    table *symbol_table) {
	addressing_type addr = get_addressing_type(operand);
	machine_word *word_to_write;
    bool is_external = FALSE;
	/* We already handled immediate addressing, we can keep going */
	if (addr == IMMEDIATE_ADDR) {
        (*curr_ic)++;
    }
	if (addr == INDEX_ADDR) {
        operand++;
    }
	if (DIRECT_ADDR == addr || INDEX_ADDR == addr) {
		long data_to_add;
		table_entry *entry = find_by_types(*symbol_table, operand, 3, DATA_SYMBOL, CODE_SYMBOL, EXTERNAL_SYMBOL);
		if (entry == NULL) {
            fprintf_error_specific(line, "The symbol %s not found", operand);
			return FALSE;
		}
		/*found symbol*/
		data_to_add = entry->value;
		/* Calculate the distance to the label from ic if needed */
		if (addr == INDEX_ADDR) {
			/* if not code symbol it's impossible to calculate distance! */
			if (entry->type != CODE_SYMBOL) {
                fprintf_error_specific(line,
                                       "The symbol %s cannot be addressed relatively because it's not a code symbol.",
                                       operand);
				return FALSE;
			}
			data_to_add = data_to_add - *ic;
		}
		/* Add to externals reference table if it's an external. increase ic because it's the next data word */
		if (entry->type == EXTERNAL_SYMBOL) {
            is_external = TRUE;
			add_table_item(symbol_table, operand, (*curr_ic) + 1, EXTERNAL_REFERENCE);
		}

		/*found symbol*/
        /*
		word_to_write = (machine_word *) better_malloc(sizeof(machine_word));
		word_to_write->length = 0;
		word_to_write->word.data = encode_operand_data(addr, data_to_add, is_external);
		code_img[(++(*curr_ic)) - IC_INIT_VALUE] = word_to_write;
*/
	}
	return TRUE;
}


/**
 * Builds the additional data word for operand in the second pass, if needed.
 * @param curr_ic Current instruction pointer of source code line
 * @param operand The operand string
 * @param code_img The code image array
 * @param symbol_table The symbol table
 * @return Whether succeeded
 */
int process_second_pass_operand(line_descriptor line, long *curr_ic, char *operand, machine_word **code_img,
                                table *symbol_table) {
    addressing_type addr = get_addressing_type(operand);
    machine_word *machine_base_word, *machine_offset_word;
    /* We already handled immediate addressing, we can keep going */
        if (addr == IMMEDIATE_ADDR) {
        (*curr_ic)++;
    }

    if (DIRECT_ADDR == addr || INDEX_ADDR == addr) {
        bool is_external = FALSE;
        table_entry *entry = find_by_types(*symbol_table, operand, 3, DATA_SYMBOL, CODE_SYMBOL, EXTERNAL_SYMBOL);
        /* operand++;  NOT NEEDED?? */

        if (entry == NULL) {
            fprintf_error_specific(line, "[ERROR] Cant find symbol %s in second pass", operand);
            return FALSE;

        }

        if (entry->type == EXTERNAL_SYMBOL) {
            is_external = TRUE;
            add_table_item(symbol_table, operand, (*curr_ic) + 2, EXTERNAL_REFERENCE);
        }

        machine_base_word = (machine_word *) better_malloc(sizeof(machine_word));
        machine_base_word->is_operand =FALSE;
        machine_base_word->length = 0;
        machine_base_word->word.data2 = encode_operand_data(addr, entry->base, is_external);
        code_img[(++(*curr_ic)) - IC_INIT_VALUE] = machine_base_word;

        machine_offset_word = (machine_word *) better_malloc(sizeof(machine_word));
        machine_offset_word->is_operand =FALSE;
        machine_offset_word->length = 0;
        machine_offset_word->word.data2 = encode_operand_data(addr, entry->offset, is_external);
        code_img[(++(*curr_ic)) - IC_INIT_VALUE] = machine_offset_word;



    }
    return TRUE;
}
