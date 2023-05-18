#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "utils.h"


/* Returns the first instruction from the specified index. if no such one, returns NONE */
instruction parse_instruction_from_index(line_descriptor line, int *index) {
    char temp[MAX_LINE_LENGTH];
    int j;
    instruction result;

    SKIP_TO_NEXT_NON_WHITESPACE(line.content, *index) /* Move to next non-whitespace char */
    if (line.content[*index] != '.') return NONE_INST;

    for (j = 0; line.content[*index] && line.content[*index] != '\t' && line.content[*index] != ' '; (*index)++, j++) {
        temp[j] = line.content[*index];
    }
    temp[j] = '\0'; /* End of string */
    /* if invalid instruction but starts with ., return error */
    if ((result = find_instruction_by_name(temp + 1)) != NONE_INST) { /* temp + 1(skip '.')*/
        return result;
    }
    fprintf_error_specific(line, "[ERROR] Invalid instruction name: %s", temp);
    return ERROR_INST; /* starts with '.' but not a valid instruction! */
}

/* Instruction line processing helper functions */

bool process_string_instruction(line_descriptor line, int index, long *data_img, long *dc) {
    char *last_quote_location = strrchr(line.content, '"'); /* str*r*char finds last occurrence*/
    int last_char_index = (last_quote_location-line.content)+1;
    SKIP_TO_NEXT_NON_WHITESPACE(line.content,last_char_index)

	SKIP_TO_NEXT_NON_WHITESPACE(line.content, index)

	if (line.content[index] != '"') {
		/* something like: LABEL: .string  hello, world\n - the string isn't surrounded with "" */
        fprintf_error_specific(line, "[ERROR] Missing opening quote of string");
		return FALSE;
	} else if (&line.content[index] == last_quote_location) { /* last quote is same as first quote */
        fprintf_error_specific(line, "[ERROR ] Missing closing quote of string");
		return FALSE;
	} else if( line.content[last_char_index] != '\n' && line.content[last_char_index] != EOF) { /* test if " is really the last char */
        fprintf_error_specific(line, "[ERROR] Chars after the closing quote");
        return FALSE;
    } else {
        index++; /* skip the first quote */
		/* Copy the string including quotes & everything until end of line */
		for (; line.content[index] && line.content[index] != '"'; index++) {
                data_img[*dc] = line.content[index];
                (*dc)++;
		}

		/* Add string terminator */
		data_img[*dc] = '\0';
		(*dc)++;
	}
	return TRUE;
}

/*
 * Parses a .data instruction. copies each number value to data_img by dc position, and returns the amount of processed data.
 */
bool process_data_instruction(line_descriptor line, int index, long *data_img, long *dc) {
	char temp[80], *temp_ptr;
	long value;
	int i;
	SKIP_TO_NEXT_NON_WHITESPACE(line.content, index)
	if (line.content[index] == ',') {
        fprintf_error_specific(line, "[ERROR] Unexpected comma after .data instruction");
        return FALSE;
	}
	do {
		for (i = 0;
		     line.content[index] && line.content[index] != EOF && line.content[index] != '\t' &&
		     line.content[index] != ' ' && line.content[index] != ',' &&
		     line.content[index] != '\n'; index++, i++) {
			temp[i] = line.content[index];
		}
		temp[i] = '\0'; /* End of string */
		if (!is_integer(temp)) {
            fprintf_error_specific(line, "Expected integer for .data instruction (got '%s')", temp);
			return FALSE;
		}
		/* Now let's write to data buffer */
		value = strtol(temp, &temp_ptr, 10);

		data_img[*dc] = value;

		(*dc)++; /* a word was written right now */
		SKIP_TO_NEXT_NON_WHITESPACE(line.content, index)
		if (line.content[index] == ',') index++;
		else if (!line.content[index] || line.content[index] == '\n' || line.content[index] == EOF)
			break; /* End of line/file/string => nothing to process anymore */
		/* Got comma. Skip white chars and check if end of line (if so, there's extraneous comma!) */
		SKIP_TO_NEXT_NON_WHITESPACE(line.content, index)
		if (line.content[index] == ',') {
            fprintf_error_specific(line, "Multiple consecutive commas.");
			return FALSE;
		} else if (line.content[index] == EOF || line.content[index] == '\n' || !line.content[index]) {
            fprintf_error_specific(line, "Missing data after comma");
			return FALSE;
		}
	} while (line.content[index] != '\n' && line.content[index] != EOF);
	return TRUE;
}
