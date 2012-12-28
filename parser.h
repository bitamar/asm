/*
 * parser.h
 *
 *  Depends on: reader
 *  Required by: main
 *  Created on: Dec 5, 2012
 *      Author: Itamar Bar-Lev
 */

#ifndef PARSER_H_
#define PARSER_H_

enum ParserLabelValues {
	MAIN = 100,
	LOOP = 105,
	END = 117,
	STR = 118,
	LENGTH = 125,
	K = 0
};

#define ParserLabelsAmount 6

#define ParserNoLabel -1

/**
 * Does the initial parsing of the assembly file.
 * A file must be opened using reader
 */
void parser_parse();

/**
 * Get the ID of a label from a line.
 */
short int parser_line_get_label(const char* line);

/**
 * Check whether a line has a specific label.
 * Returns 1 or 0.
 */
short int parser_line_has_label(const char* line, const char* label);

#endif /* PARSER_H_ */
