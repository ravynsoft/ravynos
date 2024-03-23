#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "linked_list.h"
#include "test.h"

struct list_elem {
	struct linked_list link;
	char *content;
};

static void test_linked_list_init(void) {
	struct linked_list list;
	linked_list_init(&list);

	// Both next and prev should point to self
	test_assert(list.next == &list && list.prev == &list);

	// The list should be empty
	test_assert(linked_list_empty(&list));
}

static void test_linked_list_single_insert(void) {
	struct linked_list list;
	linked_list_init(&list);

	struct list_elem elem1 = {{0}, NULL};
	linked_list_insert(&list, &elem1.link);

	// Both next and prev on list should point to the elem
	test_assert(list.next == &elem1.link && list.prev == &elem1.link);

	// Both next and prev on elem should point to the list
	test_assert(elem1.link.next == &list && elem1.link.prev == &list);

	// The list and element should not be empty
	test_assert(!linked_list_empty(&list));
	test_assert(!linked_list_empty(&elem1.link));
}

static void test_linked_list_single_remove(void) {
	struct linked_list list;
	linked_list_init(&list);

	struct list_elem elem1 = {{0}, NULL};
	linked_list_insert(&list, &elem1.link);
	linked_list_remove(&elem1.link);

	// Both next and prev on elem be NULL
	test_assert(elem1.link.next == NULL && elem1.link.prev == NULL);

	// Both next and prev should point to self
	test_assert(list.next == &list && list.prev == &list);

	// The list should be empty
	test_assert(linked_list_empty(&list));
}

static void test_linked_list_alternate_remove(void) {
	struct linked_list list;
	linked_list_init(&list);

	struct list_elem elem1 = {{0}, NULL};
	linked_list_insert(&list, &elem1.link);
	linked_list_remove(&list);

	// Both next and prev on list be NULL
	test_assert(list.next == NULL && list.prev == NULL);

	// Both next and prev should point to self
	test_assert(elem1.link.next == &elem1.link && elem1.link.prev == &elem1.link);

	// The elem should be empty
	test_assert(linked_list_empty(&elem1.link));
}

static void test_linked_list_sequential_remove(void) {
	struct linked_list list;
	linked_list_init(&list);

	struct list_elem elem1 = {{0}, NULL}, elem2 = {{0}, NULL}, elem3 = {{0}, NULL};
	linked_list_insert(&list, &elem1.link);
	linked_list_insert(&elem1.link, &elem2.link);
	linked_list_insert(&elem2.link, &elem3.link);

	// The order should now be list→elem1→elem2→elem3→list
	test_assert(list.next == &elem1.link && list.prev == &elem3.link);
	test_assert(elem1.link.next == &elem2.link && elem1.link.prev == &list);
	test_assert(elem2.link.next == &elem3.link && elem2.link.prev == &elem1.link);
	test_assert(elem3.link.next == &list && elem3.link.prev == &elem2.link);

	linked_list_remove(list.next);

	// The order should now be list→elem2→elem3→list
	test_assert(list.next == &elem2.link && list.prev == &elem3.link);
	test_assert(elem2.link.next == &elem3.link && elem2.link.prev == &list);
	test_assert(elem3.link.next == &list && elem3.link.prev == &elem2.link);
	test_assert(elem1.link.next == NULL && elem1.link.prev == NULL);

	linked_list_remove(list.next);

	// The order should now be list→elem3→list
	test_assert(list.next == &elem3.link && list.prev == &elem3.link);
	test_assert(elem3.link.next == &list && elem3.link.prev == &list);
	test_assert(elem1.link.next == NULL && elem1.link.prev == NULL);
	test_assert(elem2.link.next == NULL && elem2.link.prev == NULL);

	linked_list_remove(list.next);

	// The list should now be empty
	test_assert(elem1.link.next == NULL && elem1.link.prev == NULL);
	test_assert(elem2.link.next == NULL && elem2.link.prev == NULL);
	test_assert(elem3.link.next == NULL && elem3.link.prev == NULL);
	test_assert(list.next == &list && list.prev == &list);
	test_assert(linked_list_empty(&list));
}

static void test_linked_list_insert_after(void) {
	struct linked_list list;
	linked_list_init(&list);

	struct list_elem elem1 = {{0}, NULL}, elem2 = {{0}, NULL}, elem3 = {{0}, NULL};
	linked_list_insert(&list, &elem1.link);
	linked_list_insert(&elem1.link, &elem3.link);
	linked_list_insert(&elem1.link, &elem2.link);

	// The order should now be list→elem1→elem2→elem3→list
	test_assert(list.next == &elem1.link && list.prev == &elem3.link);
	test_assert(elem1.link.next == &elem2.link && elem1.link.prev == &list);
	test_assert(elem2.link.next == &elem3.link && elem2.link.prev == &elem1.link);
	test_assert(elem3.link.next == &list && elem3.link.prev == &elem2.link);
}

static void test_linked_list_remove_loop(void) {
	struct linked_list list;
	linked_list_init(&list);

	struct list_elem elem1 = {{0}, NULL}, elem2 = {{0}, NULL}, elem3 = {{0}, NULL};
	linked_list_insert(&list, &elem1.link);
	linked_list_insert(&elem1.link, &elem2.link);
	linked_list_insert(&elem2.link, &elem3.link);

	size_t cnt = 0;
	while (!linked_list_empty(&list)) {
		struct list_elem *elem = (struct list_elem *)list.next;
		linked_list_remove(&elem->link);
		cnt++;
	}
	test_assert(cnt == 3);

	// Link should now be empty, and next and prev on all elements hsould be NULL
	test_assert(linked_list_empty(&list));
	test_assert(elem1.link.next == NULL && elem1.link.prev == NULL);
	test_assert(elem2.link.next == NULL && elem2.link.prev == NULL);
	test_assert(elem3.link.next == NULL && elem3.link.prev == NULL);
}

