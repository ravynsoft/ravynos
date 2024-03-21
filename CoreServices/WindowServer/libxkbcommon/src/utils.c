/*
 * Copyright © 2013 Ran Benita <ran234@gmail.com>
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

#include "config.h"

#include "utils.h"

#ifdef HAVE_MMAP

#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>

bool
map_file(FILE *file, char **string_out, size_t *size_out)
{
    struct stat stat_buf;
    int fd;
    char *string;

    /* Make sure to keep the errno on failure! */
    fd = fileno(file);
    if (fd < 0)
        return false;

    if (fstat(fd, &stat_buf) != 0)
        return false;

    string = mmap(NULL, stat_buf.st_size, PROT_READ, MAP_SHARED, fd, 0);
    if (string == MAP_FAILED)
        return false;

    *string_out = string;
    *size_out = stat_buf.st_size;
    return true;
}

void
unmap_file(char *str, size_t size)
{
    munmap(str, size);
}

#else

bool
map_file(FILE *file, char **string_out, size_t *size_out)
{
    long ret;
    size_t ret_s;
    char *string;
    size_t size;

    /* Make sure to keep the errno on failure! */

    ret = fseek(file, 0, SEEK_END);
    if (ret != 0)
        return false;

    ret = ftell(file);
    if (ret < 0)
        return false;
    size = (size_t) ret;

    ret = fseek(file, 0, SEEK_SET);
    if (ret < 0)
        return false;

    string = malloc(size);
    if (!string)
        return false;

    ret_s = fread(string, 1, size, file);
    if (ret_s < size) {
        free(string);
        return false;
    }

    *string_out = string;
    *size_out = size;
    return true;
}

void
unmap_file(char *str, size_t size)
{
    free(str);
}

#endif

// ASCII lower-case map.
static const unsigned char lower_map[] = {
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20,
    21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39,
    40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58,
    59, 60, 61, 62, 63, 64, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107,
    108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122,
    91, 92, 93, 94, 95, 96, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107,
    108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122,
    123, 124, 125, 126, 127, 128, 129, 130, 131, 132, 133, 134, 135, 136, 137,
    138, 139, 140, 141, 142, 143, 144, 145, 146, 147, 148, 149, 150, 151, 152,
    153, 154, 155, 156, 157, 158, 159, 160, 161, 162, 163, 164, 165, 166, 167,
    168, 169, 170, 171, 172, 173, 174, 175, 176, 177, 178, 179, 180, 181, 182,
    183, 184, 185, 186, 187, 188, 189, 190, 191, 192, 193, 194, 195, 196, 197,
    198, 199, 200, 201, 202, 203, 204, 205, 206, 207, 208, 209, 210, 211, 212,
    213, 214, 215, 216, 217, 218, 219, 220, 221, 222, 223, 224, 225, 226, 227,
    228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239, 240, 241, 242,
    243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 254, 255
};

// ASCII tolower (to avoid locale issues).
char
to_lower(char c)
{
    return (char) lower_map[(unsigned char) c];
}

// ASCII strcasecmp (to avoid locale issues).
int
istrcmp(const char *a, const char *b)
{
    for (size_t i = 0; ; i++) {
        if (to_lower(a[i]) != to_lower(b[i]))
            return (int) to_lower(a[i]) - (int) to_lower(b[i]);
        if (!a[i])
            break;
    }
    return 0;
}

// ASCII strncasecmp (to avoid locale issues).
int
istrncmp(const char *a, const char *b, size_t n)
{
    for (size_t i = 0; i < n; i++) {
        if (to_lower(a[i]) != to_lower(b[i]))
            return (int) to_lower(a[i]) - (int) to_lower(b[i]);
        if (!a[i])
            break;
    }
    return 0;
}

#if !(defined(HAVE_ASPRINTF) && HAVE_ASPRINTF)
int
asprintf(char **strp, const char *fmt, ...)
{
    int ret;
    va_list ap;
    va_start(ap, fmt);
    ret = vasprintf(strp, fmt, ap);
    va_end(ap);
    return ret;
}

# if !(defined(HAVE_VASPRINTF) && HAVE_VASPRINTF)
int
vasprintf(char **strp, const char *fmt, va_list ap)
{
    int ret;
    char *buf;
    va_list ap_copy;

    /*
     * The value of the va_list parameter is undefined after the call to
     * vsnprintf() returns: pass a copy to make sure "ap" remains valid.
     */
    va_copy(ap_copy, ap);
    ret = vsnprintf(NULL, 0, fmt, ap_copy);
    va_end(ap_copy);

    if (ret < 0)
        return ret;

    if (!(buf = malloc(ret + 1)))
        return -1;

    if ((ret = vsnprintf(buf, ret + 1, fmt, ap)) < 0) {
        free(buf);
        return ret;
    }

    *strp = buf;
    return ret;
}
# endif /* !HAVE_VASPRINTF */
#endif /* !HAVE_ASPRINTF */
