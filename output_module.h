/* Output files related functions */
#ifndef OUTPUT_MODULE_H
#define OUTPUT_MODULE_H
#include "globals.h"
#include "symbol_table.h"

/**
 * Writes the output files of a single assembled file
 * @param code_img The code image
 * @param data_img The data image
 * @param icf The final instruction counter
 * @param dcf The final data counter
 * @param filename The filename (without the extension)
 * @param ent_table The entries table
 * @param ext_table The external references table
 * @return True if good False if bad
 */
int write_output_files(machine_word **code_img, long *data_img, long icf, long dcf, char *filename,
                       table symbol_table);


/***
 * Output the lines after macro expansion to file
 * @param lines_to_write
 * @param filename
 */
void write_macro_file(simple_node* lines_to_write, char* filename);

#endif