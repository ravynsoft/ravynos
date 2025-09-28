#ifndef UDEV_UTILS_H_
#define UDEV_UTILS_H_

#include "libudev.h"

#define	LIBUDEV_EXPORT	__attribute__((visibility("default")))

#define	DEV_PATH_ROOT	"/dev"
#define	DEV_PATH_MAX	80
#define	SYS_PATH_MAX	80

#define	UNKNOWN_SUBSYSTEM	"#"

const char *get_subsystem_by_syspath(const char *syspath);
const char *get_sysname_by_syspath(const char *syspath);
const char *get_devpath_by_syspath(const char *syspath);
const char *get_syspath_by_devpath(const char *devpath);
const char *get_syspath_by_devnum(dev_t devnum);

void invoke_create_handler(struct udev_device *ud);
size_t syspathlen_wo_units(const char *path);

#endif /* UDEV_UTILS_H_ */
