#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "vm_ht.h"

#define TYPE_INT		0
#define TYPE_DOUBLE		1
#define TYPE_CHAR		2
#define TYPE_STRING		3

#define EXT		1
#define DCLI	2
#define DCLD	3
#define DCLC	4
#define DCLS	5
#define PUSH	6
#define PUSHI	7
#define PUSHD	8
#define PUSHC	9
#define PUSHS	10				
#define POP		11
#define WRT		12
#define WRTS	13
#define WRTLN	14
#define ADD		15
#define	SUB		16
#define MUL		17
#define	DIV		18
#define MOD		19

#define MAX_STACK_SIZE	64
#define MAX_STRING_SIZE 256

struct vm_ht_item stack[MAX_STACK_SIZE];
unsigned char stack_size;
struct vm_ht *ht;

void error_exit(const char *msg) {
	printf("[ERROR] %s\n", msg);
	exit(1);
}

struct vm_ht_item *get_var(FILE *fp) {
	unsigned short id;
	fread(&id, sizeof(unsigned short), 1, fp);

	struct vm_ht_item *item = vm_ht_get(ht, id);
	if (!item)
		error_exit("Variable not found");

	return item;
}

unsigned short get_id(FILE *fp) {
	unsigned short i;
	fread(&i, sizeof(unsigned short), 1, fp);

	return i;
}

void push(struct vm_ht_item *item) {
	if (stack_size == MAX_STACK_SIZE - 1)
		error_exit("Stack full");

	stack[stack_size++] = *item;
}

struct vm_ht_item pop(FILE *fp) {
	if (stack_size == 0)
		error_exit("Stack empty");

	return stack[--stack_size];
}

