#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "hashtable.h"

struct hashtable *new_ht(unsigned int s) {
	struct hashtable *ht = malloc(sizeof(struct hashtable));
	ht->size = s;
	ht->count = 0;
	ht->items = calloc((size_t) ht->size, sizeof(struct ht_item *));

	return ht;
}

void rm_ht(struct hashtable *ht) {
	for (int i = 0; i < ht->size; i++) {
		struct ht_item *item = ht->items[i];
		
		if (item)
			rm_ht_helper(item);
	}

	free(ht->items);
	free(ht);
}

void rm_ht_helper(struct ht_item *item) {
	free(item->label);
	
	if (item->next)
		rm_ht_helper(item->next);
}

struct ht_item *new_ht_item(char opcode, const char *label, void *body) {
	struct ht_item *item = malloc(sizeof(struct ht_item));
	item->label = strdup(label);
	item->opcode = opcode;
	item->body = body;
	item->next = NULL;
	
	return item;
}

unsigned short rm_ht_item(struct hashtable *ht, char *str) {
	struct ht_item *item = ht->items[hash(str) % ht->size];

	if (item->label == str) {
		ht->items[hash(str) % ht->size] = item->next;
		free(item->label);

		return 0;
	}
		
	struct ht_item *helper = item->next;

	while (item->next) {
		if (item->label == str) {
			helper->next = item->next;
			free(item->label);

			return 0;
		}

		item = item->next;
		helper = helper->next;
	}

	return 1;
}

unsigned long hash(unsigned char *str) {
	unsigned long hash = 5381;
    	int c;

	while (c = *str++)
		hash = ((hash << 5) + hash) + c;

	return hash;
}

void hash_item(struct hashtable *ht, struct ht_item *item) {
	unsigned int index = hash(item->label) % ht->size;
	struct ht_item *stop = ht->items[index];

	if (!stop)
		ht->items[index] = item;
	else {
		while (stop->next)
			stop = stop->next;

		stop->next = item;
	}

	ht->count++;
}

struct ht_item *lookup_item(struct hashtable *ht, char *label) {
	unsigned int index = hash(label) % ht->size;
	struct ht_item *target = ht->items[index];

	while (target) {
		if (strcmp(target->label, label) == 0)
			break;
		else 
			target = target->next;
	}

	return target;
}
