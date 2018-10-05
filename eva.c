#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "vm_ht.h"

#define TYPE_INT		1
#define TYPE_DOUBLE		2
#define TYPE_CHAR		3
#define TYPE_STRING		4
#define TYPE_VINT		5
#define TYPE_VDOUBLE	6
#define TYPE_VCHAR		7
#define TYPE_VSTRING	8

#define EXT		1
#define DCLI	2
#define DCLD	3
#define DCLC	4
#define DCLS	5
#define DCLVI	6
#define DCLVD	7
#define DCLVC	8
#define DCLVS	9
#define PUSH	10
#define PUSHD	11
#define PUSHC	12
#define PUSHS	13
#define PUSHV	14
#define PUSHVD	15
#define PUSHVC	16
#define PUSHVS	17
#define PUSHKI	18
#define PUSHKD	19
#define PUSHKC	20
#define PUSHKS	21
#define POPI	22
#define POPD	23
#define POPC	24
#define POPS	25
#define POPVI	26
#define POPVD	27
#define POPVC	28
#define POPVS	29
#define ADD		30
#define	SUB		31
#define MUL		32
#define	DIV		33
#define MOD		34
#define JMP		35
#define JPEQ	36
#define JPNE	37
#define JPGT	38
#define JPGE	39
#define JPLT	40
#define JPLE	41
#define RDI		42
#define RDD		43
#define RDC		44
#define RDS		45
#define WRT		46
#define WRTS	47
#define WRTP	48
#define WRTLN	49

#define MAX_STACK_SIZE	64
#define MAX_STRING_SIZE 255

#define IS_NUMBER(type) type == TYPE_INT || type == TYPE_DOUBLE || type == TYPE_CHAR

struct vm_ht_item stack[MAX_STACK_SIZE];
u8 stack_size;
struct vm_ht *ht;

void error_exit(const char *msg) {
	printf("[ERROR] %s\n", msg);
	exit(1);
}

struct vm_ht_item *get_var(FILE *fp) {
	u16 id;
	fread(&id, sizeof(u16), 1, fp);

	struct vm_ht_item *item = vm_ht_get(ht, id);
	if (!item)
		error_exit("Variable not found");

	return item;
}

struct string *get_string(FILE *fp) {
	struct string *s = malloc(sizeof(struct string));

	char buffer[MAX_STRING_SIZE];
	char c;
	u8 i = 0;
	while ((c = fgetc(fp)) != '\x00')
		buffer[i++] = c;
	buffer[i] = '\x00';

	s->size = strlen(buffer);
	s->str = malloc(s->size);
	strcpy(s->str, buffer);

	return s;
}

void push(struct vm_ht_item *item) {
	if (stack_size == MAX_STACK_SIZE - 1)
		error_exit("Stack full");

	stack[stack_size++] = *item;
}

struct vm_ht_item pop() {
	if (stack_size == 0)
		error_exit("Stack empty");

	return stack[--stack_size];
}

inline void write(struct vm_ht_item item) {	
	switch(item.type) {
		case TYPE_INT: printf("%i", item.content.i); break;
		case TYPE_DOUBLE: printf("%lf", item.content.d); break;
		case TYPE_CHAR: printf("%c", item.content.c); break;
		case TYPE_STRING: printf("%s", item.content.s->str); break;
	}
}

u16 get_u16(FILE *fp) {
	u16 n;
	fread(&n, sizeof(n), 1, fp);

	return n;
}

u64 get_index(struct vm_ht_item *item) {
	u64 index;
	if (item->type == TYPE_STRING) {
		error_exit("Vector indexing with string is not supported (yet)");
		// index = stroul(item->content.s, NULL, 		
	}

	switch(item->type) {
		case TYPE_INT: index = item->content.i; break;
		case TYPE_DOUBLE: index = item->content.d; break;
		case TYPE_CHAR: index = item->content.c; break;
	}

	return index;
}

void jump(FILE *fp, u16 address) {
	fseek(fp, HEADER_OFFSET + address, SEEK_SET);
}

