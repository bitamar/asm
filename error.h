#ifndef _ERROR_H
#define _ERROR_H

#define ErrorTitle "Error:"
#define ErrorMissingArgument "Missing argument."
#define ErrorCantRead "Could not open file for reading."
#define ErrorMemoryAlloc "Memory allocation failed."

/**
 * Prints an error message and terminates the program.
 */
void error_fatal(const char* message);

#endif
