typedef struct {
	char *label;
	char cost;
	char ope;
	void *next;
} ht_item;

typedef struct {
	unsigned int size;
	char count;
	ht_item **items;
} hashtable;

static hashtable *new_ht(unsigned int s);
static void rm_ht(hashtable *ht);

ht_item *new_ht_item(const char *label, const char val);
void rm_ht_item(ht_item *item);

unsigned long hash(unsigned char *str);
void hash_item(hashtable *ht, ht_item *item);
ht_item *lookup_item(hashtable *ht, char *label);