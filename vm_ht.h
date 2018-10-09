#include "types.h"

struct vm_ht_item {
	u16 id;
	u8 type;
	int size;
	union {
		int i;
		double d;
		char c;
		struct string *s;
		int *vi;
		double *vd;
		char *vc;
		struct string **vs;
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
