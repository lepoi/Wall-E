typedef unsigned char u8;
typedef unsigned int u32;

struct vm_ht_item {
	unsigned short id;
	u8 type;
	u8 size;
	union {
		int i;
		double d;
		char c;
		char *s;
	} content;
	void *next;
};

struct vm_ht {
	u32 count;
	u8 size;
	struct vm_ht_item **items;
};

struct vm_ht *new_vm_ht(u8 bucket_size);
void vm_ht_add(struct vm_ht *ht, struct vm_ht_item *item);
struct vm_ht_item *vm_ht_get(struct vm_ht *ht, unsigned short id);
