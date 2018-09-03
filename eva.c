#include <stdio.h>

#define MAX_STACK_SIZE 128

struct stack_item {
   char type;
   union {
       int i;
       double d;
       char c;
       char *s;
   } content;
} stack[MAX_STACK_SIZE];

void run(FILE *fp) {
	char c;
	int size = 0;
	fseek(fp, 20, SEEK_SET);
	do {
		c = fgetc(fp);
		printf("c = %x\n", c);
		switch (c) {
			case '\x06': {
				struct stack_item i = {};
				i.type = 1;
				printf("r: %i\n", fscanf(fp, "%c", &i.content.i));
				stack[size++] = i;
			} break;
		}

	} while (c != EOF);
		printf("Stack contents:\n");
		for (int i = 0; i < size; i++) {
			printf("%i\n", stack[i].content.i);
		}
}

int main (int argc, char **argv){
    FILE *fp = fopen("out.bin", "rb+");

    run(fp);
	return 0;
}

