#ifndef _ERROR_H
#define _ERROR_H

#define ErrorMessageMaxSize 100

#define ErrorMissingArgument "Missing argument."
#define ErrorCantRead "Could not open file %s for reading."
#define ErrorMemoryAlloc "Memory allocation failed."

#define error_set(type, msg, line) {\
		fprintf(stderr, "%s in %s, on line %d: %s\n", type, reader_get_file_name(ReaderFileExtension), line, msg);\
		parser_data.errors++;\
	}

/**
 * Prints an error message and terminates the program.
 */
void error_fatal(const char* message);

#endif
