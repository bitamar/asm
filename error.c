#include "error.h"
#include <stdio.h>
#include <stdlib.h>

/**
 * Prints an error message and terminates the program.
 */
void error_fatal(const char* message) {
	fprintf(stderr, "%s %s\n", ERROR_TITLE, message);
	exit(EXIT_FAILURE);
}
