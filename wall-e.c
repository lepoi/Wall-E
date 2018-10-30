#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "types.h"
#include "messages.h"
#include "hashtable.h"
#include "instructions.h"

#define DECLARE_INSTRUCTION(opcode, label, body)\
	hash_item(state.ins, new_ht_item(opcode, label, body))

char declare_label(FILE *fp, struct asm_state *state, char *buffer) {
	struct ht_item *item;
	if (!lookup_item(state->labels, buffer))
		goto new;

	struct list_item *list = state->labels->list;
	long p;
	while (list) {
		printf("item: label = %s, opcode = %i, addr = %i\n",
			list->item->label, list->item->opcode, list->item->addr);
		if (list->item->opcode == 0) {
			p = ftell(state->fp_out);
			printf("ftell %ld\n", p);
			fseek(state->fp_out, list->item->addr, SEEK_SET);
			short diff = p - list->item->addr - 2;
			printf("diff %i\n", diff);
			fwrite(&diff, sizeof(short), 1, state->fp_out);
			fseek(state->fp_out, p, SEEK_SET);
		}

		list = list->next;
	}

	
new:
	item = new_ht_item(1, buffer, NULL);
	item->addr = (short) ftell(state->fp_out);
	hash_item(state->labels, item);
}

void assemble(FILE *fp, struct asm_state *state) {
	char buffer[16];
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

		fwrite(&ins->opcode, 1, 1, state->fp_out);

		if (ins->body)
			ins->body(fp, state);

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

	DECLARE_INSTRUCTION(1, "EXT", NULL);
	DECLARE_INSTRUCTION(2, "DCLI", dcl);
	DECLARE_INSTRUCTION(3, "DCLD", dcl);
	DECLARE_INSTRUCTION(4, "DCLC", dcl);
	DECLARE_INSTRUCTION(5, "DCLS", dcl);
	DECLARE_INSTRUCTION(6, "DCLVI", dcl);
	DECLARE_INSTRUCTION(7, "DCLVD", dcl);
	DECLARE_INSTRUCTION(8, "DCLVC", dcl);
	DECLARE_INSTRUCTION(9, "DCLVS", dcl);
	DECLARE_INSTRUCTION(10, "PUSH", var);
	DECLARE_INSTRUCTION(11, "PUSHV", var);
	DECLARE_INSTRUCTION(12, "PUSHKI", kint);
	DECLARE_INSTRUCTION(13, "PUSHKD", kdouble);
	DECLARE_INSTRUCTION(14, "PUSHKC", kchar);
	DECLARE_INSTRUCTION(15, "PUSHKS", kstring);
	DECLARE_INSTRUCTION(16, "POPI", var);
	DECLARE_INSTRUCTION(17, "POPD", var);
	DECLARE_INSTRUCTION(18, "POPC", var);
	DECLARE_INSTRUCTION(19, "POPS", var);
	DECLARE_INSTRUCTION(20, "POPVI", var);
	DECLARE_INSTRUCTION(21, "POPVD", var);
	DECLARE_INSTRUCTION(22, "POPVC", var);
	DECLARE_INSTRUCTION(23, "POPVS", var);
	DECLARE_INSTRUCTION(24, "ADD", NULL);
	DECLARE_INSTRUCTION(25, "SUB", NULL);
	DECLARE_INSTRUCTION(26, "MUL", NULL);
	DECLARE_INSTRUCTION(27, "DIV", NULL);
	DECLARE_INSTRUCTION(28, "MOD", NULL);
	DECLARE_INSTRUCTION(29, "JMP", label);
	DECLARE_INSTRUCTION(30, "JMPC", label);
	DECLARE_INSTRUCTION(31, "CEQ", NULL);
	DECLARE_INSTRUCTION(32, "CNE", NULL);
	DECLARE_INSTRUCTION(33, "CGT", NULL);
	DECLARE_INSTRUCTION(34, "CGE", NULL);
	DECLARE_INSTRUCTION(35, "CLT", NULL);
	DECLARE_INSTRUCTION(36, "CLE", NULL);
	DECLARE_INSTRUCTION(37, "RDI", NULL);
	DECLARE_INSTRUCTION(38, "RDD", NULL);
	DECLARE_INSTRUCTION(39, "RDC", NULL);
	DECLARE_INSTRUCTION(40, "RDS", NULL);
	DECLARE_INSTRUCTION(41, "WRT", NULL);
	DECLARE_INSTRUCTION(42, "WRTS", kstring);
	DECLARE_INSTRUCTION(43, "WRTLN", NULL);
	DECLARE_INSTRUCTION(44, "FREE", var);

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
	if (!state.fp_out)
		printf("Could not create output file\n");

	fwrite(MAGIC_NUMBER, sizeof(MAGIC_NUMBER) - 1, 1, state.fp_out);
	assemble(fp, &state);

	fclose(fp);
	fclose(state.fp_out);

	if (!error)
		if (rename("temp.bin", "out.eva"))
			printf("Could not move temporary to final output file\n");
	else 
		remove("temp.bin");

	return error;
}
