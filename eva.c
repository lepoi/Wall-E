#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "vm_ht.h"

#define TYPE_INT		0
#define TYPE_DOUBLE		1
#define TYPE_CHAR		2
#define TYPE_STRING		3
#define TYPE_VINT		4
#define TYPE_VDOUBLE	5
#define TYPE_VCHAR		6
#define TYPE_VSTRING	7

#define EXT		1
#define DCLI	2
#define DCLD	3
#define DCLC	4
#define DCLS	5
#define PUSH	6
#define PUSHV	7
#define PUSHI	8
#define PUSHD	9
#define PUSHC	10
#define PUSHS	11				
#define POP		12
#define POPV	13
#define WRT		15
#define WRTS	16
#define WRTLN	17
#define ADD		18
#define	SUB		19
#define MUL		20
#define	DIV		21
#define MOD		22
#define JMP		23
#define JPEQ	24
#define JPNE	25
#define JPGT	26
#define JPGE	27
#define JPLT	28
#define JPLE	29
#define DCLVI	30
#define DCLVD	31
#define DCLVC	32
#define DCLVS	33
#define RDI		34
#define RDD		35
#define RDC		36
#define RDS		37
#define WRTP	38

#define MAX_STACK_SIZE	64
#define MAX_STRING_SIZE 256

typedef unsigned short u16;
typedef unsigned char u8;

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
		case TYPE_STRING: printf("%s", item.content.s); break;
	}
}

u16 get_u16(FILE *fp) {
	u16 n;
	fread(&n, sizeof(n), 1, fp);

	return n;
}

void jump(FILE *fp, u16 address) {
	fseek(fp, 20 + address, SEEK_SET);
}

void run(FILE *fp) {
	fseek(fp, 20, SEEK_SET);
	char c;
	while ((c = fgetc(fp)) != EOF) {
		switch (c) {
			case EXT: exit(0); break;

			default: printf("default at: %lu -> [%x]", ftell(fp), c); break;

			case DCLVI: {
				struct vm_ht_item *var = get_var(fp);
				var->type = TYPE_VINT;
				var->size = get_u16(fp);
				var->vi = malloc(sizeof(int) * var->size);
				
				vm_ht_add(ht, var);
			} break;

			case DCLVI: {
				struct vm_ht_item *var = get_var(fp);
				var->type = TYPE_VINT;
				var->size = get_u16(fp);
				var->vi = malloc(sizeof(int) * var->size);
				
				vm_ht_add(ht, var);
			} break;
		
			case DCLVD: {
				struct vm_ht_item *var = get_var(fp);
				var->type = TYPE_VINT;
				var->size = get_u16(fp);
				var->vi = malloc(sizeof(int) * var->size);
				
				vm_ht_add(ht, var);
			} break;

			case DCLVS: {
				struct vm_ht_item *var = get_var(fp);
				var->type = TYPE_VSTRING;
				var->size = sizeof(char **);
				var->vs = malloc(sizeof(char **));

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
						if (!realloc(a.content.s, a.size + b.size))
							error_exit("String reallication error");
						strcat(a.content.s, b.content.s);
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
				item->size = 4;
				item->content.i = atoi(buffer);

				push(item);
			} break;

			case RDD: {
				char buffer[64];
				fgets(buffer, 64, stdin);

				struct vm_ht_item *item = malloc(sizeof(struct vm_ht_item));
				item->type = TYPE_DOUBLE;
				item->size = 8;
				item->content.d = atof(buffer);

				push(item);
			} break;

			case RDC: {
				char c = getchar();

				struct vm_ht_item *item = malloc(sizeof(struct vm_ht_item));
				item->type = TYPE_CHAR;
				item->size = 1;
				item->content.c = c;

				push(item);
			} break;
	
			case RDS: {
				char *buffer = malloc(sizeof(char) * 64);
				fgets(buffer, 64, stdin);

				struct vm_ht_item *item = malloc(sizeof(struct vm_ht_item));
				item->type = TYPE_STRING;
				item->size = strlen(buffer);
				item->content.s = buffer;

				push(item);
			} break;

			case DCLI: {
				struct vm_ht_item *var = malloc(sizeof(struct vm_ht_item));
				var->id = get_u16(fp);
				var->type = TYPE_INT;
				var->size = 4;

				vm_ht_add(ht, var);
			} break;

			case DCLD: {
				struct vm_ht_item *var = malloc(sizeof(struct vm_ht_item));
				var->id = get_u16(fp);
				var->type = TYPE_DOUBLE;
				var->size = 8;

				vm_ht_add(ht, var);
			} break;

			case DCLC: {
				struct vm_ht_item *var = malloc(sizeof(struct vm_ht_item));
				var->id = get_u16(fp);
				var->type = TYPE_CHAR;
				var->size = 1;

				vm_ht_add(ht, var);
			} break;

			case DCLS: {
				struct vm_ht_item *var = malloc(sizeof(struct vm_ht_item));
				var->id = get_u16(fp);
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
				struct vm_ht_item item = pop();

				var->content = item.content;
			} break;

			case WRT: {
				struct vm_ht_item *var = get_var(fp);
				write(*var);
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
