/*
 * reader.c
 */

#include "error.h"
#include "reader.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

FILE* reader_file;
/* The name of the file being currently read, without extension. */
char* reader_file_name_base;

/**
 * Open a file for reading.
 * Store the file pointer in the reader's global variable.
 */
void reader_open_file(const char* file_name) {
	unsigned int len;
	char* full_file_name;
	/* Create a string with the file name and extension. */
	len = strlen(file_name);
	reader_file_name_base = (char*)malloc(len + 1);
	if (!reader_file_name_base)
		error_fatal(ErrorMemoryAlloc);
	strcpy(reader_file_name_base, file_name);

	full_file_name = reader_get_file_name(ReaderFileExtension);
	reader_file = fopen(full_file_name, "r");

	if (!reader_file) {
		fprintf(stderr, ErrorCantRead, full_file_name);
		free(full_file_name);
		exit(EXIT_FAILURE);
	}
	free(full_file_name);
}

char* reader_get_file_name(const char* extension) {
	char* file_name;
	unsigned int full_length, base_length = strlen(reader_file_name_base);
	full_length = base_length + strlen(extension) + 1;
	file_name = (char*)malloc(full_length);
	if (!file_name)
		error_fatal(file_name);

	strcpy(file_name, reader_file_name_base);
	/* Add the file extension. */
	file_name[base_length] = '.';
	strcpy(file_name + base_length + 1, extension);
	file_name[full_length] = '\0';

	return file_name;
}

/**
 * Read the next line from a file.
 * The returned string must be freed by the invoker.
 */
char* reader_get_line() {
	int length = ReaderLineLengthInit;
	int position = 0;
	char *line = (char*)malloc(length);

	if (!line)
		error_fatal(ErrorMemoryAlloc);

	do {
	    line[position++] = getc(reader_file);

	    /* If the position is divisible by the initial size, then more space is
	     * needed for the line. */
	    if(!(position % ReaderLineLengthInit)) {
	    	/* Increase the line length by the initial size. */
			length += ReaderLineLengthInit;
			line = (char*)realloc(line, length);
			if (!line)
				error_fatal(ErrorMemoryAlloc);
		}
	} while (line[position - 1] != '\n' && line[position - 1] != EOF);

	if (line[0] == EOF) {
		free(line);
		return NULL;
	}

	line[position - 1] = '\0';
	return line;
}

/**
 * Close the reader's file.
 */
void reader_close_file() {
	fclose(reader_file);
}
