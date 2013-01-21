/*
 * parser.h
 *
 *  Depends on: reader
 *  Required by: main
 * 
 *  Created on: Dec 5, 2012
 *      Author: Itamar Bar-Lev
 */

#ifndef PARSER_H_
#define PARSER_H_

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
