/*
 * attr.c - extended attributes (xattr) manipulation
 *
 * This file is part of zsh, the Z shell.
 *
 * Copyright (c) 2009 Mikael Magnusson
 * All rights reserved.
 *
 * Permission is hereby granted, without written agreement and without
 * license or royalty fees, to use, copy, modify, and distribute this
 * software and to distribute modified versions of this software for any
 * purpose, provided that the above copyright notice and the following
 * two paragraphs appear in all copies of this software.
 *
 * In no event shall Mikael Magnusson or the Zsh Development Group be liable
 * to any party for direct, indirect, special, incidental, or consequential
 * damages arising out of the use of this software and its documentation,
 * even if Andrew Main and the Zsh Development Group have been advised of
 * the possibility of such damage.
 *
 * Mikael Magnusson and the Zsh Development Group specifically disclaim any
 * warranties, including, but not limited to, the implied warranties of
 * merchantability and fitness for a particular purpose.  The software
 * provided hereunder is on an "as is" basis, and Mikael Magnusson and the
 * Zsh Development Group have no obligation to provide maintenance,
 * support, updates, enhancements, or modifications.
 *
 */

#include <sys/types.h>
#include <sys/xattr.h>

#include "attr.mdh"
#include "attr.pro"

static ssize_t
xgetxattr(const char *path, const char *name, void *value, size_t size, int symlink)
{
#ifdef XATTR_EXTRA_ARGS
    return getxattr(path, name, value, size, 0, symlink ? XATTR_NOFOLLOW: 0);
#else
    switch (symlink) {
    case 0:
        return getxattr(path, name, value, size);
    default:
        return lgetxattr(path, name, value, size);
    }
#endif
}

static ssize_t
xlistxattr(const char *path, char *list, size_t size, int symlink)
{
#ifdef XATTR_EXTRA_ARGS
    return listxattr(path, list, size, symlink ? XATTR_NOFOLLOW : 0);
#else
    switch (symlink) {
    case 0:
        return listxattr(path, list, size);
    default:
        return llistxattr(path, list, size);
    }
#endif
}

static int
xsetxattr(const char *path, const char *name, const void *value,
          size_t size, int flags, int symlink)
{
#ifdef XATTR_EXTRA_ARGS
    return setxattr(path, name, value, size, 0, flags | symlink ? XATTR_NOFOLLOW : 0);
#else
    switch (symlink) {
    case 0:
        return setxattr(path, name, value, size, flags);
    default:
        return lsetxattr(path, name, value, size, flags);
    }
#endif
}

static int
xremovexattr(const char *path, const char *name, int symlink)
{
#ifdef XATTR_EXTRA_ARGS
    return removexattr(path, name, symlink ? XATTR_NOFOLLOW : 0);
#else
    switch (symlink) {
    case 0:
        return removexattr(path, name);
    default:
        return lremovexattr(path, name);
    }
#endif
}

static int
bin_getattr(char *nam, char **argv, Options ops, UNUSED(int func))
{
    int ret = 0;
    int val_len = 0, attr_len = 0, slen;
    char *value, *file = argv[0], *attr = argv[1], *param = argv[2];
    int symlink = OPT_ISSET(ops, 'h');

    unmetafy(file, &slen);
    unmetafy(attr, NULL);
    val_len = xgetxattr(file, attr, NULL, 0, symlink);
    if (val_len == 0) {
        if (param)
            unsetparam(param);
        return 0;
    }
    if (val_len > 0) {
        value = (char *)zalloc(val_len+1);
        attr_len = xgetxattr(file, attr, value, val_len, symlink);
        if (attr_len > 0 && attr_len <= val_len) {
            value[attr_len] = '\0';
            if (param)
                setsparam(param, metafy(value, attr_len, META_DUP));
            else
                printf("%s\n", value);
        }
        zfree(value, val_len+1);
    }
    if (val_len < 0 || attr_len < 0 || attr_len > val_len)  {
        zwarnnam(nam, "%s: %e", metafy(file, slen, META_NOALLOC), errno);
        ret = 1 + ((val_len > 0 && attr_len > val_len) || attr_len < 0);
    }
    return ret;
}