void run(FILE *fp) {
	fseek(fp, HEADER_OFFSET, SEEK_SET);
	char c;
	while ((c = fgetc(fp)) != EOF) {
		switch (c) {
			case EXT: exit(0); break;

			default: printf("default at: %lu -> [%x]", ftell(fp), c); break;

			case DCLI: {
				struct vm_ht_item *var = malloc(sizeof(struct vm_ht_item));
				var->id = get_u16(fp);
				var->type = TYPE_INT;

				vm_ht_add(ht, var);
			} break;

			case DCLD: {
				struct vm_ht_item *var = malloc(sizeof(struct vm_ht_item));
				var->id = get_u16(fp);
				var->type = TYPE_DOUBLE;

				vm_ht_add(ht, var);
			} break;

			case DCLC: {
				struct vm_ht_item *var = malloc(sizeof(struct vm_ht_item));
				var->id = get_u16(fp);
				var->type = TYPE_CHAR;

				vm_ht_add(ht, var);
			} break;

			case DCLS: {
				struct vm_ht_item *var = malloc(sizeof(struct vm_ht_item));
				var->id = get_u16(fp);
				var->type = TYPE_STRING;

				vm_ht_add(ht, var);
			} break;

			case DCLVI: {
				struct vm_ht_item *var = get_var(fp);
				var->id = get_u16(fp);
				var->type = TYPE_VINT;
				var->size = get_u16(fp);
				var->content.vi = malloc(sizeof(int) * var->size);
				
				vm_ht_add(ht, var);
			} break;

			case DCLVD: {
				struct vm_ht_item *var = get_var(fp);
				var->type = TYPE_VDOUBLE;
				var->size = get_u16(fp);
				var->content.vd = malloc(sizeof(double) * var->size);
				
				vm_ht_add(ht, var);
			} break;
		
			case DCLVC: {
				struct vm_ht_item *var = get_var(fp);
				var->id = get_u16(fp);
				var->type = TYPE_VCHAR;
				var->size = get_u16(fp);
				var->content.vc = malloc(sizeof(char) * var->size);
				
				vm_ht_add(ht, var);
			} break;

			case DCLVS: {
				struct vm_ht_item *var = get_var(fp);
				var->id = get_u16(fp);
				var->type = TYPE_VSTRING;
				var->size = get_u16(fp);
				var->content.vs = malloc(sizeof(struct string *) * var->size);

				vm_ht_add(ht,  var);
			} break;

			case JMP: {
				u16 address = get_u16(fp);
				jump(fp, address);
			} break;

			case ADD: {
				struct vm_ht_item b = pop();
				struct vm_ht_item a = pop();

				if (a.type != b.type)
					error_exit("Adding different types is not supported");

				switch(a.type) {
					case TYPE_INT: a.content.i += b.content.i; break;
					case TYPE_DOUBLE: a.content.d += b.content.d; break;
					case TYPE_STRING: {
						/*if (!realloc(a.content.s, a.size + b.size))
							error_exit("String reallication error");
						strcat(a.content.s.str, b.content.s.str);*/
					} break;
					default: error_exit("Invalid operands"); break;
				}

				push(&a);
			} break;

			case SUB: {
				struct vm_ht_item b = pop();
				struct vm_ht_item a = pop();

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
				struct vm_ht_item b = pop();
				struct vm_ht_item a = pop();

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
				struct vm_ht_item b = pop();
				struct vm_ht_item a = pop();

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
				struct vm_ht_item b = pop();
				struct vm_ht_item a = pop();

				if (a.type != b.type)
					error_exit("Adding different types is not supported");

				switch(a.type) {
					case TYPE_INT: a.content.i %= b.content.i; break;
					default: error_exit("Invalid operands"); break;
				}

				push(&a);
			} break;

			case RDI: {
				char buffer[64];
				fgets(buffer, 64, stdin);

				struct vm_ht_item *item = malloc(sizeof(struct vm_ht_item));
				item->type = TYPE_INT;
				item->content.i = atoi(buffer);

				push(item);
			} break;

			case RDD: {
				char buffer[64];
				fgets(buffer, 64, stdin);

				struct vm_ht_item *item = malloc(sizeof(struct vm_ht_item));
				item->type = TYPE_DOUBLE;
				item->content.d = atof(buffer);

				push(item);
			} break;

			case RDC: {
				char c = getchar();

				struct vm_ht_item *item = malloc(sizeof(struct vm_ht_item));
				item->type = TYPE_CHAR;
				item->content.c = c;

				push(item);
			} break;
	
			case RDS: {
				char *buffer = malloc(sizeof(char) * MAX_STRING_SIZE);
				fgets(buffer, MAX_STRING_SIZE, stdin);

				struct vm_ht_item *item = malloc(sizeof(struct vm_ht_item));
				item->type = TYPE_STRING;
				item->content.s->size = strlen(buffer);
				item->content.s->str = buffer;

				push(item);
			} break;

			case PUSHKI: {
				struct vm_ht_item *item = malloc(sizeof(struct vm_ht_item));
				item->type = TYPE_INT;

				int i;
				fread(&i, sizeof(int), 1, fp);
				item->content.i = i;

				push(item);
			} break;

			case PUSHKD: {
				struct vm_ht_item *item = malloc(sizeof(struct vm_ht_item));
				item->type = TYPE_DOUBLE;

				double d;
				fread(&d, sizeof(double), 1, fp);
				item->content.d = d;

				push(item);
			} break;

			case PUSHKC: {
				struct vm_ht_item *item = malloc(sizeof(struct vm_ht_item));
				item->type = TYPE_CHAR;

				char c;
				fread(&c, sizeof(char), 1, fp);
				item->content.c = c;

				push(item);
			} break;

			case PUSHKS: {
				struct vm_ht_item *item = malloc(sizeof(struct vm_ht_item));
				item->type = TYPE_STRING;
				item->content.s = get_string(fp);

				push(item);
			} break;

			case PUSH: {
				struct vm_ht_item *var = get_var(fp);
				struct vm_ht_item *item = malloc(sizeof(struct vm_ht_item));
				var->type = item->type;
				var->content = item->content;

				push(item);
			} break;

			case PUSHV: {
				struct vm_ht_item index_item = pop(fp);
				u16 index = get_index(&index_item);

				struct vm_ht_item *var = get_var(fp);
			} break;

			case WRT: {
				struct vm_ht_item *var = get_var(fp);
				write(*var);
			} break;

			case WRTS: {
				struct string *s = get_string(fp);
				printf("%s", s->str);
			} break;

			case WRTLN: printf("\n"); break;

			case WRTP: {
				struct vm_ht_item item = pop();
				write(item);
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
