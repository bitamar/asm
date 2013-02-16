/*
 * parser.h
 *
 *  Depends on: reader
 *  Required by: main
 */

#ifndef PARSER_H_
#define PARSER_H_

#define MAX_LABEL_SIZE 30
/* =2^19-1 */
#define MAX_DATA_NUMBER 524287 
/* =2^20-2^19 */
#define MIN_DATA_NUMBER -524288 

/**
 *  Assembler instruction set.
 */
typedef struct {
	unsigned int code : 4;
	unsigned int num_of_args : 2;
	char *instruction;
} Instruction;

/**
 * The data stored on a labels-list's node.
 */
typedef struct {
	char* label;
	unsigned int line; 
} Label;

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

/**
 * Callback function for list_add_ordered(); Performs lexicographical comparison 
 * of two labels
 * 
 * @param a
 *   Pointer to Label.
 * @param Label b
 *   Pointer to Label.
 * 
 * @return 
 *   positive number if the first label's name is "larger" than the second's 
 *   name.  
 */
int _parser_compare_labels(void* a, void* b);

/**
 * Callback function for list_print(). Prints one label list item.
 * 
 * @param data
 *   Pointer to label.
 */
void _parser_print_label(void* data);

#endif /* PARSER_H_ */
