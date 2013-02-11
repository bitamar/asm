#include "error.h"
#include "list.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * List of errors found while parsing files.
 */
List* error_list;

/**
 * Prints an error message and terminates the program.
 */
void error_fatal(const char* message) {
	fprintf(stderr, "%s %s\n", ErrorTitle, message);
	exit(EXIT_FAILURE);
}

void error_set(const char* message) {
	/* Allocate memory for the message, to make it persistent. */
	char* _message = (char*)malloc(strlen(message) + 1);
	if (!_message)
		error_fatal(ErrorMemoryAlloc);
	strcpy(_message, message);
	
	error_list = list_append(error_list, _message);
}

void error_print_list() {
	list_print(error_list, &_error_print_item);
}

void _error_print_item(void* data) {
	fprintf(stderr, "Error: %s\n", (char*)data);
}