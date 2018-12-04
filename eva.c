#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "vm_ht.h"

#define CHAR		1
#define INT			2
#define DOUBLE		3
#define STRING		4
#define VCHAR		5
#define VINT		6
#define VDOUBLE		7
#define VSTRING		8

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
#define PUSHV	11
#define PUSHKI	12
#define PUSHKD	13
#define PUSHKC	14
#define PUSHKS	15
#define POPI	16
#define POPD	17
#define POPC	18
#define POPS	19
#define POPVI	20
#define POPVD	21
#define POPVC	22
#define POPVS	23
#define ADD		24
#define	SUB		25
#define MUL		26
#define	DIV		27
#define MOD		28
#define JMP		29
#define JMPC	30
#define CEQ		31
#define CNE		32
#define CGT		33
#define CGE		34
#define CLT		35
#define CLE		36
#define RDI		37
#define RDD		38
#define RDC		39
#define RDS		40
#define WRT		41
#define WRTS	42
#define WRTLN	43
#define FREE	44		

#define MAX_STACK_SIZE	64
#define MAX_STRING_SIZE 255

struct vm_ht_item stack[MAX_STACK_SIZE];
u8 stack_size;
struct vm_ht *ht;

void error_exit(const char *msg) {
	printf("[ERROR] %s\n", msg);
	exit(1);
}

struct string *new_string() {
	struct string *s = malloc(sizeof(struct string));
	s->str = malloc(1);
	s->str[0] = '\x00';

	return s;
}

char to_char(struct vm_ht_item item) {
	char ret;

	switch(item.type) {
		case INT: ret = (char) item.content.i; break;
		case DOUBLE: ret = (char) item.content.d; break;
		case CHAR: ret = item.content.c; break;
		case STRING: ret = item.content.s->str[0]; break;
		default: error_exit("[to_char] Invalid operands"); break;
	}

	return ret;
}

int to_int(struct vm_ht_item item) {
	int ret;

	switch(item.type) {
		case INT: ret = item.content.i; break;
		case DOUBLE: ret = (int) item.content.d; break;
		case CHAR: ret = (int) item.content.c; break;
		case STRING: ret = atoi(item.content.s->str); break;
		default: error_exit("[to_int] Invalid operands"); break;
	}

	return ret;
}

double to_double(struct vm_ht_item item) {
	double ret;

	switch(item.type) {
		case INT: ret = (double) item.content.i; break;
		case DOUBLE: ret = item.content.d; break;
		case CHAR: ret = (double) item.content.c; break;
		case STRING: ret = atof(item.content.s->str); break;
		default: error_exit("[to_double] Invalid operands"); break;
	}

	return ret;
}

struct string *to_string(struct vm_ht_item item) {
	struct string *s = new_string();
	int ret;
	char buffer[MAX_STRING_SIZE];

	switch(item.type) {
		case INT: ret = snprintf(buffer, MAX_STRING_SIZE, "%i", item.content.i); break;
		case DOUBLE: ret = snprintf(buffer, MAX_STRING_SIZE, "%lf", item.content.d); break;
		case CHAR: ret = snprintf(buffer, MAX_STRING_SIZE, "%c", item.content.c); break;
		case STRING:
			s->str = malloc(item.content.s->size);
			strcpy(s->str, item.content.s->str);
			s->size = item.content.s->size;
			return s;
		break;
	}

	if (ret < 0 || ret > MAX_STRING_SIZE)
		error_exit("[to_string] snprintf failed");
	
