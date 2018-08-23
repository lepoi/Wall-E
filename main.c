#include <stdio.h>
#include <string.h>

void decli(FILE *fp) {
	char buffer[64];
	fscanf(fp, "%s", buffer);
	printf("%s\n", buffer);
}

/*
 * TODO: Change this for a hash table.
 */
void *search(char *buf) {
	if (!strcmp(buf, "decli")) {
		return decli;		
	}

	return NULL;
}

void lex(FILE *fp) {
	char buffer[64];

	fscanf(fp, "%s", buffer);
	printf("%s\n", buffer);

	void (*body)(FILE *) = search(buffer);
	if (!body) {
		printf("Invalid instruction\n");	
		return;
	}

	body(fp);
}

int main(int argc, char *args[]) {
	FILE *fp;

	if (!args[1]) {
		printf("Needs more arguments\n");
		return 1;
	}

	fp = fopen(args[1], "r");
	if (!fp) {
		printf("Invalid file");
		return 1;
	}

	lex(fp);

	return 0;
}
