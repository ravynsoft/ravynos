#ifndef UDEV_LIST_H_
#define UDEV_LIST_H_

#include "config.h"
#include "utils.h"

RB_HEAD(udev_list, udev_list_entry);

void udev_list_init(struct udev_list *ul);
int udev_list_insert(struct udev_list *ul, char const *name,
    char const *value);
void udev_list_free(struct udev_list *ul);
struct udev_list_entry *udev_list_entry_get_first(struct udev_list *ul);
const char *_udev_list_entry_get_name(struct udev_list_entry *ule);
const char *_udev_list_entry_get_value(struct udev_list_entry *ule);

#endif /* UDEV_LIST_H_ */