	s->str = malloc(ret);
	strcpy(s->str, buffer);
	s->size = ret;
	return s;
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
	int i = 0;
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

static inline u8 highest_precision(struct vm_ht_item a, struct vm_ht_item b) {
	return a.type > b.type ? a.type : b.type;
}

void write(struct vm_ht_item item) {	
	switch(item.type) {
		case INT: printf("%i", item.content.i); break;
		case DOUBLE: printf("%lf", item.content.d); break;
		case CHAR: printf("%c", item.content.c); break;
		case STRING: printf("%s", item.content.s->str); break;
	}
}

u16 get_u16(FILE *fp) {
	u16 n;
	fread(&n, sizeof(n), 1, fp);

	return n;
}

short get_short(FILE *fp) {
	short n;
	fread(&n, 2, 1, fp);

	return n;
}

int get_int(FILE *fp) {
	int n;
	fread(&n, 4, 1, fp);

	return n;
}

void check_exist(u16 id) {
	if (vm_ht_get(ht, id))
		error_exit("Redeclaration of variable");
}

void jump(FILE *fp, short addr) {
	if (addr < 0)
		addr -= 3;
	fseek(fp, addr, SEEK_CUR);
}

struct vm_ht_item *new_vm_ht_item(u8 type) {
	struct vm_ht_item *ret = malloc(sizeof(struct vm_ht_item));
	ret->type = type;
	ret->size = 0;
	return ret;
}

void run(FILE *fp) {
	char c;
	while ((c = fgetc(fp)) != EOF) {
		switch (c) {
			case EXT: exit(0); break;

			default: printf("default at: %lu -> [%x]\n", ftell(fp), c); break;

			case DCLI: {
				u16 id = get_u16(fp);
				check_exist(id);

				struct vm_ht_item *var = new_vm_ht_item(INT);
				var->id = id;

				vm_ht_add(ht, var);
			} break;

			case DCLD: {
				u16 id = get_u16(fp);
				check_exist(id);

				struct vm_ht_item *var = new_vm_ht_item(DOUBLE);
				var->id = id;

				vm_ht_add(ht, var);
			} break;

			case DCLC: {
				u16 id = get_u16(fp);
				check_exist(id);

				struct vm_ht_item *var = new_vm_ht_item(CHAR);
				var->id = id;

				vm_ht_add(ht, var);
			} break;

			case DCLS: {
				u16 id = get_u16(fp);
				check_exist(id);

				struct vm_ht_item *var = new_vm_ht_item(STRING);
				var->id = id;
				var->content.s = new_string();

				vm_ht_add(ht, var);
			} break;

			case DCLVI: {
				u16 id = get_u16(fp);
				check_exist(id);

				struct vm_ht_item *var = new_vm_ht_item(VINT);
				var->id = id;
				var->size = to_int(pop());
				var->content.vi = malloc(sizeof(int) * var->size);

				vm_ht_add(ht, var);
			} break;

			case DCLVD: {
				u16 id = get_u16(fp);
				check_exist(id);

				struct vm_ht_item *var = new_vm_ht_item(VDOUBLE);
				var->id = id;
				var->size = to_int(pop());
				var->content.vd = malloc(sizeof(double) * var->size);

				vm_ht_add(ht, var);
			} break;
		
			case DCLVC: {
				struct vm_ht_item *var = new_vm_ht_item(VCHAR);
				u16 id = get_u16(fp);
				check_exist(id);

				var->id = id;
				var->size = to_int(pop());
				var->content.vc = malloc(sizeof(char) * var->size);
				
				vm_ht_add(ht, var);
			} break;

			case DCLVS: {
				u16 id = get_u16(fp);
				check_exist(id);

				struct vm_ht_item *var = new_vm_ht_item(VSTRING);
				var->id = id;
				var->size = to_int(pop());
				var->content.vs = malloc(sizeof(struct string *) * var->size);

				vm_ht_add(ht,  var);
			} break;

			case PUSH: {
				struct vm_ht_item *var = get_var(fp);
				struct vm_ht_item *item = malloc(sizeof(struct vm_ht_item));
				item->type = var->type;
				item->content = var->content;

				push(item);
			} break;

			case PUSHV: {
				struct vm_ht_item *var = get_var(fp);
				int index = to_int(pop());
				if (index > var->size)
					error_exit("[PUSHV] Index out of bounds");

				struct vm_ht_item *item = new_vm_ht_item(var->type - 4);	
				switch(var->type) {
					case VCHAR: item->content.c = var->content.vc[index]; break;
					case VINT: item->content.i = var->content.vi[index]; break;
					case VDOUBLE: item->content.d = var->content.vd[index]; break;
					case VSTRING: item->content.s = var->content.vs[index]; break;
					default: error_exit("[PUSHV] Default case"); break;
				}

				push(item);
			} break;

			case PUSHKI: {
				struct vm_ht_item *item = new_vm_ht_item(INT);
				fread(&item->content.i, sizeof(int), 1, fp);

				push(item);
			} break;

			case PUSHKD: {
				struct vm_ht_item *item = new_vm_ht_item(DOUBLE);
				fread(&item->content.d, sizeof(double), 1, fp);

				push(item);
			} break;

			case PUSHKC: {
				struct vm_ht_item *item = new_vm_ht_item(CHAR);
				fread(&item->content.c, sizeof(char), 1, fp);

				push(item);
			} break;

			case PUSHKS: {
				struct vm_ht_item *item = new_vm_ht_item(STRING);
				item->content.s = get_string(fp);

				push(item);
			} break;

			case POPI: {
				struct vm_ht_item item = pop();
				struct vm_ht_item *var = get_var(fp);

				var->content.i = to_int(item);
			} break;

			case POPD: {
				struct vm_ht_item item = pop();
				struct vm_ht_item *var = get_var(fp);

				var->content.d = to_double(item);
			} break;

			case POPC: {
				struct vm_ht_item item = pop();
				struct vm_ht_item *var = get_var(fp);

				var->content.c = to_char(item);
			} break;

			case POPS: {
				struct vm_ht_item item = pop();
				struct vm_ht_item *var = get_var(fp);

				var->content.s = to_string(item);
			} break;

			case POPVI: {
				struct vm_ht_item *var = get_var(fp);
				int index = to_int(pop());
				if (index > var->size)
					error_exit("Index out of bounds");

				var->content.vi[index] = to_int(pop());
			} break;

			case POPVD: {
				struct vm_ht_item *var = get_var(fp);
				int index = to_int(pop());
				if (index > var->size)
					error_exit("Index out of bounds");

				var->content.vd[index] = to_double(pop());
			} break;

			case POPVC: {
				struct vm_ht_item *var = get_var(fp);
				int index = to_int(pop());
				if (index > var->size)
					error_exit("Index out of bounds");

				var->content.vc[index] = to_char(pop());
			} break;

			case POPVS: {
				struct vm_ht_item *var = get_var(fp);
				int index = to_int(pop());
				if (index > var->size)
					error_exit("Index out of bounds");

				var->content.vs[index] = to_string(pop());
			} break;

			case ADD: {
				struct vm_ht_item b = pop();
				struct vm_ht_item a = pop();

				struct vm_ht_item *r = new_vm_ht_item(highest_precision(a, b));
				switch(r->type) {
					case CHAR: r->content.c = to_char(a) + to_char(b); break;
					case INT: r->content.i = to_int(a) + to_int(b); break;
					case DOUBLE: r->content.d = to_double(a) + to_double(b); break;
					case STRING: {
						struct string *a_s = to_string(a);
						struct string *b_s = to_string(b);
						r->content.s = malloc(sizeof(struct string *));
						r->content.s->size = a_s->size + b_s->size;
						r->content.s->str = malloc(r->content.s->size);
						strcpy(r->content.s->str, a_s->str);
						strcat(r->content.s->str, b_s->str);
					} break;
					default: error_exit("[ADD] Invalid operands"); break;
				}

				push(r);
			} break;

			case SUB: {
				struct vm_ht_item b = pop();
				struct vm_ht_item a = pop();

				struct vm_ht_item *r = new_vm_ht_item(highest_precision(a, b));
				switch(r->type) {
					case CHAR: r->content.c = to_char(a) - to_char(b); break;
					case INT: r->content.i = to_int(a) - to_int(b); break;
					case DOUBLE: r->content.d = to_double(a) - to_double(b); break;
					default: error_exit("[SUB] Invalid operands"); break;
				}

				push(r);
			} break;

			case MUL: {
				struct vm_ht_item b = pop();
				struct vm_ht_item a = pop();

				struct vm_ht_item *r = new_vm_ht_item(highest_precision(a, b));
				switch(r->type) {
					case CHAR: r->content.c = to_char(a) * to_char(b); break;
					case INT: r->content.i = to_int(a) * to_int(b); break;
					case DOUBLE: r->content.d = to_double(a) * to_double(b); break;
					default: error_exit("[MOV] Invalid operands"); break;
				}

				push(r);
			} break;

			case DIV: {
				struct vm_ht_item b = pop();
				struct vm_ht_item a = pop();
	
				struct vm_ht_item *r = new_vm_ht_item(highest_precision(a, b));
				switch(r->type) {
					case CHAR: r->content.c = to_char(a) / to_char(b); break;
					case INT: r->content.i = to_int(a) / to_int(b); break;
					case DOUBLE: r->content.d = to_double(a) / to_double(b); break;
					default: error_exit("[DIV] Invalid operands"); break;
				}

				push(r);
			} break;

			case MOD: {
				struct vm_ht_item b = pop();
				struct vm_ht_item a = pop();

				switch(a.type) {
					case INT: a.content.i %= b.content.i; break;
					default: error_exit("Invalid operands"); break;
				}

				push(&a);
			} break;

			case JMP: {
				short address = get_short(fp);
				jump(fp, address);
			} break;

			case JMPC: {
				struct vm_ht_item item = pop();

				short address = get_short(fp);
				if (to_int(item) == 0)
					jump(fp, address);
			} break;

			case CEQ: {
				struct vm_ht_item b = pop();
				struct vm_ht_item a = pop();
				u8 type = highest_precision(a, b);

				struct vm_ht_item *item = new_vm_ht_item(INT);
				switch(type) {
					case CHAR: item->content.i = to_char(a) == to_char(b); break;
					case INT: item->content.i = to_int(a) == to_int(b); break;
					case DOUBLE: item->content.i = to_double(a) == to_double(b); break;
					case STRING:
						item->content.i = strcmp(to_string(a)->str, to_string(b)->str) == 0;
					break;
				}

				push(item);
			} break;

			case CNE: {
				struct vm_ht_item b = pop();
				struct vm_ht_item a = pop();
				u8 type = highest_precision(a, b);

				struct vm_ht_item *item = new_vm_ht_item(INT);
				switch(type) {
					case CHAR: item->content.i = to_char(a) != to_char(b); break;
					case INT: item->content.i = to_int(a) != to_int(b); break;
					case DOUBLE: item->content.i = to_double(a) != to_double(b); break;
					case STRING:
						item->content.i = strcmp(to_string(a)->str, to_string(b)->str) != 0;
					break;
				}

				push(item);
			} break;

			case CGT: {
				struct vm_ht_item b = pop();
				struct vm_ht_item a = pop();
				u8 type = highest_precision(a, b);

				struct vm_ht_item *item = new_vm_ht_item(INT);
				switch(type) {
					case CHAR: item->content.i = to_char(a) > to_char(b); break;
					case INT: item->content.i = to_int(a) > to_int(b); break;
					case DOUBLE: item->content.i = to_double(a) > to_double(b); break;
					case STRING:
						item->content.i = strcmp(to_string(a)->str, to_string(b)->str) > 0;
					break;
				}

				push(item);
			} break;

			case CGE: {
				struct vm_ht_item b = pop();
				struct vm_ht_item a = pop();
				u8 type = highest_precision(a, b);

				struct vm_ht_item *item = new_vm_ht_item(INT);
				switch(type) {
					case CHAR: item->content.i = to_char(a) >= to_char(b); break;
					case INT: item->content.i = to_int(a) >= to_int(b); break;
					case DOUBLE: item->content.i = to_double(a) >= to_double(b); break;
					case STRING:
						item->content.i = strcmp(to_string(a)->str, to_string(b)->str) >= 0;
					break;
				}

				push(item);
			} break;

			case CLT: {
				struct vm_ht_item b = pop();
				struct vm_ht_item a = pop();
				u8 type = highest_precision(a, b);

				struct vm_ht_item *item = new_vm_ht_item(INT);
				switch(type) {
					case CHAR: item->content.i = to_char(a) < to_char(b); break;
					case INT: item->content.i = to_int(a) < to_int(b); break;
					case DOUBLE: item->content.i = to_double(a) < to_double(b); break;
					case STRING:
						item->content.i = strcmp(to_string(a)->str, to_string(b)->str) < 0;
					break;
				}

				push(item);
			} break;

			case CLE: {
				struct vm_ht_item b = pop();
				struct vm_ht_item a = pop();
				u8 type = highest_precision(a, b);

				struct vm_ht_item *item = new_vm_ht_item(INT);
				switch(type) {
					case CHAR: item->content.i = to_char(a) <= to_char(b); break;
					case INT: item->content.i = to_int(a) <= to_int(b); break;
					case DOUBLE: item->content.i = to_double(a) <= to_double(b); break;
					case STRING:
						item->content.i = strcmp(to_string(a)->str, to_string(b)->str) <= 0;
					break;
				}

				push(item);
			} break;

			case RDI: {
				char buffer[64];
				fgets(buffer, 64, stdin);

				struct vm_ht_item *item = new_vm_ht_item(INT);
				item->content.i = atoi(buffer);

				push(item);
			} break;

			case RDD: {
				char buffer[64];
				fgets(buffer, 64, stdin);

				struct vm_ht_item *item = new_vm_ht_item(DOUBLE);
				item->content.d = atof(buffer);

				push(item);
			} break;

			case RDC: {
				char c = getchar();

				struct vm_ht_item *item = new_vm_ht_item(CHAR);
				item->content.c = c;

				push(item);
			} break;
	
			case RDS: {
				char *buffer = malloc(sizeof(char) * MAX_STRING_SIZE);
				fgets(buffer, MAX_STRING_SIZE, stdin);

				struct vm_ht_item *item = new_vm_ht_item(STRING);
				item->content.s = malloc(sizeof(struct string));
				item->content.s->size = strlen(buffer);
				item->content.s->str = buffer;

				push(item);
			} break;

			case WRT: {
				struct vm_ht_item item = pop();
				write(item);
			} break;

			case WRTS: {
				struct string *s = get_string(fp);
				printf("%s", s->str);
			} break;

			case WRTLN: printf("\n"); break;

			case FREE: vm_ht_del(ht, get_u16(fp)); break;
		}
	}
}

int main() {
	FILE *fp = fopen("out.eva", "rb+");
	setvbuf(fp, NULL, _IONBF, 0);

	if (!fp) {
		printf("Input file not found\n");
		exit(1);
	}

	fseek(fp, HEADER_OFFSET, SEEK_SET);
	ht = new_vm_ht(64);
	run(fp);
}
