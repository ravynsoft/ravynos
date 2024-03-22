/*
 * Copyright © 2018 Broadcom
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

/**
 * @file
 *
 * Implements wrappers of libc functions to fake having a DRM device that
 * isn't actually present in the kernel.
 */

/* Prevent glibc from defining open64 when we want to alias it. */
#undef _FILE_OFFSET_BITS
#undef _TIME_BITS
#define _LARGEFILE64_SOURCE

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>
#include <stdarg.h>
#include <fcntl.h>
#include <dlfcn.h>
#include <dirent.h>
#include <c11/threads.h>
#include <drm-uapi/drm.h>

#include "util/anon_file.h"
#include "util/set.h"
#include "util/simple_mtx.h"
#include "util/u_debug.h"
#include "drm_shim.h"

#define REAL_FUNCTION_POINTER(x) __typeof__(x) *real_##x

static simple_mtx_t shim_lock = SIMPLE_MTX_INITIALIZER;
struct set *opendir_set;
bool drm_shim_debug;

/* If /dev/dri doesn't exist, we'll need an arbitrary pointer that wouldn't be
 * returned by any other opendir() call so we can return just our fake node.
 */
DIR *fake_dev_dri = (void *)&opendir_set;

REAL_FUNCTION_POINTER(close);
REAL_FUNCTION_POINTER(closedir);
REAL_FUNCTION_POINTER(dup);
REAL_FUNCTION_POINTER(fcntl);
REAL_FUNCTION_POINTER(fopen);
REAL_FUNCTION_POINTER(ioctl);
REAL_FUNCTION_POINTER(mmap);
REAL_FUNCTION_POINTER(mmap64);
REAL_FUNCTION_POINTER(open);
REAL_FUNCTION_POINTER(opendir);
REAL_FUNCTION_POINTER(readdir);
REAL_FUNCTION_POINTER(readdir64);
REAL_FUNCTION_POINTER(readlink);
REAL_FUNCTION_POINTER(realpath);

#define HAS_XSTAT __GLIBC__ == 2 && __GLIBC_MINOR__ < 33

#if HAS_XSTAT
REAL_FUNCTION_POINTER(__xstat);
REAL_FUNCTION_POINTER(__xstat64);
REAL_FUNCTION_POINTER(__fxstat);
REAL_FUNCTION_POINTER(__fxstat64);
#else
REAL_FUNCTION_POINTER(stat);
REAL_FUNCTION_POINTER(stat64);
REAL_FUNCTION_POINTER(fstat);
REAL_FUNCTION_POINTER(fstat64);
#endif

static char render_node_dir[] = "/dev/dri/";
/* Full path of /dev/dri/renderD* */
static char *render_node_path;
/* renderD* */
static char *render_node_dirent_name;
/* /sys/dev/char/major: */
static int drm_device_path_len;
static char *drm_device_path;
/* /sys/dev/char/major:minor/device */
static int device_path_len;
static char *device_path;
/* /sys/dev/char/major:minor/device/subsystem */
static char *subsystem_path;
int render_node_minor = -1;

struct file_override {
   const char *path;
   char *contents;
};
static struct file_override file_overrides[10];
static int file_overrides_count;
extern bool drm_shim_driver_prefers_first_render_node;

static int
nfvasprintf(char **restrict strp, const char *restrict fmt, va_list ap)
{
   int ret = vasprintf(strp, fmt, ap);
   assert(ret >= 0);
   return ret;
}

static int
nfasprintf(char **restrict strp, const char *restrict fmt, ...)
{
   va_list ap;
   va_start(ap, fmt);
   int ret = nfvasprintf(strp, fmt, ap);
   va_end(ap);
   return ret;
}

/* Pick the minor and filename for our shimmed render node.  This can be
 * either a new one that didn't exist on the system, or if the driver wants,
 * it can replace the first render node.
 */