static void test_linked_list_manual_iterate(void) {
	struct linked_list list;
	linked_list_init(&list);

	struct list_elem elem1 = {{0}, "elem1"};
	struct list_elem elem2 = {{0}, "elem2"};
	struct list_elem elem3 = {{0}, "elem3"};
	linked_list_insert(&list, &elem1.link);
	linked_list_insert(&elem1.link, &elem2.link);
	linked_list_insert(&elem2.link, &elem3.link);

	struct list_elem *ptr = NULL;

	ptr = (struct list_elem *)list.next;
	test_assert(strcmp("elem1", ptr->content) == 0);

	ptr = (struct list_elem *)ptr->link.next;
	test_assert(strcmp("elem2", ptr->content) == 0);

	ptr = (struct list_elem *)ptr->link.next;
	test_assert(strcmp("elem3", ptr->content) == 0);

	test_assert(ptr->link.next == &list);
}

static void test_linked_list_loop_iterate(void) {
	struct linked_list list;
	linked_list_init(&list);

	struct list_elem elem1 = {{0}, "elem"};
	struct list_elem elem2 = {{0}, "elem"};
	struct list_elem elem3 = {{0}, "elem"};
	linked_list_insert(&list, &elem1.link);
	linked_list_insert(&elem1.link, &elem2.link);
	linked_list_insert(&elem1.link, &elem3.link);

	size_t cnt = 0;
	for (struct linked_list *ptr = list.next; ptr != &list; ptr = ptr->next) {
		struct list_elem *elem = (struct list_elem *)ptr;
		test_assert(strcmp("elem", elem->content) == 0);
		cnt++;
	}
	test_assert(cnt == 3);
}

static void test_linked_list_take_empty(void) {
	struct linked_list list1, list2;
	linked_list_init(&list1);
	linked_list_init(&list2);

	linked_list_take(&list2, &list1);

	test_assert(linked_list_empty(&list1));
	test_assert(linked_list_empty(&list2));
}

static void test_linked_list_take_single(void) {
	struct linked_list list1, list2;
	linked_list_init(&list1);
	linked_list_init(&list2);

	struct list_elem elem1 = {{0}, NULL};
	linked_list_insert(&list1, &elem1.link);

	linked_list_take(&list2, &list1);

	test_assert(linked_list_empty(&list1));
	test_assert(list2.next == &elem1.link && list2.prev == &elem1.link);
	test_assert(elem1.link.next == &list2 && elem1.link.prev == &list2);
}

static void test_linked_list_take_many(void) {
	struct linked_list list1, list2;
	linked_list_init(&list1);
	linked_list_init(&list2);

	struct list_elem elem1 = {{0}, NULL};
	struct list_elem elem2 = {{0}, NULL};
	linked_list_insert(&list1, &elem2.link);
	linked_list_insert(&list1, &elem1.link);

	linked_list_take(&list2, &list1);

	test_assert(linked_list_empty(&list1));
	test_assert(list2.next == &elem1.link && list2.prev == &elem2.link);
	test_assert(elem1.link.next == &elem2.link && elem1.link.prev == &list2);
	test_assert(elem2.link.next == &list2 && elem2.link.prev == &elem1.link);
}

static void test_linked_list_take_concat(void) {
	struct linked_list list1, list2;
	linked_list_init(&list1);
	linked_list_init(&list2);

	struct list_elem elem1 = {{0}, NULL};
	struct list_elem elem2 = {{0}, NULL};
	struct list_elem elem3 = {{0}, NULL};
	struct list_elem elem4 = {{0}, NULL};
	linked_list_insert(&list1, &elem2.link);
	linked_list_insert(&list1, &elem1.link);
	linked_list_insert(&list2, &elem4.link);
	linked_list_insert(&list2, &elem3.link);

	linked_list_take(&list2, &list1);

	test_assert(linked_list_empty(&list1));
	test_assert(list2.next == &elem1.link && list2.prev == &elem4.link);
	test_assert(elem1.link.next == &elem2.link && elem1.link.prev == &list2);
	test_assert(elem2.link.next == &elem3.link && elem2.link.prev == &elem1.link);
	test_assert(elem3.link.next == &elem4.link && elem3.link.prev == &elem2.link);
	test_assert(elem4.link.next == &list2 && elem4.link.prev == &elem3.link);
}

int main(int argc, char *argv[]) {
	(void)argc;
	(void)argv;

	test_run(test_linked_list_init);
	test_run(test_linked_list_single_insert);
	test_run(test_linked_list_single_remove);
	test_run(test_linked_list_alternate_remove);
	test_run(test_linked_list_sequential_remove);
	test_run(test_linked_list_insert_after);
	test_run(test_linked_list_remove_loop);
	test_run(test_linked_list_manual_iterate);
	test_run(test_linked_list_loop_iterate);
	test_run(test_linked_list_take_empty);
	test_run(test_linked_list_take_single);
	test_run(test_linked_list_take_many);
	test_run(test_linked_list_take_concat);

	return 0;
}
