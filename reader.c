/*
 * reader.c
 *
 *	Required by: 
 */

#include "error.h"
#include "reader.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

FILE* reader_file;

/**
 * Open a file for reading.
 * Store the file pointer in the reader's global variable.
 */
void reader_open_file(const char* file_name) {
	int len;
	char* full_file_name;

	/* Create a string with the file name and extension. */
	len = strlen(file_name);
	full_file_name = (char*) malloc(len + ReaderFileExtensionLength + 1);
	if (!full_file_name)
		error_fatal(ErrorMemoryAlloc);

	strcpy(full_file_name, file_name);
	full_file_name[len] = '.';
	strcpy(full_file_name + len + 1, ReaderFileExtension);
	full_file_name[len + ReaderFileExtensionLength + 1] = '\0';

	reader_file = fopen(full_file_name, "r");
	if (!reader_file) {
		fprintf(stderr, ErrorCantRead, full_file_name);
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
	} while (line[position] != '\n' && line[position] != EOF);
	if (line[position] = '\n')
		getc(reader_file);

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
