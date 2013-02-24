#ifndef _READER_H
#define _READER_H

/**
 *  The initial size of the string that will hold a line of the file. If the row
 *  exceeds the initial length, more space will be allocated.
 */
#define ReaderLineLengthInit 20

#define ReaderFileExtension "as"

/**
 * Open a file for reading.
 */
void reader_open_file(const char* file_name);

/**
 * Close the reader's file.
 */
void reader_close_file();

/**
 * Get the currenly open file's name.
 * @param extension
 *   Extension to attach to the file name.
 */
char* reader_get_file_name(const char* extension);

/**
 * Read the next line from a file.
 * The returned string must be freed by the invoker.
 */
char* reader_get_line();

#endif