static int
bin_setattr(char *nam, char **argv, Options ops, UNUSED(int func))
{
    int ret = 0, slen, vlen;
    int symlink = OPT_ISSET(ops, 'h');
    char *file = argv[0], *attr = argv[1], *value = argv[2];

    unmetafy(file, &slen);
    unmetafy(attr, NULL);
    unmetafy(value, &vlen);
    if (xsetxattr(file, attr, value, vlen, 0, symlink)) {
        zwarnnam(nam, "%s: %e", metafy(file, slen, META_NOALLOC), errno);
        ret = 1;
    }
    return ret;
}

static int
bin_delattr(char *nam, char **argv, Options ops, UNUSED(int func))
{
    int ret = 0, slen;
    int symlink = OPT_ISSET(ops, 'h');
    char *file = argv[0], **attr = argv;

    unmetafy(file, &slen);
    while (*++attr) {
        unmetafy(*attr, NULL);
        if (xremovexattr(file, *attr, symlink)) {
            zwarnnam(nam, "%s: %e", metafy(file, slen, META_NOALLOC), errno);
            ret = 1;
            break;
        }
    }
    return ret;
}

static int
bin_listattr(char *nam, char **argv, Options ops, UNUSED(int func))
{
    int ret = 0;
    int val_len, list_len = 0, slen;
    char *value, *file = argv[0], *param = argv[1];
    int symlink = OPT_ISSET(ops, 'h');

    unmetafy(file, &slen);
    val_len = xlistxattr(file, NULL, 0, symlink);
    if (val_len == 0) {
        if (param)
            unsetparam(param);
        return 0;
    }
    if (val_len > 0) {
        value = (char *)zalloc(val_len+1);
        list_len = xlistxattr(file, value, val_len, symlink);
        if (list_len > 0 && list_len <= val_len) {
            char *p = value;
            if (param) {
                int arrlen = 0;
                char **array = NULL, **arrptr = NULL;

                while (p < &value[list_len]) {
                    arrlen++;
                    p += strlen(p) + 1;
                }
                arrptr = array = (char **)zshcalloc((arrlen+1) * sizeof(char *));
                p = value;
                while (p < &value[list_len]) {
                    *arrptr++ = metafy(p, -1, META_DUP);
                    p += strlen(p) + 1;
                }
                setaparam(param, array);
            } else while (p < &value[list_len]) {
                printf("%s\n", p);
                p += strlen(p) + 1;
            }
        }
        zfree(value, val_len+1);
    }
    if (val_len < 0 || list_len < 0 || list_len > val_len) {
        zwarnnam(nam, "%s: %e", metafy(file, slen, META_NOALLOC), errno);
        ret = 1 + (list_len > val_len || list_len < 0);
    }
    return ret;
}

/* module paraphernalia */

static struct builtin bintab[] = {
    BUILTIN("zgetattr", 0, bin_getattr, 2, 3, 0, "h", NULL),
    BUILTIN("zsetattr", 0, bin_setattr, 3, 3, 0, "h", NULL),
    BUILTIN("zdelattr", 0, bin_delattr, 2, -1, 0, "h", NULL),
    BUILTIN("zlistattr", 0, bin_listattr, 1, 2, 0, "h", NULL),
};

static struct features module_features = {
    bintab, sizeof(bintab)/sizeof(*bintab),
    NULL, 0,
    NULL, 0,
    NULL, 0,
    0
};

/**/
int
setup_(UNUSED(Module m))
{
    return 0;
}

/**/
int
features_(Module m, char ***features)
{
    *features = featuresarray(m, &module_features);
    return 0;
}

/**/
int
enables_(Module m, int **enables)
{
    return handlefeatures(m, &module_features, enables);
}

/**/
int
boot_(UNUSED(Module m))
{
    return 0;
}

/**/
int
cleanup_(Module m)
{
    return setfeatureenables(m, &module_features, NULL);
}

/**/
int
finish_(UNUSED(Module m))
{
    return 0;
}