static void
get_dri_render_node_minor(void)
{
   for (int i = 0; i < 10; i++) {
      UNUSED int minor = 128 + i;
      nfasprintf(&render_node_dirent_name, "renderD%d", minor);
      nfasprintf(&render_node_path, "/dev/dri/%s",
                 render_node_dirent_name);
      struct stat st;
      if (drm_shim_driver_prefers_first_render_node ||
          stat(render_node_path, &st) == -1) {

         render_node_minor = minor;
         return;
      }
   }

   fprintf(stderr, "Couldn't find a spare render node slot\n");
}

static void *get_function_pointer(const char *name)
{
   void *func = dlsym(RTLD_NEXT, name);
   if (!func) {
      fprintf(stderr, "Failed to resolve %s\n", name);
      abort();
   }
   return func;
}

#define GET_FUNCTION_POINTER(x) real_##x = get_function_pointer(#x)

void
drm_shim_override_file(const char *contents, const char *path_format, ...)
{
   assert(file_overrides_count < ARRAY_SIZE(file_overrides));

   char *path;
   va_list ap;
   va_start(ap, path_format);
   nfvasprintf(&path, path_format, ap);
   va_end(ap);

   struct file_override *override = &file_overrides[file_overrides_count++];
   override->path = path;
   override->contents = strdup(contents);
}

static void
destroy_shim(void)
{
   _mesa_set_destroy(opendir_set, NULL);
   free(render_node_path);
   free(render_node_dirent_name);
   free(subsystem_path);
}

/* Initialization, which will be called from the first general library call
 * that might need to be wrapped with the shim.
 */
static void
init_shim(void)
{
   static bool inited = false;
   drm_shim_debug = debug_get_bool_option("DRM_SHIM_DEBUG", false);

   /* We can't lock this, because we recurse during initialization. */
   if (inited)
      return;

   /* This comes first (and we're locked), to make sure we don't recurse
    * during initialization.
    */
   inited = true;

   opendir_set = _mesa_set_create(NULL,
                                  _mesa_hash_string,
                                  _mesa_key_string_equal);

   GET_FUNCTION_POINTER(close);
   GET_FUNCTION_POINTER(closedir);
   GET_FUNCTION_POINTER(dup);
   GET_FUNCTION_POINTER(fcntl);
   GET_FUNCTION_POINTER(fopen);
   GET_FUNCTION_POINTER(ioctl);
   GET_FUNCTION_POINTER(mmap);
   GET_FUNCTION_POINTER(mmap64);
   GET_FUNCTION_POINTER(open);
   GET_FUNCTION_POINTER(opendir);
   GET_FUNCTION_POINTER(readdir);
   GET_FUNCTION_POINTER(readdir64);
   GET_FUNCTION_POINTER(readlink);
   GET_FUNCTION_POINTER(realpath);

#if HAS_XSTAT
   GET_FUNCTION_POINTER(__xstat);
   GET_FUNCTION_POINTER(__xstat64);
   GET_FUNCTION_POINTER(__fxstat);
   GET_FUNCTION_POINTER(__fxstat64);
#else
   GET_FUNCTION_POINTER(stat);
   GET_FUNCTION_POINTER(stat64);
   GET_FUNCTION_POINTER(fstat);
   GET_FUNCTION_POINTER(fstat64);
#endif

   get_dri_render_node_minor();

   if (drm_shim_debug) {
      fprintf(stderr, "Initializing DRM shim on %s\n",
              render_node_path);
   }

   drm_device_path_len =
      nfasprintf(&drm_device_path, "/sys/dev/char/%d:", DRM_MAJOR);

   device_path_len =
      nfasprintf(&device_path,
                 "/sys/dev/char/%d:%d/device",
                 DRM_MAJOR, render_node_minor);

   nfasprintf(&subsystem_path,
              "/sys/dev/char/%d:%d/device/subsystem",
              DRM_MAJOR, render_node_minor);

   drm_shim_device_init();

   atexit(destroy_shim);
}

