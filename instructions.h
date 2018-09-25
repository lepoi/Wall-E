#define MAX_VAR_NAME_LENGTH 64

#define DECLARE_VAR(var_name, size)\
	hash_item(state->var_addrs, new_ht_item(state->count++, var_name, NULL));

static char *consume_var(FILE *fp, struct asm_state *state) {
	char *buffer = (char *) malloc(MAX_VAR_NAME_LENGTH);

	if (fscanf(fp, "%s", buffer) != 1) {
		printf("Invalid argument on line %i, expected variable\n", state->line_number);
		return NULL;
	}

	return buffer;
}

char dcl(FILE *fp, struct asm_state *state) {
	char *var_name = consume_var(fp, state);
	if (!var_name)
		return 1;

	if (lookup_item(state->var_addrs, var_name)) {
		printf("Variable already declared -> \"%s\"\n", var_name);
		return 1;
	}

	DECLARE_VAR(var_name, 0);
	addr_t addr = (addr_t) state->count - 1;
	fwrite(&addr, 2, 1, state->fp_out);
	state->exec_size += sizeof(addr_t);
	return 0;
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

char dcl_8(FILE *fp, struct asm_state *state) {
	char *var_name = consume_var(fp, state);
	if (!var_name)
		return 1;

	if (lookup_item(state->var_addrs, var_name)) {
		printf("Variable already declared -> \"%s\"\n", var_name);
		return 1;
	}

	DECLARE_VAR(var_name, 8);
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

char dcl_s(FILE *fp, struct asm_state *state) {
	char *var_name = consume_var(fp, state);
	if (!var_name)
		return 1;

	if (lookup_item(state->var_addrs, var_name)) {
		printf("Variable already decared -> \"%s\"\n", var_name);
		return 1;
	}

	char *buffer;
	if (fscanf(fp, "%s", buffer) != 1) {
		printf("String expected\n");
		return 1;
	}

	DECLARE_VAR(var_name, strlen(buffer));
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

char dclv_8(FILE *fp, struct asm_state *state) {
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

	DECLARE_VAR(var_name, buffer * 8);
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

char dclv_s(FILE *fp, struct asm_state *state) {
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

	DECLARE_VAR(var_name, buffer * 2);
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

char kdouble(FILE *fp, struct asm_state *state) {
	double buffer;

	if (fscanf(fp, "%lf", &buffer) != 1) {
		printf("Expected constant float\n");
		return 1;
	}

	fwrite(&buffer, sizeof(buffer), 1, state->fp_out);
	state->exec_size += sizeof(buffer);
	return 0;
}

char kchar(FILE *fp, struct asm_state *state) {
	char buffer[2];

	if (fscanf(fp, "%s", buffer) != 1) {
		printf("Expected constant char\n");
		return 1;
	}

	fwrite(&buffer[0], sizeof(char), 1, state->fp_out);
	printf("char: %c\n", buffer[0]);
	state->exec_size += sizeof(char);
	return 0;
}

char kstring(FILE *fp, struct asm_state *state) {
	char buffer[256];

	int r = fscanf(fp, " \"%256[^\"]\"", buffer);

	int len = strlen(buffer);
	fwrite(&buffer, len + 1, 1, state->fp_out);
	state->exec_size += len + 1;
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
		return 0;
	}

	addr_t addr = (addr_t) item->opcode;
	fwrite(&addr, 2, 1, state->fp_out);
	state->exec_size += sizeof(addr_t);
	return 0;
}
