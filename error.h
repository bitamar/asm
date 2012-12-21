#ifndef _ERROR_H
#define _ERROR_H

#define ERROR_TITLE "Error:"
#define ERROR_MISSING_ARGUMENT "Missing argument."
#define ERROR_CANT_READ "Could not open file for reading."
#define ERROR_MEMORY_ALLOC "Memory allocation failed."

/**
 * Prints an error message and terminates the program.
 */
void error_fatal(const char* message);

#endif