static bool hide_drm_device_path(const char *path)
{
   if (render_node_minor == -1)
      return false;

   /* If the path looks like our fake render node device, then don't hide it.
    */
   if (strncmp(path, device_path, device_path_len) == 0 ||
       strcmp(path, render_node_path) == 0)
      return false;

   /* String starts with /sys/dev/char/226: but is not the fake render node.
    * We want to hide all other drm devices for the shim.
    */
   if (strncmp(path, drm_device_path, drm_device_path_len) == 0)
      return true;

   /* String starts with /dev/dri/ but is not the fake render node. We want to
    * hide all other drm devices for the shim.
    */
   if (strncmp(path, render_node_dir, sizeof(render_node_dir) - 1) == 0)
      return true;

   return false;
}

static int file_override_open(const char *path)
{
   for (int i = 0; i < file_overrides_count; i++) {
      if (strcmp(file_overrides[i].path, path) == 0) {
         int fd = os_create_anonymous_file(0, "shim file");
         write(fd, file_overrides[i].contents,
               strlen(file_overrides[i].contents));
         lseek(fd, 0, SEEK_SET);
         return fd;
      }
   }

   return -1;
}

/* Override libdrm's reading of various sysfs files for device enumeration. */
PUBLIC FILE *fopen(const char *path, const char *mode)
{
   init_shim();

   int fd = file_override_open(path);
   if (fd >= 0)
      return fdopen(fd, "r");

   return real_fopen(path, mode);
}
PUBLIC FILE *fopen64(const char *path, const char *mode)
   __attribute__((alias("fopen")));

/* Intercepts open(render_node_path) to redirect it to the simulator. */
PUBLIC int open(const char *path, int flags, ...)
{
   init_shim();

   va_list ap;
   va_start(ap, flags);
   mode_t mode = va_arg(ap, mode_t);
   va_end(ap);

   int fd = file_override_open(path);
   if (fd >= 0)
      return fd;

   if (hide_drm_device_path(path)) {
      errno = ENOENT;
      return -1;
   }

   if (strcmp(path, render_node_path) != 0)
      return real_open(path, flags, mode);

   fd = real_open("/dev/null", O_RDWR, 0);

   drm_shim_fd_register(fd, NULL);

   return fd;
}
PUBLIC int open64(const char*, int, ...) __attribute__((alias("open")));

/* __open64_2 isn't declared unless _FORTIFY_SOURCE is defined. */
PUBLIC int __open64_2(const char *path, int flags);
PUBLIC int __open64_2(const char *path, int flags)
{
   return open(path, flags, 0);
}

PUBLIC int close(int fd)
{
   init_shim();

   drm_shim_fd_unregister(fd);

   return real_close(fd);
}

#if HAS_XSTAT
/* Fakes stat to return character device stuff for our fake render node. */
PUBLIC int __xstat(int ver, const char *path, struct stat *st)
{
   init_shim();

   /* Note: call real stat if we're in the process of probing for a free
    * render node!
    */
   if (render_node_minor == -1)
      return real___xstat(ver, path, st);

   if (hide_drm_device_path(path)) {
      errno = ENOENT;
      return -1;
   }

   /* Fool libdrm's probe of whether the /sys dir for this char dev is
    * there.
    */
   char *sys_dev_drm_dir;
   nfasprintf(&sys_dev_drm_dir,
              "/sys/dev/char/%d:%d/device/drm",
              DRM_MAJOR, render_node_minor);
   if (strcmp(path, sys_dev_drm_dir) == 0) {
      free(sys_dev_drm_dir);
      return 0;
   }
   free(sys_dev_drm_dir);

   if (strcmp(path, render_node_path) != 0)
      return real___xstat(ver, path, st);

   memset(st, 0, sizeof(*st));
   st->st_rdev = makedev(DRM_MAJOR, render_node_minor);
   st->st_mode = S_IFCHR;

   return 0;
}

