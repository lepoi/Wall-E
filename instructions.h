void decli() {

}


struct instruction {
	char *name;
	void *args;
};

static const struct instruction instructions[] = {
	ASSIGN_BODY("decli", decli);
};
