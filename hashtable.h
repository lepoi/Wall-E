struct state_s;

typedef struct {
	char *label;
	char opcode;
	void (*body)(FILE *, struct state_s *);
	void *next;
} ht_item;

typedef struct {
	unsigned int size;
	char count;
	ht_item **items;
} hashtable;

hashtable *new_ht(unsigned int s);
void rm_ht(hashtable *ht);

ht_item *new_ht_item(char opcode, const char *label, void *body);
void rm_ht_item(ht_item *item);

unsigned long hash(unsigned char *str);
void hash_item(hashtable *ht, ht_item *item);
ht_item *lookup_item(hashtable *ht, char *label);
