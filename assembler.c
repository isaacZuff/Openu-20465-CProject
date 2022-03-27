#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "writefiles.h"
#include "utils.h"
#include "first_pass.h"
#include "second_pass.h"

/**
 * Processes a single assembly source file, and returns the result status.
 * @param filename The filename, without it's extension
 * @return Whether succeeded
 */
static bool process_file(char *filename);

/**
 * Entry point - 24bit assembler. Assembly language specified in booklet.
 */
int main(int argc, char *argv[]) {
	int i;
	/* To break line if needed */
	bool succeeded = TRUE;
	/* Process each file by arguments */
	for (i = 1; i < argc; ++i) {
		/* if last process failed and there's another file, break line: */
		if (!succeeded) puts("");
		/* foreach argument (file name), send it for full processing. */
		succeeded = process_file(argv[i]);
		/* Line break if failed */
	}
	return 0;
}

static bool process_file(char *filename) {
    /* Memory address counters */
    int temp_c;
    long ic = IC_INIT_VALUE, dc = 0, ICF, DCF;
    bool success_flag = TRUE; /* is succeeded so far */
    char *filename_with_ext;
    char temp_line[MAX_LINE_LENGTH + 2]; /* temporary string for storing line, read from file */
    FILE *file_des; /* Current assembly file descriptor to process */
    long data_img[CODE_ARR_IMG_LENGTH]; /* Contains an image of the machine code */
    machine_word *code_img[CODE_ARR_IMG_LENGTH];
    /* Our symbol table */
    table symbol_table = NULL;
    line_descriptor current_line;

    /* Concat extensionless filename with .as extension */
    filename_with_ext = strcat_to_new(filename, ".as");

    /* Open file, skip on failure */
    if ((file_des = fopen(filename_with_ext, "r")) == NULL) {
        /* if file couldn't be opened, write to stderr. */
        printf_error("[ERROR] Unable to read file: %s\n", filename);
        free(filename_with_ext); /*free the memory we allocated to the string concat */
        return FALSE;
    }

    /* start first pass: */
    current_line.file_name = filename_with_ext;
    current_line.content = temp_line; /* We use temp_line to read from the file, but it stays at same location. */
    /* Read line - stop if read failed (when NULL returned) - usually when EOF. increase line counter for error printing. */
    for (current_line.line_number = 1;
         fgets(temp_line, MAX_LINE_LENGTH + 2, file_des) != NULL; current_line.line_number++) {
        /* if line too long, the buffer doesn't include the '\n' char OR the file isn't on end. */
        if (strchr(temp_line, '\n') == NULL && !feof(file_des)) {
            /* Print message and prevent further line processing, as well as second pass.  */
            fprintf_error_specific(current_line,
                                   "[ERROR] Line is longer than MAX_LINE_LENGTH. Maximum line length should be %d.",
                                   MAX_LINE_LENGTH);
            success_flag = FALSE;
            /* skip leftovers */
            do {
                temp_c = fgetc(file_des);
            } while (temp_c != '\n' && temp_c != EOF);
        } else {
            if (!process_line_fpass(current_line, &ic, &dc, code_img, data_img, &symbol_table)) {
                if (success_flag) {
                    ICF = -1;
                    success_flag = FALSE;
                }
            }
        }
    }

    /* Step 18 Save IC and DC*/
    ICF = ic;
    DCF = dc;

    /* If we succeeded in step 1 we can continue to the second pass and finish the first pass */
    if (success_flag) {

        ic = IC_INIT_VALUE;

        /* First pass step 19 with ICF value */
        update_symbol_table_value(symbol_table, ICF, DATA_SYMBOL);

        /* First pass finished successfully */
        rewind(file_des); /* Readfile again from the beginning */

        /* Step 2 start */
        for (current_line.line_number = 1; !feof(file_des); current_line.line_number++) {
            int i = 0;
            fgets(temp_line, MAX_LINE_LENGTH, file_des); /* Get line */
            SKIP_TO_NEXT_NON_WHITESPACE(temp_line, i)
            if (code_img[ic - IC_INIT_VALUE] != NULL || temp_line[i] == '.')
                if(process_line_second_pass(current_line, &ic, code_img, &symbol_table) == FALSE){
                    success_flag = FALSE;
                }
        }

        /* Write files if second pass succeeded */
        if (success_flag) {
            /* Everything was done. Write to *filename.ob/.ext/.ent */
            success_flag = write_output_files(code_img, data_img, ICF, DCF, filename, symbol_table);
        }
    }

    /* Now let's free some pointer: */
    /* current file name */
    free(filename_with_ext);
    /* Free symbol table */
    free_table(symbol_table);
    /* Free code & data buffer contents */
    free_code_image(code_img, ICF);

    return success_flag;
}