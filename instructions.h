#define MAX_VAR_NAME_LENGTH 64

#define DECLARE_VAR(var_name, size)\
	hash_item(state->var_addrs, new_ht_item(state->data_size, var_name, NULL));\
	state->data_size += size

static char *consume_var(FILE *fp, struct asm_state *state) {
	char *buffer = (char *) malloc(MAX_VAR_NAME_LENGTH);

	if (fscanf(fp, "%s", buffer) != 1) {
		printf("Invalid argument on line %i, expected variable\n", state->line_number);
		return NULL;
	}

	return buffer;
}

char dcl_4(FILE *fp, struct asm_state *state) {
	char *var_name = consume_var(fp, state);
	if (!var_name)
		return 1;

	if (lookup_item(state->var_addrs, var_name)) {
		printf("Variable already declared -> \"%s\"\n", var_name);
		return 1;
	}

	DECLARE_VAR(var_name, 4);
	return 0;
}

char dcl_1(FILE *fp, struct asm_state *state) {
	char *var_name = consume_var(fp, state);
	if (!var_name)
		return 1;

	if (lookup_item(state->var_addrs, var_name)) {
		printf("Variable already declared -> \"%s\"\n", var_name);
		return 1;
	}

	DECLARE_VAR(var_name, 1);
	return 0;
}

char dclv_4(FILE *fp, struct asm_state *state) {
	char *var_name = consume_var(fp, state);
	if (!var_name)
		return 1;

	if (lookup_item(state->var_addrs, var_name)) {
		printf("Variable already declared -> \"%s\"\n", var_name);
		return 1;
	}

	int buffer;	
	if (fscanf(fp, "%i", &buffer) != 1) {
		printf("Int expected\n");
		return 1;
	}

	DECLARE_VAR(var_name, buffer * 4);
	return 0;
}

char dclv_1(FILE *fp, struct asm_state *state) {
	char *var_name = consume_var(fp, state);
	if (!var_name)
		return 1;

	if (lookup_item(state->var_addrs, var_name)) {
		error_log(state->line_number, "Variable "C_BLU"%s"C_RST" already declared", var_name);
		return 1;
	}

	int buffer;	
	if (fscanf(fp, "%i", &buffer) != 1) {
		error_log(state->line_number, "Expected int constant");
		printf("Int expected\n");
		return 1;
	}

	DECLARE_VAR(var_name, buffer);
	return 0;
}

char kint(FILE *fp, struct asm_state *state) {
	int buffer;

	if (fscanf(fp, "%i", &buffer) != 1) {
		printf("Expected constant int\n");
		return 1;
	}

	fwrite(&buffer, sizeof(buffer), 1, state->fp_out);
	state->exec_size += sizeof(buffer);
	return 0;
}

char kfloat(FILE *fp, struct asm_state *state) {
	float buffer;

	if (fscanf(fp, "%f", &buffer) != 1) {
		printf("Expected constant float\n");
		return 1;
	}

	fwrite(&buffer, sizeof(buffer), 1, state->fp_out);
	state->exec_size += sizeof(buffer);
	return 0;
}

char kchar(FILE *fp, struct asm_state *state) {
	char buffer;

	if (fscanf(fp, "%c", &buffer) != 1) {
		printf("Expected constant long\n");
		return 1;
	}

	fwrite(&buffer, sizeof(buffer), 1, state->fp_out);
	state->exec_size += sizeof(buffer);
	return 0;
}

char kstring(FILE *fp, struct asm_state *state) {
	char buffer[64];

	if (fscanf(fp, "%s", buffer) != 1) {
		printf("Expected constant string");
		return 1;
	}

	fwrite(&buffer, strlen(buffer), 1, state->fp_out);
	state->exec_size += strlen(buffer);
	return 0;
}

char var(FILE *fp, struct asm_state *state) {
	char *var_name = consume_var(fp, state);
	struct ht_item *item = lookup_item(state->var_addrs, var_name);
	if (!item) {
		printf("Variable not declared -> \"%s\"\n", var_name);
		return 1;
	}

	addr_t addr = (addr_t) item->opcode;
	fwrite(&addr, 2, 1, state->fp_out);
	state->exec_size += sizeof(addr_t);
	return 0;
}

char label(FILE *fp, struct asm_state *state) {
	char *label_name = consume_var(fp, state);
	struct ht_item *item = lookup_item(state->labels, label_name);
	if (!item) {
		struct ht_item *add = new_ht_item(0, label_name, NULL);
		hash_item(state->labels, add);
	}
	return 0;
}
