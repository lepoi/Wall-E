#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "hashtable.h"

typedef unsigned short addr_t;

struct state_s {
	FILE *fp_out;
	addr_t data_size;
	addr_t exec_size;
	hashtable *var_addrs;
	hashtable *ins;
	unsigned int line_number;
};

#include "instructions.h"

#define MAGIC_NUMBER "\x00\x45\x56\x41\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
#define DATA_SEGMENT_OFFSET sizeof(MAGIC_NUMBER)
#define DATA_SEGMENT_OFFSETT DATA_SEGMENT_OFFSET + sizeof(addr_t)

#define DECLARE_INS(set, opcode, label, body) \
	hash_item(set, new_ht_item(opcode, label, body))	

#define DECLARE_VAR(set, var_name, addr) \
	hash_item(set, new_ht_item(addr, var_name, NULL, 0));

void assemble(FILE *fp, struct state_s *s) {
	char buffer[7];
	ht_item *ins;

	while (fscanf(fp, "%s", buffer) != -1) {
		ins = lookup_item(s->ins, buffer);

		if (!ins) {
			printf("Invalid instruction\n");
			return;
		}

		if (ins->opcode) {
			fwrite(&ins->opcode, 1, 1, s->fp_out);
		}

		if (ins->body) {
			ins->body(fp, s);
		}

		s->line_number++;
	}
}

int main(int argc, char *args[]) {
	FILE *fp;
	struct state_s s = {
		.ins = new_ht(64),
		.var_addrs = new_ht(64)
	};

	DECLARE_INS(s.ins, 0, "DCLI", dcli);
	DECLARE_INS(s.ins, 1, "EXT", NULL);
	DECLARE_INS(s.ins, 2, "PUSHI", var);
	DECLARE_INS(s.ins, 14, "POPI", var);
	DECLARE_INS(s.ins, 31, "RDI", NULL);

	if (!args[1]) {
		printf("Needs more arguments\n");
		return 1;
	}

	fp = fopen(args[1], "r");
	if (!fp) {
		printf("Invalid file\n");
		return 1;
	}
	
	s.fp_out = fopen("out.bin", "wb+");
	if (!s.fp_out) {
		printf("Could not create output file\n");
	}
	
	// write magic number
	fwrite(MAGIC_NUMBER, sizeof(MAGIC_NUMBER) - 1, 1, s.fp_out);
	// data and exec segment
	fwrite("\x00\x00\x00\x00", 4, 1, s.fp_out);
	assemble(fp, &s);

	fseek(s.fp_out, sizeof(MAGIC_NUMBER) - 1, SEEK_SET);
	fwrite(&s.data_size, sizeof(addr_t), 1, s.fp_out);
	fwrite(&s.exec_size, sizeof(addr_t), 1, s.fp_out);

	fclose(fp);
	fclose(s.fp_out);
	return 0;
}