/* Fakes stat to return character device stuff for our fake render node. */
PUBLIC int __xstat64(int ver, const char *path, struct stat64 *st)
{
   init_shim();

   /* Note: call real stat if we're in the process of probing for a free
    * render node!
    */
   if (render_node_minor == -1)
      return real___xstat64(ver, path, st);

   if (hide_drm_device_path(path)) {
      errno = ENOENT;
      return -1;
   }

   /* Fool libdrm's probe of whether the /sys dir for this char dev is
    * there.
    */
   char *sys_dev_drm_dir;
   nfasprintf(&sys_dev_drm_dir,
              "/sys/dev/char/%d:%d/device/drm",
              DRM_MAJOR, render_node_minor);
   if (strcmp(path, sys_dev_drm_dir) == 0) {
      free(sys_dev_drm_dir);
      return 0;
   }
   free(sys_dev_drm_dir);

   if (strcmp(path, render_node_path) != 0)
      return real___xstat64(ver, path, st);

   memset(st, 0, sizeof(*st));
   st->st_rdev = makedev(DRM_MAJOR, render_node_minor);
   st->st_mode = S_IFCHR;

   return 0;
}

/* Fakes fstat to return character device stuff for our fake render node. */
PUBLIC int __fxstat(int ver, int fd, struct stat *st)
{
   init_shim();

   struct shim_fd *shim_fd = drm_shim_fd_lookup(fd);

   if (!shim_fd)
      return real___fxstat(ver, fd, st);

   memset(st, 0, sizeof(*st));
   st->st_rdev = makedev(DRM_MAJOR, render_node_minor);
   st->st_mode = S_IFCHR;

   return 0;
}

PUBLIC int __fxstat64(int ver, int fd, struct stat64 *st)
{
   init_shim();

   struct shim_fd *shim_fd = drm_shim_fd_lookup(fd);

   if (!shim_fd)
      return real___fxstat64(ver, fd, st);

   memset(st, 0, sizeof(*st));
   st->st_rdev = makedev(DRM_MAJOR, render_node_minor);
   st->st_mode = S_IFCHR;

   return 0;
}

#else

PUBLIC int stat(const char* path, struct stat* stat_buf)
{
   init_shim();

   /* Note: call real stat if we're in the process of probing for a free
    * render node!
    */
   if (render_node_minor == -1)
      return real_stat(path, stat_buf);

   if (hide_drm_device_path(path)) {
      errno = ENOENT;
      return -1;
   }

   /* Fool libdrm's probe of whether the /sys dir for this char dev is
    * there.
    */
   char *sys_dev_drm_dir;
   nfasprintf(&sys_dev_drm_dir,
              "/sys/dev/char/%d:%d/device/drm",
              DRM_MAJOR, render_node_minor);
   if (strcmp(path, sys_dev_drm_dir) == 0) {
      free(sys_dev_drm_dir);
      return 0;
   }
   free(sys_dev_drm_dir);

   if (strcmp(path, render_node_path) != 0)
      return real_stat(path, stat_buf);

   memset(stat_buf, 0, sizeof(*stat_buf));
   stat_buf->st_rdev = makedev(DRM_MAJOR, render_node_minor);
   stat_buf->st_mode = S_IFCHR;

   return 0;
}

PUBLIC int stat64(const char* path, struct stat64* stat_buf)
{
   init_shim();

   /* Note: call real stat if we're in the process of probing for a free
    * render node!
    */
   if (render_node_minor == -1)
      return real_stat64(path, stat_buf);

   if (hide_drm_device_path(path)) {
      errno = ENOENT;
      return -1;
   }

   /* Fool libdrm's probe of whether the /sys dir for this char dev is
    * there.
    */
   char *sys_dev_drm_dir;
   nfasprintf(&sys_dev_drm_dir,
              "/sys/dev/char/%d:%d/device/drm",
              DRM_MAJOR, render_node_minor);
   if (strcmp(path, sys_dev_drm_dir) == 0) {
      free(sys_dev_drm_dir);
      return 0;
   }
   free(sys_dev_drm_dir);

   if (strcmp(path, render_node_path) != 0)
      return real_stat64(path, stat_buf);

   memset(stat_buf, 0, sizeof(*stat_buf));
   stat_buf->st_rdev = makedev(DRM_MAJOR, render_node_minor);
   stat_buf->st_mode = S_IFCHR;

   return 0;
}

