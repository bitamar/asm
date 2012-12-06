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

/**
 * Does the initial parsing of the assembly file.
 * A file must be opened using reader
 */
void parser_parse();

#endif /* PARSER_H_ */
