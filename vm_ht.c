#include <stdio.h>
#include <stdlib.h>
#include "vm_ht.h"

struct vm_ht *new_vm_ht(u8 bucket_size) {
	struct vm_ht *ht = malloc(sizeof(struct vm_ht));
	ht->size = bucket_size;
	ht->count = 0;
	ht->items = calloc((size_t) ht->size, sizeof(struct vm_ht_item *));

	return ht;
}

void vm_ht_add(struct vm_ht *ht, struct vm_ht_item *item) {
	int index = ht->count++ % ht->size;
	struct vm_ht_item *stop = ht->items[index];

	if (!stop)
		ht->items[index] = item;
	else {
		while (stop->next)
			stop = stop->next;

		stop->next = item;
	}
}

struct vm_ht_item *vm_ht_get(struct vm_ht *ht, unsigned short id) {
	int index = id % ht->size;
	struct vm_ht_item *current = ht->items[index];

	while (current) {
		if (id == current->id)
			break;
		else
			current = current->next;
	}

	return current;
}