PUBLIC int fstat(int fd, struct stat* stat_buf)
{
   init_shim();

   struct shim_fd *shim_fd = drm_shim_fd_lookup(fd);

   if (!shim_fd)
      return real_fstat(fd, stat_buf);

   memset(stat_buf, 0, sizeof(*stat_buf));
   stat_buf->st_rdev = makedev(DRM_MAJOR, render_node_minor);
   stat_buf->st_mode = S_IFCHR;

   return 0;
}

PUBLIC int fstat64(int fd, struct stat64* stat_buf)
{
   init_shim();

   struct shim_fd *shim_fd = drm_shim_fd_lookup(fd);

   if (!shim_fd)
      return real_fstat64(fd, stat_buf);

   memset(stat_buf, 0, sizeof(*stat_buf));
   stat_buf->st_rdev = makedev(DRM_MAJOR, render_node_minor);
   stat_buf->st_mode = S_IFCHR;

   return 0;
}
#endif

/* Tracks if the opendir was on /dev/dri. */
PUBLIC DIR *
opendir(const char *name)
{
   init_shim();

   DIR *dir = real_opendir(name);
   if (strcmp(name, "/dev/dri") == 0) {
      if (!dir) {
         /* If /dev/dri didn't exist, we still want to be able to return our
          * fake /dev/dri/render* even though we probably can't
          * mkdir("/dev/dri").  Return a fake DIR pointer for that.
          */
         dir = fake_dev_dri;
      }

      simple_mtx_lock(&shim_lock);
      _mesa_set_add(opendir_set, dir);
      simple_mtx_unlock(&shim_lock);
   }

   return dir;
}

/* If we're looking at /dev/dri, add our render node to the list
 * before the real entries in the directory.
 */
PUBLIC struct dirent *
readdir(DIR *dir)
{
   init_shim();

   struct dirent *ent = NULL;

   static struct dirent render_node_dirent = { 0 };

   simple_mtx_lock(&shim_lock);
   if (_mesa_set_search(opendir_set, dir)) {
      strcpy(render_node_dirent.d_name,
             render_node_dirent_name);
      render_node_dirent.d_type = DT_CHR;
      ent = &render_node_dirent;
      _mesa_set_remove_key(opendir_set, dir);
   }
   simple_mtx_unlock(&shim_lock);

   if (!ent && dir != fake_dev_dri)
      ent = real_readdir(dir);

   return ent;
}

/* If we're looking at /dev/dri, add our render node to the list
 * before the real entries in the directory.
 */
PUBLIC struct dirent64 *
readdir64(DIR *dir)
{
   init_shim();

   struct dirent64 *ent = NULL;

   static struct dirent64 render_node_dirent = { 0 };

   simple_mtx_lock(&shim_lock);
   if (_mesa_set_search(opendir_set, dir)) {
      strcpy(render_node_dirent.d_name,
             render_node_dirent_name);
      render_node_dirent.d_type = DT_CHR;
      ent = &render_node_dirent;
      _mesa_set_remove_key(opendir_set, dir);
   }
   simple_mtx_unlock(&shim_lock);

   if (!ent && dir != fake_dev_dri)
      ent = real_readdir64(dir);

   return ent;
}

/* Cleans up tracking of opendir("/dev/dri") */
PUBLIC int
closedir(DIR *dir)
{
   init_shim();

   simple_mtx_lock(&shim_lock);
   _mesa_set_remove_key(opendir_set, dir);
   simple_mtx_unlock(&shim_lock);

   if (dir != fake_dev_dri)
      return real_closedir(dir);
   else
      return 0;
}

