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
 *  Addressing code are 0 not in use, 1 in use
 * source opperand and destination_opperand are 0 if not in use , and 1 if in use
 */
typedef struct {
	unsigned int code : 4;
	unsigned int num_of_args : 2;
	unsigned int source_imidiat_addressing : 1;
	unsigned int source_direct_addressing : 1;
	unsigned int source_index_addressing : 1;
	unsigned int source_direct_register_addressing : 1;
	unsigned int destination_imidiat_addressing : 1;
	unsigned int destination_direct_addressing : 1;
	unsigned int destination_index_addressing : 1;
	unsigned int destination_direct_register_addressing : 1;
	unsigned int source_opperand : 1;
	unsigned int destination_opperand : 1;
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
char* parser_get_label(const char* line, int line_num);

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
 * Callback function for list_add_ordered(); Issues error message when a 
 * duplicated label is declared.
 *  
 * @param data
 *   Pointer to Label.
 */
void _parser_duplicated_label(void* data);

/**
 * Callback function for list_print(). Prints one label list item.
 * 
 * @param data
 *   Pointer to label.
 */
void _parser_print_label(void* data);

#endif /* PARSER_H_ */
