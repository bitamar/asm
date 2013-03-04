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

void _parser_translate_command(void* data);

void translate();

/**
 *
 */
void _parser_translate_data();

/**
 * Create the entry symbols file.
 */
void parser_create_ent_file();

/**
 * Performs "Second phase" commands translation.
 */
void parser_translate_commands();

/**
 *
 */
void _parser_find_label_address(void* data);

#endif
