/*
 * reader.c
 *
 *	Required by: parser
 *
 *  Created on: Dec 5, 2012
 *      Author: Itamar Bar-Lev
 */

#include "reader.h"
#include "error.h"
#include <stdio.h>
#include <stdlib.h>

FILE* reader_file;

/**
 * Open a file for reading.
 * Store the file pointer in the reader's global variable.
 */
void reader_open_file(const char* file_name) {
	reader_file = fopen(file_name, "r");
	if (!reader_file) {
		fprintf(stderr, ErrorCantRead, file_name);
		exit(EXIT_FAILURE);
	}
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

	if (line[0] == EOF)
		return NULL;

	line[position - 1] = '\0';
	return line;
}

/**
 * Close the reader's file.
 */
void reader_close_file() {
	fclose(reader_file);
}
