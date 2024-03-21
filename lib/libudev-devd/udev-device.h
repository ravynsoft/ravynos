#ifndef UDEV_DEVICE_H_
#define UDEV_DEVICE_H_

#include "libudev.h"
#include "udev-list.h"

/* udev_device flags */
enum {
	UD_ACTION_NONE,
	UD_ACTION_ADD,
	UD_ACTION_REMOVE,
	UD_ACTION_HOTPLUG,
};

struct udev_device *udev_device_new_common(struct udev *udev,
    const char *syspath, int action);
struct udev_list *udev_device_get_properties_list(struct udev_device *ud);
struct udev_list *udev_device_get_sysattr_list(struct udev_device *ud);
struct udev_list *udev_device_get_tags_list(struct udev_device *ud);
struct udev_list *udev_device_get_devlinks_list(struct udev_device *ud);
void udev_device_set_parent(struct udev_device *ud, struct udev_device *parent);

#endif /* UDEV_DVICE_H_ */
