#include <sys/types.h>
#include <sys/queue.h>
#include <stdbool.h>

enum {
	UDEV_FILTER_TYPE_SUBSYSTEM,
	UDEV_FILTER_TYPE_SYSNAME,
	UDEV_FILTER_TYPE_PROPERTY,
	UDEV_FILTER_TYPE_INITIALIZED,
	UDEV_FILTER_TYPE_TAG,
	UDEV_FILTER_TYPE_SYSATTR,
};
STAILQ_HEAD(udev_filter_head, udev_filter_entry);

void udev_filter_init(struct udev_filter_head *ufh);
bool udev_filter_match_subsystem(struct udev_filter_head *ufh,
    const char *subsystem);
bool udev_filter_match(struct udev *udev, struct udev_filter_head *ufh,
    const char *syspath);
int udev_filter_add(struct udev_filter_head *ufh, int type, int neg,
    const char *expr, const char *value);
void udev_filter_free(struct udev_filter_head *ufh);