/* Handles libdrm's readlink to figure out what kind of device we have. */
PUBLIC ssize_t
readlink(const char *path, char *buf, size_t size)
{
   init_shim();

   if (hide_drm_device_path(path)) {
      errno = ENOENT;
      return -1;
   }

   if (strcmp(path, subsystem_path) != 0)
      return real_readlink(path, buf, size);

   static const struct {
      const char *name;
      int bus_type;
   } bus_types[] = {
      { "/pci", DRM_BUS_PCI },
      { "/usb", DRM_BUS_USB },
      { "/platform", DRM_BUS_PLATFORM },
      { "/spi", DRM_BUS_PLATFORM },
      { "/host1x", DRM_BUS_HOST1X },
   };

   for (uint32_t i = 0; i < ARRAY_SIZE(bus_types); i++) {
      if (bus_types[i].bus_type != shim_device.bus_type)
         continue;

      strncpy(buf, bus_types[i].name, size);
      buf[size - 1] = 0;
      break;
   }

   return strlen(buf) + 1;
}

#if __USE_FORTIFY_LEVEL > 0 && !defined _CLANG_FORTIFY_DISABLE
/* Identical to readlink, but with buffer overflow check */
PUBLIC ssize_t
__readlink_chk(const char *path, char *buf, size_t size, size_t buflen)
{
   if (size > buflen)
      abort();
   return readlink(path, buf, size);
}
#endif

/* Handles libdrm's realpath to figure out what kind of device we have. */
PUBLIC char *
realpath(const char *path, char *resolved_path)
{
   init_shim();

   if (strcmp(path, device_path) != 0)
      return real_realpath(path, resolved_path);

   strcpy(resolved_path, path);

   return resolved_path;
}

/* Main entrypoint to DRM drivers: the ioctl syscall.  We send all ioctls on
 * our DRM fd to drm_shim_ioctl().
 */
PUBLIC int
ioctl(int fd, unsigned long request, ...)
{
   init_shim();

   va_list ap;
   va_start(ap, request);
   void *arg = va_arg(ap, void *);
   va_end(ap);

   struct shim_fd *shim_fd = drm_shim_fd_lookup(fd);
   if (!shim_fd)
      return real_ioctl(fd, request, arg);

   return drm_shim_ioctl(fd, request, arg);
}

/* Gallium uses this to dup the incoming fd on gbm screen creation */
PUBLIC int
fcntl(int fd, int cmd, ...)
{
   init_shim();

   struct shim_fd *shim_fd = drm_shim_fd_lookup(fd);

   va_list ap;
   va_start(ap, cmd);
   void *arg = va_arg(ap, void *);
   va_end(ap);

   int ret = real_fcntl(fd, cmd, arg);

   if (shim_fd && (cmd == F_DUPFD || cmd == F_DUPFD_CLOEXEC))
      drm_shim_fd_register(ret, shim_fd);

   return ret;
}
PUBLIC int fcntl64(int, int, ...)
   __attribute__((alias("fcntl")));

/* I wrote this when trying to fix gallium screen creation, leaving it around
 * since it's probably good to have.
 */
PUBLIC int
dup(int fd)
{
   init_shim();

   int ret = real_dup(fd);

   struct shim_fd *shim_fd = drm_shim_fd_lookup(fd);
   if (shim_fd && ret >= 0)
      drm_shim_fd_register(ret, shim_fd);

   return ret;
}

PUBLIC void *
mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset)
{
   init_shim();

   struct shim_fd *shim_fd = drm_shim_fd_lookup(fd);
   if (shim_fd)
      return drm_shim_mmap(shim_fd, length, prot, flags, fd, offset);

   return real_mmap(addr, length, prot, flags, fd, offset);
}

PUBLIC void *
mmap64(void* addr, size_t length, int prot, int flags, int fd, off64_t offset)
{
   init_shim();

   struct shim_fd *shim_fd = drm_shim_fd_lookup(fd);
   if (shim_fd)
      return drm_shim_mmap(shim_fd, length, prot, flags, fd, offset);

   return real_mmap64(addr, length, prot, flags, fd, offset);
}
