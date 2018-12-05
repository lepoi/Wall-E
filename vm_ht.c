#include <stdio.h>
#include <stdlib.h>
#include "vm_ht.h"

struct vm_ht *new_vm_ht(u8 bucket_size) {
	struct vm_ht *ht = malloc(sizeof(struct vm_ht));
	ht->size = bucket_size;
	ht->items = calloc((size_t) ht->size, sizeof(struct vm_ht_item *));

	return ht;
}

void vm_ht_add(struct vm_ht *ht, struct vm_ht_item *item) {
	int index = item->id % ht->size;
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

void vm_ht_del(struct vm_ht *ht, u16 id) {
	int index = id % ht->size;
	struct vm_ht_item *current = ht->items[index];
	if (current && current->id == id) {
		ht->items[index] = current->next;
		return;
	}

	while (current->next) {
		if (((struct vm_ht_item *) (current->next))->id == id) {
			current->next = ((struct vm_ht_item *) (current->next))->next;
			break;
		}

		current = current->next;
	}
}
