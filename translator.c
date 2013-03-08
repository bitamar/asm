#include "error.h"
#include "reader.h"
#include "translator.h"
#include "utils.h"
#include <stdlib.h>

FILE* _translate_output_files[3];

void _translate_line(void* _line_data) {
	LineData* line_data = _line_data;
	Label* label_found = NULL;
	Label* dummy_label;
	long address;
	long data;
	char code[CODE_SIZE + 1];

	if (line_data->label_to_extract) {
		NewLabel(dummy_label);
		dummy_label->label = line_data->label_to_extract;
		label_found = list_find_item(parser_data.parser_symbols, dummy_label, &_parser_compare_labels);
		free(dummy_label);

		line_data->are = label_found && label_found->label_type == LABEL_TYPE_EXTERN ? 'e' : 'r';

		if (!label_found) {
			fprintf(stderr, "Error: Label \"%s\" not found\n", line_data->label_to_extract);
		}
		else if (label_found->line) {
			line_data->line_word.data = label_found->line + LINE_OFFSET - 1;
			if (line_data->is_instruction || label_found->label_type == LABEL_TYPE_DATA)
				line_data->line_word.data += parser_data.IC;
		}
		else if (label_found->label_type == LABEL_TYPE_EXTERN && !line_data->is_instruction)
			fprintf(_translate_output_files[EXT_FILE], "%s\t%d\n", label_found->label, base4(line_data->decimal_address + LINE_OFFSET - 1));
	}

	data = line_data->line_word.data;
	if (line_data->is_instruction) {
		data = line_data->line_word.inst.comb + 4 * line_data->line_word.inst.dest_reg + 32 * line_data->line_word.inst.dest_address + 128 * line_data->line_word.inst.src_reg + 1024 * line_data->line_word.inst.src_address + 4096 * line_data->line_word.inst.opcode + 65536 * line_data->line_word.inst.type;
	}

	/* Avoid adding the offset when the address is zero. */
	address = line_data->decimal_address + LINE_OFFSET - 1;
	if (!line_data->are)
		address += parser_data.IC;
	fprintf(_translate_output_files[OB_FILE], "%d\t\t\t\t%s\t\t%c\n", base4(address), base4code(data, code), line_data->are);
}

void _translate_commands() {
	OpenFile(_translate_output_files[EXT_FILE], "ext");
	OpenFile(_translate_output_files[OB_FILE], "ob");

	fprintf(_translate_output_files[OB_FILE], "\t\t\t\t\t%d\t%d\n", base4(parser_data.IC), base4(parser_data.DC));
	list_foreach(parser_data.commands_list, &_translate_line);
	list_foreach(parser_data.data_list, &_translate_line);

	fclose(_translate_output_files[EXT_FILE]);
	fclose(_translate_output_files[OB_FILE]);
}

void _translate_find_label_address(void* data) {
	unsigned long address;

	Label* label = data;
	Label* label_found;
	label_found = list_find_item(parser_data.parser_symbols, label, &_parser_compare_labels);
	if (!label_found) {
		fprintf(stderr, "Error in %s: Entry label not defined.\n", reader_get_file_name(ReaderFileExtension));
		return;
	}
	address = label_found->line + LINE_OFFSET - 1;
	if (label_found->label_type == LABEL_TYPE_DATA)
		address += parser_data.IC;

	fprintf(_translate_output_files[ENT_FILE], "%s\t%d\n", label_found->label, base4(address));
}

void _translate_create_ent_file() {
	OpenFile(_translate_output_files[ENT_FILE], "ent");
	list_foreach(parser_data.parser_entry_symbols, &_translate_find_label_address);
	fclose(_translate_output_files[ENT_FILE]);
}

void translate() {
	_translate_create_ent_file();
	_translate_commands();
}
