#ifndef UDEV_H_
#define UDEV_H_

#include "libudev.h"

struct udev *_udev_ref(struct udev *udev);
void _udev_unref(struct udev *udev);

#endif /* UDEV_H_ */
