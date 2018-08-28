#define MAX_VAR_NAME_LENGTH 64

char *consume_var(FILE *fp, struct asm_state *state) {
	char *buffer = (char *) malloc(MAX_VAR_NAME_LENGTH);

	if (fscanf(fp, "%s", buffer) != 1) {
		printf("Invalid argument on line %i, expected variable int\n", state->line_number);
		return NULL;
	}

	return buffer;
}

void dcli(FILE *fp, struct asm_state *state) {
	char *var_name = consume_var(fp, state);
	if (!var_name) {
		return;
	}

	struct ht_item *item = new_ht_item(state->data_size, var_name, NULL);
	state->data_size += 4;
	hash_item(state->var_addrs, item);
}

void var(FILE *fp, struct asm_state *state) {
	char *var_name = consume_var(fp, state);
	struct ht_item *item = lookup_item(state->var_addrs, var_name);

	addr_t addr = (addr_t) item->opcode;
	fwrite(&addr, 2, 1, state->fp_out);
	state->exec_size += sizeof(addr_t);
}
