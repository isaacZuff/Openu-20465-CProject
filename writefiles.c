#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "utils.h"
#include "table.h"

#define KEEP_ONLY_24_LSB(value) ((value) & 0xFFFFFF)
/**
 * "Cuts" the msb of the value, keeping only it's lowest 21 bits
 * 0b00000000000111111111111111111111 = 0x1FFFFF
 */
#define KEEP_ONLY_21_LSB(value) ((value) & 0x1FFFFF)

/**
 * Writes the code and data2 image into an .ob file, with lengths on top
 * @param code_img The code image
 * @param data_img The data2 image
 * @param icf The final instruction counter
 * @param dcf The final data2 counter
 * @param filename The filename, without the extension
 * @return Whether succeeded
 */
static bool write_ob(machine_word **code_img, long *data_img, long icf, long dcf, char *filename);

/**
 * Writes a symbol table to a file. Each symbol and it's address in line, separated by a single space.
 * @param tab The symbol table to write
 * @param filename The filename without the extension
 * @param file_extension The extension of the file, including dot before
 * @return Whether succeeded
 */
static bool write_entries_file(table tab, char *filename, char *file_extension);

bool write_external_file(table tab, char *filename, char *file_extension);

int write_output_files(machine_word **code_img, long *data_img, long icf, long dcf, char *filename,
                       table symbol_table) {
	bool result;
	table externals = filter_table_by_type(symbol_table, EXTERNAL_REFERENCE);
	table entries = filter_table_by_type(symbol_table, ENTRY_SYMBOL);
	/* Write .ob file */
	result = write_ob(code_img, data_img, icf, dcf, filename) &&
	         /* Write *.ent and *.ext files: call with symbols from external references type or entry type only */
             write_external_file(externals, filename, ".ext") &&
            write_entries_file(entries, filename, ".ent");
	/* Release filtered tables */
	free_table(externals);
	free_table(entries);
	return result;
}

static bool write_ob(machine_word **code_img, long *data_img, long icf, long dcf, char *filename) {
	int i;
	long val;
	FILE *file_desc;
	/* add extension of file to open */
	char *output_filename = strcat_to_new(filename, ".ob");
	/* Try to open the file for writing */
	file_desc = fopen(output_filename, "w");
	if (file_desc == NULL) {
		printf("Can't create or rewrite to file %s.", output_filename);
        free(output_filename);
		return FALSE;
	}
    free(output_filename);

	/* print code image length and data2 image length */
	fprintf(file_desc, "%ld %ld\n", icf - IC_INIT_VALUE, dcf);


	/* starting from index 0, not IC_INIT_VALUE as icf, so we have to subtract it. */
	for (i = 0; i < icf - IC_INIT_VALUE; i++) {
        /* Place for '\0' */
        char special_base[MACHINE_WORD_LENGTH+1]="";
        /* Only first opcode wards contain length field */
        if (code_img[i]->length > 0) {
            opcode_word *opc = code_img[i]->word.opcode;
            /** Group A */
            /* placeholder is always 0 => 0 Opcode is always A => 100  ==> 0100 = 4 */
            special_base[0] = 'A';
            special_base[1] = '4';

            /** Group B-D rest of opcode */
            special_base[opc->opcode] =
            /** String end of line */
            special_base[20] = '\0';


        } else if (code_img[i]->is_operand == TRUE) {
            operand_word *opw  =code_img[i]->word.operand;
        } else{

        }
        fprintf(file_desc, "%s\n", special_base);
    }

	/* Write data2 image. dcf starts at 0 so it's fine */
	for (i = 0; i < dcf; i++) {
		/* print only lower 24 bytes */
		val = KEEP_ONLY_24_LSB(data_img[i]);
		/* print at least 6 digits of hex, and 7 digits of dc */
		fprintf(file_desc, "\n%.7ld %.6lx", icf + i, val);
	}

	/* Close the file */
	fclose(file_desc);
	return TRUE;
}

static bool write_entries_file(table tab, char *filename, char *file_extension) {
	FILE *file_desc;
	/* concatenate filename & extension, and open the file for writing: */
	char *full_filename = strcat_to_new(filename, file_extension);
	file_desc = fopen(full_filename, "w");
	/* if failed, print error and exit */
	if (file_desc == NULL) {
		printf("Can't create or rewrite to file %s.", full_filename);
        free(full_filename);
		return FALSE;
	}
    free(full_filename);
	/* if table is null, nothing to write */
	if (tab == NULL) return TRUE;

	/* Write first line without \n to avoid extraneous line breaks */
	fprintf(file_desc, "%s,%ld,%ld", tab->key, tab->base, tab->offset);
	while ((tab = tab->next) != NULL) {
		fprintf(file_desc, "\n%s,%ld,%ld", tab->key, tab->base, tab->offset);
	}
	fclose(file_desc);
	return TRUE;
}

bool write_external_file(table tab, char *filename, char *file_extension){
    FILE *file_desc;
    /* concatenate filename & extension, and open the file for writing: */
    char *full_filename = strcat_to_new(filename, file_extension);
    file_desc = fopen(full_filename, "w");
    /* if failed, print error and exit */
    if (file_desc == NULL) {
        printf("Can't create or rewrite to file %s.", full_filename);
        free(full_filename);
        return FALSE;
    }
    free(full_filename);
    /* if table is null, nothing to write */
    if (tab == NULL) return TRUE;

    /* Write first line without \n to avoid extraneous line breaks */
    fprintf(file_desc, "%s BASE %ld\n", tab->key, tab->base);
    fprintf(file_desc, "%s OFFSET %ld\n", tab->key, tab->offset);
    while ((tab = tab->next) != NULL) {
        fprintf(file_desc, "\n%s BASE %ld", tab->key, tab->base);
        fprintf(file_desc, "\n%s OFFSET %ld", tab->key, tab->offset);
        fprintf(file_desc,"\n");
    }
    fclose(file_desc);
    return TRUE;
}