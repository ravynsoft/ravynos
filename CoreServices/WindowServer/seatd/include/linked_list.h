#ifndef _LINKED_LIST_H
#define _LINKED_LIST_H

#include <stdbool.h>

struct linked_list {
	struct linked_list *prev;
	struct linked_list *next;
};

void linked_list_init(struct linked_list *list);
void linked_list_insert(struct linked_list *list, struct linked_list *elem);
void linked_list_remove(struct linked_list *elem);
bool linked_list_empty(struct linked_list *list);
void linked_list_take(struct linked_list *target, struct linked_list *source);

#endif
