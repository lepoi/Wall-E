#define MAX_VAR_NAME_LENGTH 64

#define DECLARE_VAR(var_name, size)\
	hash_item(state->var_addrs, new_ht_item(state->count++, var_name, NULL));

static char *get_name(FILE *fp, struct asm_state *state) {
	char *buffer = (char *) malloc(MAX_VAR_NAME_LENGTH);

	if (fscanf(fp, "%s", buffer) != 1) {
		printf("Invalid argument on line %i, expected variable\n", state->line_number);
		return NULL;
	}

	return buffer;
}

char dcl(FILE *fp, struct asm_state *state) {
	char *var_name = get_name(fp, state);
	if (!var_name)
		return 1;

	if (lookup_item(state->var_addrs, var_name)) {
		printf("Variable already declared -> \"%s\"\n", var_name);
		return 1;
	}

	u16 var_id = state->count;
	hash_item(state->var_addrs, new_ht_item(state->count++, var_name, NULL));
	fwrite(&var_id, sizeof(var_id), 1, state->fp_out);
	return 0;
}

char kint(FILE *fp, struct asm_state *state) {
	int buffer;

	if (fscanf(fp, "%i", &buffer) != 1) {
		printf("[kint] Expected constant int\n");
		return 1;
	}

	fwrite(&buffer, sizeof(int), 1, state->fp_out);
	return 0;
}

char kdouble(FILE *fp, struct asm_state *state) {
	double buffer;

	if (fscanf(fp, "%lf", &buffer) != 1) {
		printf("Expected constant float\n");
		return 1;
	}

	fwrite(&buffer, sizeof(double), 1, state->fp_out);
	return 0;
}

char kchar(FILE *fp, struct asm_state *state) {
	char buf[3];

	if (fscanf(fp, " \'%2s\'", buf) != 1) {
		printf("Expected constant char\n");
		return 1;
	}
	
	char c;
	if (buf[0] == '\\') {
		switch(buf[1]) {
			case 'n': c = '\n'; break;
			default: printf("Expected constant char\n"); break;
		}
	} else
		c = buf[0];

	fwrite(&c, sizeof(char), 1, state->fp_out);
	return 0;
}

char kstring(FILE *fp, struct asm_state *state) {
	char buf[257];
	char wbuf[257];
	int i = 0;
	int wi = 0;

	fscanf(fp, " \"%256[^\"]\"", buf);
	char c;
	while ((c = buf[i++]) != '\0') {
		if (c == '\\') {
			switch (c = buf[i++]) {
				case 'n': c = '\n'; break;
				default: c = '\\'; i--; break;
			}
		}
		
		wbuf[wi++] = c;
	}
	wbuf[wi] = '\0';
	
	fwrite(wbuf, strlen(wbuf) + 1, 1, state->fp_out);
	return 0;
}

char var(FILE *fp, struct asm_state *state) {
	char *var_name = get_name(fp, state);
	struct ht_item *item = lookup_item(state->var_addrs, var_name);
	if (!item) {
		printf("Variable not declared -> \"%s\"\n", var_name);
		return 1;
	}

	fwrite(&item->opcode, sizeof(u16), 1, state->fp_out);
	return 0;
}

char label(FILE *fp, struct asm_state *state) {
	char *label_name = get_name(fp, state);	
	struct ht_item *item = lookup_item(state->labels, label_name);
	if (!item || item->opcode == 0) {
		struct ht_item *add = new_ht_item(0, label_name, NULL);
		add->addr = ftell(state->fp_out);
		rm_ht_item(state->labels, label_name);
		hash_item(state->labels, add);
		short addr = 0;
		fwrite(&addr, sizeof(short), 1, state->fp_out);
		return 0;
	}

	printf("label %s jump to: %i\n", item->label, item->addr);
	short addr = item->addr - ftell(state->fp_out);
	fwrite(&addr, sizeof(short), 1, state->fp_out);
	return 0;
}
