#include "error.h"
#include "reader.h"
#include "translator.h"
#include "utils.h"
#include <stdlib.h>

FILE* output[3];

void translate() {
	parser_create_ent_file();
	parser_translate_commands();
}

void parser_translate_commands() {
	OpenFile(output[EXT_FILE], "ext");
	OpenFile(output[OB_FILE], "ob");

	fprintf(output[OB_FILE], "\t\t\t\t\t%d\t%d\n", base4(parser_data.IC), base4(parser_data.DC));
	list_foreach(parser_data.commands_list, &_parser_translate_command);
	list_foreach(parser_data.data_list, &_parser_translate_data);

	fclose(output[EXT_FILE]);
	fclose(output[OB_FILE]);
}

void parser_create_ent_file() {
	OpenFile(output[ENT_FILE], "ent");
	list_foreach(parser_data.parser_entry_symbols, &_parser_find_label_address);
	fclose(output[ENT_FILE]);
}

void _parser_translate_line(LineData* line_data, unsigned int extra_address_offset) {
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
			line_data->line_word.data = label_found->line + LINE_OFFSET - 1 + extra_address_offset;
			if (label_found->label_type == LABEL_TYPE_DATA)
				line_data->line_word.data += parser_data.IC;
		}
		else if (label_found->label_type == LABEL_TYPE_EXTERN && !extra_address_offset)
			fprintf(output[EXT_FILE], "%s\t%d\n", label_found->label, base4(line_data->decimal_address + LINE_OFFSET - 1));
	}

	data = line_data->line_word.data;
	if (line_data->is_instruction == 1) {
		data = line_data->line_word.inst.comb + 4 * line_data->line_word.inst.dest_reg + 32 * line_data->line_word.inst.dest_address + 128 * line_data->line_word.inst.src_reg + 1024 * line_data->line_word.inst.src_address + 4096 * line_data->line_word.inst.opcode + 8192 * line_data->line_word.inst.type;
	}

	/* Avoid adding the offset when the address is zero. */
	address = line_data->decimal_address + LINE_OFFSET - 1 + extra_address_offset;
	fprintf(output[OB_FILE], "%d\t\t\t\t%s\t\t%c\n", base4(address), base4code(data, code), line_data->are);
}

void _parser_translate_data(void* data) {
	LineData* line_data = data;
	_parser_translate_line(line_data, parser_data.IC);
}

void _parser_translate_command(void* data) {
	LineData* line_data = data;
	_parser_translate_line(line_data, 0);
}

void _parser_find_label_address(void* data) {
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

	fprintf(output[ENT_FILE], "%s\t%d\n", label_found->label, base4(address));
}
