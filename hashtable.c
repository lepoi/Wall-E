#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "hashtable.h"

hashtable *new_ht(unsigned int s) {
	hashtable *ht = malloc(sizeof(hashtable));
	ht->size = s;
	ht->count = 0;
	ht->items = calloc((size_t)ht->size, sizeof(ht_item*));

	return ht;
}

void rm_ht(hashtable *ht) {
	for (int i = 0; i < ht->size; i++) {
		ht_item *item = ht->items[i];
		
		if (item)
			rm_ht_item(item);
	}
	free(ht->items);
	free(ht);
}

ht_item *new_ht_item(char opcode, const char *label, void *body) {
	ht_item *item = malloc(sizeof(ht_item));
	item->label = strdup(label);
	item->opcode = opcode;
	item->body = body;
	item->next = NULL;
	
	return item;
}

void rm_ht_item(ht_item *item) {
	free(item->label);
	
	if (item->next)
		rm_ht_item(item->next);
}

unsigned long hash(unsigned char *str) {
	unsigned long hash = 5381;
    int c;

    while (c = *str++)
        hash = ((hash << 5) + hash) + c;

    return hash;
}

void hash_item(hashtable *ht, ht_item *item) {
	unsigned int index = hash(item->label) % ht->size;
	ht_item *stop = ht->items[index];

	if (!stop)
		ht->items[index] = item;
	else {
		while (stop->next)
			stop = stop->next;

		stop->next = item;
	}
	ht->count++;
}

ht_item *lookup_item(hashtable *ht, char *label) {
	unsigned int index = hash(label) % ht->size;
	ht_item *target = ht->items[index];

	while (target) {
		if (strcmp(target->label, label) == 0)
			break;
		else 
			target = target->next;
	}
	return target;
}
