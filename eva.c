#include <stdio.h>
#include <stdlib.h>
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

#define MAX_STACK_SIZE	64

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
			case DCLI: {
				struct vm_ht_item *var = malloc(sizeof(struct vm_ht_item));
				var->type = TYPE_INT;
				var->size = 4;

				vm_ht_add(ht, var);
			} break;

			case DCLD: {
			} break;

			case DCLC: {
			} break;

			case DCLS: {	
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
/*
			case MOVK: {
				unsigned int id;
				fread(&id, sizeof(int), 1, fp);
			
				struct var *v = find_var(id);
				switch (v->type) {
					case 0: {
						int i;
						fread(&i, sizeof(int), 1, fp);
						v->content.i = i;
					} break;

					case 1: {
						double d;
						fread(&d, sizeof(double), 1, fp);
						v->content.d = d;
					} break;

					case 2: {
						char c;
						fread(&c, sizeof(char), 1, fp);
						v->content.c = c;
					} break;

					case 3: {
						int len = 0;

						for (char c = fgetc(fp); c != '\x00'; c = fgetc(fp), len++);
						char *s = malloc(sizeof(char) * len);
						fseek(fp, -len, SEEK_CUR);
					} break;
				}
			} break;
*/
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
