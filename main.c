#include "error.h"
#include "parser.h"
#include "reader.h"
#include <stdlib.h>

/**
 * Main is responsible for initiating the file reading and parsing.
 */
int main(int argc, char* argv[]) {
	int i;
	
	if (argc < 2)
		error_fatal(ErrorMissingArgument);
	
	/* Open and read files. */
	for (i = 1; i < argc; i++) {
		reader_open_file(argv[i]);
		parser_parse();
		reader_close_file();

		/* Perform "Second iteration". */
		parser_create_ent_file();
		parser_translate_commands();

		printf("File %s assembled successfully.\n", reader_get_file_name(ReaderFileExtension));

		/* Clean data. */
		parser_clean();
	}

	return EXIT_SUCCESS;
}
