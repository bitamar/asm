/*
 * translator.h
 */

#ifndef TRANSLATOR_H_
#define TRANSLATOR_H_

#include "parser.h"

#define OpenFile(file, extension) {\
	char* file_name = reader_get_file_name(extension);\
	file = fopen(file_name, "w");\
	if (!file) {\
		fprintf(stderr, ErrorCantRead, file_name);\
		fprintf(stderr, "\n");\
		free(file_name);\
		exit(EXIT_FAILURE);\
	}\
	free(file_name);\
}

void translate();

/**
 * Create the entry symbols file.
 */
void parser_create_ent_file();

#endif
