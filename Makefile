# Basic compilation macros
CC = gcc
# Mandatory flags for mmn14
CFLAGS = -ansi -Wall -pedantic
# Holds global variables, consts and enums that used in all the project
GLOBAL_CONSTS = globals.h
# Executable dependencies
EXE_DEPS = assembler.o opcode_builder.o first_pass.o second_pass.o instruction_builder.o symbol_table.o helper.o output_module.o linkedlist.o pre_assembler.o

# Executable
assembler: $(EXE_DEPS) $(GLOBAL_CONSTS)
	$(CC) -g $(EXE_DEPS) $(CFLAGS) -o $@

# Main:
assembler.o: assembler.c $(GLOBAL_CONSTS)
	$(CC) -c assembler.c $(CFLAGS) -o $@

# Code helper functions:
opcode_builder.o: opcode_builder.c opcode_builder.h $(GLOBAL_CONSTS)
	$(CC) -c code.c $(CFLAGS) -o $@

# First pass main:
first_pass.o: first_pass.c first_pass.h $(GLOBAL_CONSTS)
	$(CC) -c first_pass.c $(CFLAGS) -o $@

# Second pass main:
second_pass.o: second_pass.c second_pass.h $(GLOBAL_CONSTS)
	$(CC) -c second_pass.c $(CFLAGS) -o $@

# Instructions helper functions:
instruction_builder.o: instruction_builder.c instruction_builder.h $(GLOBAL_CONSTS)
	$(CC) -c instruction_builder.c $(CFLAGS) -o $@

# Symbol table:
symbol_table.o: symbol_table.c symbol_table.h $(GLOBAL_CONSTS)
	$(CC) -c symbol_table.c $(CFLAGS) -o $@

## Useful functions:
helper.o: helper.c instruction_builder.h $(GLOBAL_CONSTS)
	$(CC) -c helper.c $(CFLAGS) -o $@

## macro expansion functions:
pre_assembler.o: pre_assembler.c pre_assembler.h $(GLOBAL_CONSTS)
	$(CC) -c pre_assembler.c $(CFLAGS) -o $@

## linkedlist for pre_assembler functions:
linkedlist.o: linkedlist.c linkedlist.h $(GLOBAL_CONSTS)
	$(CC) -c linkedlist.c $(CFLAGS) -o $@

## Output module to handle files:
output_module.o: output_module.c output_module.h $(GLOBAL_CONSTS)
	$(CC) -c output_module.c $(CFLAGS) -o $@

# clean compilation leftovers if we decide to recompile
clean:
	rm -rf *.o