void run(FILE *fp) {
	fseek(fp, 20, SEEK_SET);
	char c;
	while ((c = fgetc(fp)) != EOF) {
		switch (c) {
			case EXT: break;

			default: printf("default at: %lu -> [%x]", ftell(fp), c); break;

			case JMP: {
				unsigned short address;
				fread(&address, sizeof(address), 1, fp);

				fseek(fp, 20 + address, SEEK_SET);
			} break;

			case JEQ: {
				struct vm_ht_item b = pop(fp);
				struct vm_ht_item a = pop(fp);

				if (a.type != b.type)
					error_exit("Different operands");

				switch(a.type) {
					
				}
				unsigned short address;
				fread(&address, sizeof(address), 1, fp);

				fseek(fp, 20 + address, SEEK_SET);
			} break;

			case ADD: {
				struct vm_ht_item b = pop(fp);
				struct vm_ht_item a = pop(fp);

				if (a.type != b.type)
					error_exit("Adding different types is not supported");

				switch(a.type) {
					case TYPE_INT: a.content.i += b.content.i; break;
					case TYPE_DOUBLE: a.content.d += b.content.d; break;
					case TYPE_STRING: {
						if (!realloc(a.content.s, a.size + b.size))
							error_exit("String reallication error");
						strcat(a.content.s, b.content.s);
					} break;
					default: error_exit("Invalid operands"); break;
				}

				push(&a);
			} break;

			case SUB: {
				struct vm_ht_item b = pop(fp);
				struct vm_ht_item a = pop(fp);

				if (a.type != b.type)
					error_exit("Adding different types is not supported");

				switch(a.type) {
					case TYPE_INT: a.content.i -= b.content.i; break;
					case TYPE_DOUBLE: a.content.d -= b.content.d; break;
					default: error_exit("Invalid operands"); break;
				}

				push(&a);
			} break;

			case MUL: {
				struct vm_ht_item b = pop(fp);
				struct vm_ht_item a = pop(fp);

				if (a.type != b.type)
					error_exit("Adding different types is not supported");

				switch(a.type) {
					case TYPE_INT: a.content.i *= b.content.i; break;
					case TYPE_DOUBLE: a.content.d *= b.content.d; break;
					default: error_exit("Invalid operands"); break;
				}

				push(&a);
			} break;

			case DIV: {
				struct vm_ht_item b = pop(fp);
				struct vm_ht_item a = pop(fp);

				if (a.type != b.type)
					error_exit("Adding different types is not supported");

				switch(a.type) {
					case TYPE_INT: a.content.i /= b.content.i; break;
					case TYPE_DOUBLE: a.content.d /= b.content.d; break;
					default: error_exit("Invalid operands"); break;
				}

				push(&a);
			} break;

			case MOD: {
				struct vm_ht_item b = pop(fp);
				struct vm_ht_item a = pop(fp);

				if (a.type != b.type)
					error_exit("Adding different types is not supported");

				switch(a.type) {
					case TYPE_INT: a.content.i %= b.content.i; break;
					default: error_exit("Invalid operands"); break;
				}

				push(&a);
			} break;

			case DCLI: {
				struct vm_ht_item *var = malloc(sizeof(struct vm_ht_item));
				var->id = get_id(fp);
				var->type = TYPE_INT;
				var->size = 4;

				vm_ht_add(ht, var);
			} break;

			case DCLD: {
				struct vm_ht_item *var = malloc(sizeof(struct vm_ht_item));
				var->id = get_id(fp);
				var->type = TYPE_DOUBLE;
				var->size = 8;

				vm_ht_add(ht, var);
			} break;

			case DCLC: {
				struct vm_ht_item *var = malloc(sizeof(struct vm_ht_item));
				var->id = get_id(fp);
				var->type = TYPE_CHAR;
				var->size = 1;

				vm_ht_add(ht, var);
			} break;

			case DCLS: {
				struct vm_ht_item *var = malloc(sizeof(struct vm_ht_item));
				var->id = get_id(fp);
				var->type = TYPE_STRING;
				var->size = 0;

				vm_ht_add(ht, var);
			} break;

			case PUSH: {
				push(get_var(fp));
			} break;

			case PUSHI: {
				struct vm_ht_item *item = malloc(sizeof(struct vm_ht_item));
				item->type = TYPE_INT;
				item->size = 4;

				int i;
				fread(&i, sizeof(int), 1, fp);
				item->content.i = i;

				push(item);
			} break;

			case PUSHD: {
				struct vm_ht_item *item = malloc(sizeof(struct vm_ht_item));
				item->type = TYPE_DOUBLE;
				item->size = 8;

				double d;
				fread(&d, sizeof(double), 1, fp);
				item->content.d = d;

				push(item);
			} break;

			case PUSHC: {
				struct vm_ht_item *item = malloc(sizeof(struct vm_ht_item));
				item->type = TYPE_CHAR;
				item->size = 1;

				char c;
				fread(&c, sizeof(char), 1, fp);
				item->content.c = c;

				push(item);
			} break;

			case PUSHS: {
				struct vm_ht_item *item = malloc(sizeof(struct vm_ht_item));
				item->type = TYPE_STRING;

				long offset = ftell(fp);
				char s[MAX_STRING_SIZE];
				fread(&s, sizeof(s), 1, fp);
				item->size = strlen(s);
				item->content.s = malloc(item->size);
				strcpy(item->content.s, s);
				
				fseek(fp, offset + item->size + 1, SEEK_SET);
				push(item);
			} break;

			case POP: {
				struct vm_ht_item *var = get_var(fp);
				struct vm_ht_item item = pop(fp);

				if (var->type != item.type)
					error_exit("Top of stack and variable types differ");

				switch(var->type) {
					case TYPE_INT: var->content.i = item.content.i; break;
					case TYPE_DOUBLE: var->content.d = item.content.d; break;
					case TYPE_CHAR: var->content.c = item.content.c; break;
					case TYPE_STRING: var->content.s = item.content.s; break;
				}
			} break;

			case WRT: {
				struct vm_ht_item *var = get_var(fp);

				switch(var->type) {
					case TYPE_INT: printf("%i", var->content.i); break;
					case TYPE_DOUBLE: printf("%lf", var->content.d); break;
					case TYPE_CHAR: printf("%c", var->content.c); break;
					case TYPE_STRING: printf("%s", var->content.s); break;
				}
			} break;

			case WRTS: {
				char s[256];
				char c;
				int i = 0;
				while ((c = fgetc(fp)) != '\x00') {
					s[i++] = c;
				}

				s[i] = '\x00';
				printf("%s", s);
			} break;

			case WRTLN: {
				printf("\n");
			} break;
		}
	}
}

int main() {
	FILE *fp = fopen("out.eva", "rb+");
	if (!fp) {
		printf("Input file not found\n");
		exit(1);
	}
	
	ht = new_vm_ht(64);
	run(fp);
}
