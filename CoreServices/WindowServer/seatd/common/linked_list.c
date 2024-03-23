#include <assert.h>
#include <stdbool.h>
#include <stddef.h>

#include "linked_list.h"

void linked_list_init(struct linked_list *list) {
	list->next = list;
	list->prev = list;
}

void linked_list_insert(struct linked_list *list, struct linked_list *elem) {
	assert(list->prev != NULL && list->next != NULL);
	assert(elem->prev == NULL && elem->next == NULL);

	elem->prev = list;
	elem->next = list->next;
	list->next = elem;
	elem->next->prev = elem;
}

void linked_list_remove(struct linked_list *elem) {
	assert(elem->prev != NULL && elem->next != NULL);

	elem->prev->next = elem->next;
	elem->next->prev = elem->prev;
	elem->next = NULL;
	elem->prev = NULL;
}

bool linked_list_empty(struct linked_list *list) {
	assert(list->prev != NULL && list->next != NULL);
	return list->next == list;
}

void linked_list_take(struct linked_list *target, struct linked_list *source) {
	if (linked_list_empty(source)) {
		return;
	}

	source->next->prev = target;
	source->prev->next = target->next;
	target->next->prev = source->prev;
	target->next = source->next;
	linked_list_init(source);
}
