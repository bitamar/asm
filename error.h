#ifndef _ERROR_H
#define _ERROR_H

#define ErrorMessageMaxSize 100

#define ErrorMissingArgument "Missing argument."
#define ErrorCantRead "Could not open file %s for reading."
#define ErrorMemoryAlloc "Memory allocation failed."

#define error_set(msg, line) fprintf(stderr, "Error in %s, on line %d: %s\n", reader_get_file_name(), line, msg)

/**
 * Prints an error message and terminates the program.
 */
void error_fatal(const char* message);

#endif
