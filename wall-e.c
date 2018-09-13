#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "types.h"
#include "messages.h"
#include "hashtable.h"
#include "instructions.h"

#define MAGIC_NUMBER "\x00\x45\x56\x41\x00\x00\x00\x00\x00\x00\x00\x32\x38\x32\x30\x32"
#define DATA_SEGMENT_OFFSET sizeof(MAGIC_NUMBER) - 1

#define DECLARE_INSTRUCTION(opcode, label, body)\
	hash_item(state.ins, new_ht_item(opcode, label, body))

char declare_label(FILE *fp, struct asm_state *state, char *buffer) {
	struct ht_item *item = lookup_item(state->labels, buffer);
	if (item) {
		if (item->opcode != 0) {
			error_log(state->line_number, "Label "C_BLU"%s"C_RST" already declared", buffer);
			return 1;
		}
		else {
			item->opcode = ftell(state->fp_out);
			return 0;
		}
	}
	hash_item(state->labels, new_ht_item(ftell(state->fp_out), buffer, NULL));
	return 0;
}

void assemble(FILE *fp, struct asm_state *state) {
	char buffer[7];
	struct ht_item *ins;

	while (fscanf(fp, "%s", buffer) > 0) {
		ins = lookup_item(state->ins, buffer);

		int length = strlen(buffer);
		if (buffer[length - 1] == ':') {
			buffer[length - 1] = '\x00';
			declare_label(fp, state, buffer);
			goto next;
		}

		if (!ins) {
			error_log(state->line_number, "Instruction "C_BLU"%s"C_RST" not recognized", buffer);
			goto next;
		}

		if (ins->opcode) {
			fwrite(&ins->opcode, 1, 1, state->fp_out);
			state->exec_size += 1;
		}

		if (ins->body)
			if (!ins->body(fp, state))
				goto next;

next:
		state->line_number++;
	}
}

int main(int argc, char *args[]) {
	FILE *fp;
	char error = 0;
	struct asm_state state = {
		.line_number = 1,
		.ins = new_ht(64),
		.var_addrs = new_ht(64),
		.labels = new_ht(64)
	};

	/* 
	 * Instructions with opcode 0 will not be written to the binary file, as they
	 * are not read by the virtual machine.
	*/
	DECLARE_INSTRUCTION(0, "DCLI", dcl_4);
	DECLARE_INSTRUCTION(0, "DCLA", dcl_4);
	DECLARE_INSTRUCTION(0, "DCLC", dcl_1);
	// DECLARE_INSTRUCTION(0, "DCLS", dcl_s);
	DECLARE_INSTRUCTION(0, "DCLVI", dclv_4);
	DECLARE_INSTRUCTION(0, "DCLVA", dclv_4);
	DECLARE_INSTRUCTION(0, "DCLVC", dclv_1);
	// DECLARE_INSTRUCTION(0, "DCLVS", dcl_vs);
	DECLARE_INSTRUCTION(1, "EXT", NULL);
	DECLARE_INSTRUCTION(2, "PUSHI", var);
	DECLARE_INSTRUCTION(3, "PUSHD", var);
	DECLARE_INSTRUCTION(4, "PUSHC", var);
	DECLARE_INSTRUCTION(5, "PUSHS", var);
	DECLARE_INSTRUCTION(6, "PUSHKI", kint);
	DECLARE_INSTRUCTION(7, "PUSHKD", kfloat);
	DECLARE_INSTRUCTION(8, "PUSHKC", kchar);
	//DECLARE_INSTRUCTION(9, "PUSHKS", kstring);
	DECLARE_INSTRUCTION(10, "PUSHVI", var);
	DECLARE_INSTRUCTION(11, "PUSHVD", var);
	DECLARE_INSTRUCTION(12, "PUSHVC", var);
	//DECLARE_INSTRUCTION(13, "PUSHVS", var);
	DECLARE_INSTRUCTION(14, "POPI", var);
	DECLARE_INSTRUCTION(15, "POPD", var);
	DECLARE_INSTRUCTION(16, "POPC", var);
	DECLARE_INSTRUCTION(17, "POPS", var);
	DECLARE_INSTRUCTION(18, "POPV", var);
	DECLARE_INSTRUCTION(19, "POPX", var);
	DECLARE_INSTRUCTION(20, "POPXK", kint);
	DECLARE_INSTRUCTION(21, "MOVX", var);
	DECLARE_INSTRUCTION(22, "MOVXK", kint);
	DECLARE_INSTRUCTION(31, "RDI", NULL);
	DECLARE_INSTRUCTION(32, "RDD", NULL);
	DECLARE_INSTRUCTION(33, "RDC", NULL);
	DECLARE_INSTRUCTION(34, "RDS", NULL);
	DECLARE_INSTRUCTION(35, "RDV", NULL);
	DECLARE_INSTRUCTION(36, "WRTLN", NULL);
	DECLARE_INSTRUCTION(37, "WRTI", NULL);
	DECLARE_INSTRUCTION(38, "WRTD", NULL);
	DECLARE_INSTRUCTION(39, "WRTC", NULL);
	DECLARE_INSTRUCTION(40, "WRTS", NULL);
	DECLARE_INSTRUCTION(41, "WRTM", NULL);
	DECLARE_INSTRUCTION(42, "WRTV", NULL);
	DECLARE_INSTRUCTION(43, "JMP", label);
	DECLARE_INSTRUCTION(44, "JPEQ", label);
	DECLARE_INSTRUCTION(45, "JPNT", label);
	DECLARE_INSTRUCTION(46, "JPGT", label);
	DECLARE_INSTRUCTION(47, "JPGE", label);
	DECLARE_INSTRUCTION(48, "JPLT", label);
	DECLARE_INSTRUCTION(49, "JPLE", label);
	DECLARE_INSTRUCTION(50, "ADD", NULL);
	DECLARE_INSTRUCTION(51, "SUB", NULL);
	DECLARE_INSTRUCTION(52, "MUL", NULL);
	DECLARE_INSTRUCTION(53, "DIV", NULL);
	DECLARE_INSTRUCTION(54, "MOD", NULL);
	DECLARE_INSTRUCTION(55, "INC", NULL);
	DECLARE_INSTRUCTION(56, "DEC", NULL);

	if (!args[1]) {
		printf("Needs more arguments\n");
		return 1;
	}

	fp = fopen(args[1], "r");
	if (!fp) {
		printf("Invalid file\n");
		return 1;
	}

	state.fp_out = fopen("temp.bin", "wb+");
	if (!state.fp_out) {
		printf("Could not create output file\n");
	}

	// write magic number
	fwrite(MAGIC_NUMBER, sizeof(MAGIC_NUMBER) - 1, 1, state.fp_out);
	// data and exec segment
	fwrite("\x00\x00\x00\x00", 4, 1, state.fp_out);

	assemble(fp, &state);

	struct list_item *list = state.labels->list;
	while (list) {
		if (list->item->opcode == 0) {
			// print label name and/or address
			printf("Invalid label found\n");
			error = 1;
		}
		if (error)
			break;

		list = list->next;
	}


	fseek(state.fp_out, sizeof(MAGIC_NUMBER) - 1, SEEK_SET);
	fwrite(&state.data_size, sizeof(addr_t), 1, state.fp_out);
	fwrite(&state.exec_size, sizeof(addr_t), 1, state.fp_out);

	fclose(fp);
	fclose(state.fp_out);

	if (!error)
		if (rename("temp.bin", "out.eva"))
			printf("Could not move temporary to final output file\n");
	else 
		remove("temp.bin");

	return error;
}
