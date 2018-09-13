#include <stdio.h>
#include <stdlib.h>

#define MAX_STACK_SIZE 128

#define TYPE_INT 1
#define TYPE_DOUBLE 2
#define TYPE_CHAR 3

struct stack_item {
	char type;
	union {
       int i;
       double d;
       char c;
       char *s;
   } content;
} stack[MAX_STACK_SIZE];

typedef unsigned short addr_t;
    
void run(FILE *fp) {
	char c;
	int size = 0;
 	fseek(fp, 20, SEEK_SET);
	do {
		c = fgetc(fp);
		switch (c) {
			case '\x06': {
				struct stack_item i = {};
				i.type = TYPE_INT;
				fread(&i.content.i, sizeof(int), 1, fp);
				stack[size++] = i; 
			} break;
			case '\x07': {
				struct stack_item i = {};
				i.type = TYPE_DOUBLE;
				fread(&i.content.d, sizeof(float), 1, fp);
				stack[size++] = i;
			}break;
			case '\x08': {
				struct stack_item i = {};
				i.type = TYPE_CHAR;
				fread(&i.content.c, sizeof(char), 1, fp);
				printf("char: %c\n", i.content.c);
				stack[size++] = i;
			}break;
			
			case '\x2b': {
				addr_t addr;
				fread(&addr, 2, 1, fp);
				fseek(fp, addr, SEEK_SET);
			} break;

			case EOF: break;
			
			default:
				printf("Unrecognized instruction: %x\n", c);
			break;
			/*
			case '\x09':{
				struct stack_item i = {};
				i.type = 4; // push k string    aaaaaaa
				printf("r: %i\n", fscanf(fp, "%c", &i.content.i));
				stack[size++] = i;
			}break;
			case '\x10':{
				struct stack_item i = {};
				i.type = 5; //  push k vector integer
				printf("r: %i\n", fscanf(fp, "%c", &i.content.i));
				stack[size++] = i;
			}break;
			*/
		}
	} while (c != EOF);

	printf("Stack contents:\n");
	for (int i = 0; i < size; i++) {
		printf("[%i] [%p] ", i, &stack[i]);
		switch(stack[i].type) {
			case TYPE_INT:
				printf("%i", stack[i].content.i);
			break;

			case TYPE_DOUBLE:
				printf("%lf", stack[i].content.d);
			break;

			case TYPE_CHAR:
				printf("%c", stack[i].content.c);
			break;
		}
		printf("\n");
	}
}

int main (int argc, char **argv){
    FILE *fp = fopen("out.eva", "rb+");

    run(fp);
	return 0;
}

