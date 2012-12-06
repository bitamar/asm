/*
 * reader.c
 *
 *	Required by: parser
 *  Created on: Dec 5, 2012
 *      Author: Itamar Bar-Lev
 */

#include "reader.h"
#include "error.h"
#include <stdio.h>
#include <stdlib.h>

FILE* file;

/**
 * Open a file for reading.
 * Store the file pointer in the reader's global variable.
 */
void reader_open_file(const char* file_name) {
	file = fopen(file_name, "r");
	if (!file)
		error_fatal(ERROR_CANT_READ);
}

/**
 * Read the next line from a file.
 * The returned string must be freed by the invoker.
 */
char* reader_get_line() {
	int length = READER_LINE_LENGTH_INIT;
	int position = 0;
	char *line = (char*)malloc(length);

	if (!line)
		error_fatal(ERROR_MEMORY_ALLOC);

	do {
	    line[position++] = getc(file);

	    /* If the position is divisible by the initial size, then more space is
	     * needed for the line. */
	    if(!(position %  READER_LINE_LENGTH_INIT)) {
	    	/* Increase the line length by the initial size. */
			length += READER_LINE_LENGTH_INIT;
			line = (char*)realloc(line, length);
			if (!line)
				error_fatal(ERROR_MEMORY_ALLOC);
		}
	} while (line[position - 1] != '\n' && line[position - 1] != EOF);

	if (line[0] == EOF)
		return NULL;

	line[position - 1] = '\0';
	return line;
}

/**
 * Close the reader's file.
 */
void reader_close_file() {
	fclose(file);
}
