/*
 * parser.h
 *
 *  Depends on: reader
 *  Required by: main
 */

#ifndef PARSER_H_
#define PARSER_H_

#define MAX_LABEL_SIZE 30

/**
 *  Structure holding assembler instruction set.
 */
struct instructions {
	unsigned int code : 4;
	unsigned int num_of_args : 2;
	char *instruction;
};

/**
 * Does the initial parsing of the assembly file.
 * A file must be opened using reader
 */
void parser_parse();

/**
 * Check whether a line starts with a label.
 * Returns the label as a string. The string must be freed by the invoker.
 */
char* parser_get_label(const char* line);

#endif /* PARSER_H_ */
