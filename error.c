#include "error.h"
#include <stdio.h>
#include <stdlib.h>

/**
 * Prints an error message and terminates the program.
 */
void error_fatal(const char* message) {
	fprintf(stderr, "%s:\n%s\n", ERROR_FATAL_ERROR, message);
	exit(EXIT_FAILURE);
}
