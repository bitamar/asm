#ifndef _ERROR_H
#define _ERROR_H

#define ErrorMessageMaxSize 100

#define ErrorTitle "Error:"
#define ErrorMissingArgument "Missing argument."
#define ErrorCantRead "Could not open file %s for reading."
#define ErrorMemoryAlloc "Memory allocation failed."

/**
 * Prints an error message and terminates the program.
 */
void error_fatal(const char* message);

/**
 * Adds an error to the errors list.
 * 
 * @param message
 *   Error message
 */
void error_set(const char* message);

/**
 * Prints the stored list of errors.
 */
void error_print_list();

/**
 * Destructor: Frees the error list.
 */
void error_destruct();

/**
 * Callback function for list_print(). Prints one error list item.
 * 
 * @param data
 *   The actual data will be a char* error message.
 */
void _error_print_item(void* data);

#endif
