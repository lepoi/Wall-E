#define NEW_INS(name, opcode, args) \{name, opcode, args\}
#define MAX_VAR_NAME_LENGTH 64

typedef unsigned short addr_t;

char *consume_var(FILE *fp, struct state_s *s) {
	char *buffer = (char *) malloc(MAX_VAR_NAME_LENGTH);

	if (fscanf(fp, "%s", buffer) != 1) {
		printf("Invalid argument on line %i, expected variable int\n", s->line_number);
		return NULL;
	}

	return buffer;
}

void dcli(FILE *fp, struct state_s *s) {
	char *var_name = consume_var(fp, s);
	if (!var_name) {
		return;
	}

	ht_item *item = new_ht_item(s->data_size, var_name, NULL);
	s->data_size += 4;
	hash_item(s->var_addrs, item);
}

void var(FILE *fp, struct state_s *s) {
	char *var_name = consume_var(fp, s);

	ht_item *item = lookup_item(s->var_addrs, var_name);

	addr_t addr = (addr_t) item->opcode;
	fwrite(&addr, 2, 1, s->fp_out);
}
