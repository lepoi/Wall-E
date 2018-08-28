#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "types.h"
#include "messages.h"
#include "hashtable.h"
#include "instructions.h"

#define MAGIC_NUMBER "\x00\x45\x56\x41\x00\x00\x00\x00\x00\x00\x00\x32\x38\x32\x30\x32"
#define DATA_SEGMENT_OFFSET sizeof(MAGIC_NUMBER) - 1

#define DECLARE_INS(set, opcode, label, body)\
	hash_item(set, new_ht_item(opcode, label, body))

char declare_label(FILE *fp, struct asm_state *state, char *buffer) {
	if (lookup_item(state->labels, buffer)) {
		return 1;
	}

	hash_item(state->labels, new_ht_item(ftell(fp), buffer, NULL));
	return 0;
}

void assemble(FILE *fp, struct asm_state *s) {
	char buffer[7];
	struct ht_item *ins;

	while (fscanf(fp, "%s", buffer) > 0) {
		ins = lookup_item(s->ins, buffer);

		if (buffer[strlen(buffer) - 1] == ':') {
			declare_label(fp, s, buffer);
			goto next;
		}

		if (!ins) {
			printf("Invalid instruction -> \"%s\"\n", buffer);
			goto next;
		}

		if (ins->opcode) {
			fwrite(&ins->opcode, 1, 1, s->fp_out);
			s->exec_size += 1;
		}

		if (ins->body)
			if (!ins->body(fp, s))
				goto next;

next:
		s->line_number++;
	}
}

int main(int argc, char *args[]) {
	FILE *fp;
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
	DECLARE_INS(state.ins, 0, "DCLI", dcl_4);
	DECLARE_INS(state.ins, 0, "DCLF", dcl_4);
	DECLARE_INS(state.ins, 0, "DCLC", dcl_1);
	// DECLARE_INS(state.ins, 0, "DCLS", dcl_s);
	DECLARE_INS(state.ins, 0, "DCLVI", dclv_4);
	DECLARE_INS(state.ins, 0, "DCLVF", dclv_4);
	DECLARE_INS(state.ins, 0, "DCLVC", dclv_1);
	// DECLARE_INS(state.ins, 0, "DCLVS", dcl_vs);
	DECLARE_INS(state.ins, 1, "EXT", NULL);
	DECLARE_INS(state.ins, 2, "PUSHI", var);
	DECLARE_INS(state.ins, 3, "PUSHF", var);
	DECLARE_INS(state.ins, 4, "PUSHC", var);
	DECLARE_INS(state.ins, 5, "PUSHS", var);
	DECLARE_INS(state.ins, 6, "PUSHKI", kint);
	DECLARE_INS(state.ins, 7, "PUSHKF", kfloat);
	DECLARE_INS(state.ins, 8, "PUSHKC", kchar);
	DECLARE_INS(state.ins, 9, "PUSHKS", kstring);
	DECLARE_INS(state.ins, 10, "PUSHVI", var);
	DECLARE_INS(state.ins, 11, "PUSHVF", var);
	DECLARE_INS(state.ins, 12, "PUSHVC", var);
	DECLARE_INS(state.ins, 13, "PUSHVS", var);
	DECLARE_INS(state.ins, 14, "POPI", var);
	DECLARE_INS(state.ins, 15, "POPF", var);
	DECLARE_INS(state.ins, 16, "POPC", var);
	DECLARE_INS(state.ins, 17, "POPS", var);
	DECLARE_INS(state.ins, 18, "POPV", var);
	DECLARE_INS(state.ins, 19, "POPX", var);
	DECLARE_INS(state.ins, 20, "POPXK", kint);
	DECLARE_INS(state.ins, 21, "MOVX", var);
	DECLARE_INS(state.ins, 22, "MOVXK", kint);
	DECLARE_INS(state.ins, 31, "RDI", NULL);
	DECLARE_INS(state.ins, 32, "RDF", NULL);
	DECLARE_INS(state.ins, 33, "RDC", NULL);
	DECLARE_INS(state.ins, 34, "RDS", NULL);
	DECLARE_INS(state.ins, 35, "RDV", NULL);
	DECLARE_INS(state.ins, 36, "WRTLN", NULL);
	DECLARE_INS(state.ins, 37, "WRTI", NULL);
	DECLARE_INS(state.ins, 38, "WRTF", NULL);
	DECLARE_INS(state.ins, 39, "WRTC", NULL);
	DECLARE_INS(state.ins, 40, "WRTS", NULL);
	DECLARE_INS(state.ins, 41, "WRTM", NULL);
	DECLARE_INS(state.ins, 42, "WRTV", NULL);
	DECLARE_INS(state.ins, 43, "JMP", label);
	DECLARE_INS(state.ins, 44, "JPEQ", label);
	DECLARE_INS(state.ins, 45, "JPNT", label);
	DECLARE_INS(state.ins, 46, "JPGT", label);
	DECLARE_INS(state.ins, 47, "JPGE", label);
	DECLARE_INS(state.ins, 48, "JPLT", label);
	DECLARE_INS(state.ins, 49, "JPLE", label);
	DECLARE_INS(state.ins, 50, "ADD", NULL);
	DECLARE_INS(state.ins, 51, "SUB", NULL);
	DECLARE_INS(state.ins, 52, "MUL", NULL);
	DECLARE_INS(state.ins, 53, "DIV", NULL);
	DECLARE_INS(state.ins, 54, "MOD", NULL);
	DECLARE_INS(state.ins, 55, "INC", NULL);
	DECLARE_INS(state.ins, 56, "DEC", NULL);

	if (!args[1]) {
		printf("Needs more arguments\n");
		return 1;
	}

	fp = fopen(args[1], "r");
	if (!fp) {
		printf("Invalid file\n");
		return 1;
	}
	
	state.fp_out = fopen("out.bin", "wb+");
	if (!state.fp_out) {
		printf("Could not create output file\n");
	}
	
	// write magic number
	fwrite(MAGIC_NUMBER, sizeof(MAGIC_NUMBER) - 1, 1, state.fp_out);
	// data and exec segment
	fwrite("\x00\x00\x00\x00", 4, 1, state.fp_out);
	assemble(fp, &state);

	fseek(state.fp_out, sizeof(MAGIC_NUMBER) - 1, SEEK_SET);
	fwrite(&state.data_size, sizeof(addr_t), 1, state.fp_out);
	fwrite(&state.exec_size, sizeof(addr_t), 1, state.fp_out);

	fclose(fp);
	fclose(state.fp_out);
	return 0;
}
