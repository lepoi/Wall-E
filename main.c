#include <stdio.h>

int main(int n, char **args) {
	FILE *source;
	char *line = NULL, *result;
	size_t len = 0;
	ssize_t read;

	if (! args[1]) {
		printf("Needs more arguments!\n");
		return 1;
	}

	source = fopen(args[1], "r");
	
	if (!source) {
		printf("Error opening %s\n", args[1]);
		return 1;
	}

	result = "";

	printf("Printing input file:\n");
	printf("-----\n");
	while (read = getline(&line, &len, source) != -1) {
        printf("Retrieved: %s", line);
	}

	return 0;
}