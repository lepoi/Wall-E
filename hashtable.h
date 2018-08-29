struct ht_item {
	char *label;
	char opcode;
	char (*body)(FILE *, struct asm_state *);
	void *next;
};

struct hashtable {
	unsigned int size;
	char count;
	struct ht_item **items;
};

struct hashtable *new_ht(unsigned int s);
void rm_ht(struct hashtable *ht);
static void rm_ht_helper(struct ht_item *item);

struct ht_item *new_ht_item(char opcode, const char *label, void *body);
unsigned short rm_ht_item(struct hashtable *ht, char *str);

unsigned long hash(unsigned char *str);
void hash_item(struct hashtable *ht, struct ht_item *item);
struct ht_item *lookup_item(struct hashtable *ht, char *label);
