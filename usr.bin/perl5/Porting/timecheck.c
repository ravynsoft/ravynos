/* A helper tool for perl's 2038 support.
 *	See Porting/README.y2038 for details
 */

#include <sys/types.h>
#include <stdio.h>
#include <time.h>
#include <errno.h>
#include <string.h>

int opt_v = 0;
int i;
struct tm *tmp;
time_t pt, pt_max, pt_min;

static char hexbuf[80];
char *hex (time_t t)
{
    if ((long long)t < 0)
        sprintf (hexbuf, " -0x%016lx", -t);
    else
        sprintf (hexbuf, "  0x%016lx",  t);
    return (hexbuf);
    } /* hex */

void gm_check (time_t t, int min_year, int max_year)
{
    tmp = gmtime (&t);
    if ( tmp == NULL ||
        /* Check tm_year overflow */
         tmp->tm_year < min_year || tmp->tm_year > max_year) {
        if (opt_v)
            fprintf (stderr, "gmtime (%ld) failed with errno %d\n", t, errno);
        }
    else {
        if (opt_v)
            fprintf (stderr, "%3d:%s: %12ld-%02d-%02d %02d:%02d:%02d\n",
                i, hex (t),
                (long)(tmp->tm_year) + 1900, tmp->tm_mon + 1, tmp->tm_mday,
                tmp->tm_hour, tmp->tm_min, tmp->tm_sec);
        pt = t;
        }
    } /* gm_check */

int check_gm_max ()
{
    tmp = NULL;
    pt  = 0;
    if (tmp == NULL || tmp->tm_year < 0) {
        for (i = 63; i >= 0; i--) {
            time_t x = pt | ((time_t)1 << i);
            if (x < 0 || x < pt) continue;
            gm_check (x, 69, 0x7fffffff);
            }
        }
    pt_max = pt;
    return (0);
    } /* check_gm_max */

int check_gm_min ()
{
    tmp = NULL;
    pt  = 0;
    if (tmp == NULL) {
        for (i = 36; i >= 0; i--) {
            time_t x = pt - ((time_t)1 << i);
            if (x > 0) continue;
            gm_check (x, -1900, 70);
            }
        }
    pt_min = pt;
    return (0);
    } /* check_gm_min */

void lt_check (time_t t, int min_year, int max_year)
{
    if (sizeof (time_t) > 4 && t > 0x7ffffffffffff000LL)
        tmp = NULL;
    else
        tmp = localtime (&t);
    if ( tmp == NULL ||
        /* Check tm_year overflow */
         tmp->tm_year < min_year || tmp->tm_year > max_year) {
        if (opt_v)
            fprintf (stderr, "localtime (%ld) failed with errno %d\n", t, errno);
        }
    else {
        if (opt_v)
            fprintf (stderr, "%3d:%s: %12ld-%02d-%02d %02d:%02d:%02d\n",
                i, hex (t),
                (long)(tmp->tm_year) + 1900, tmp->tm_mon + 1, tmp->tm_mday,
                tmp->tm_hour, tmp->tm_min, tmp->tm_sec);
        pt = t;
        }
    } /* lt_check */

int check_lt_max ()
{
    tmp = NULL;
    pt  = 0;
    if (tmp == NULL || tmp->tm_year < 0) {
        for (i = 63; i >= 0; i--) {
            time_t x = pt | ((time_t)1 << i);
            if (x < 0 || x < pt) continue;
            lt_check (x, 69, 0x7fffffff);
            }
        }
    pt_max = pt;
    return (0);
    } /* check_lt_max */

int check_lt_min ()
{
    tmp = NULL;
    pt  = 0;
    if (tmp == NULL) {
        for (i = 36; i >= 0; i--) {
            time_t x = pt - ((time_t)1 << i);
            if (x > 0) continue;
            lt_check (x, -1900, 70);
            }
        }
    pt_min = pt;
    return (0);
    } /* check_lt_min */

int main (int argc, char *argv[])
{
    time_t gm_max, gm_min, lt_max, lt_min;
    if (argc > 1 && strcmp (argv[1], "-v") == 0) opt_v++;

    check_gm_max (); gm_max = pt_max;
    check_gm_min (); gm_min = pt_min;
    check_lt_max (); lt_max = pt_max;
    check_lt_min (); lt_min = pt_min;

    opt_v++;
    printf ("======================\n");
    printf ("Sizeof time_t = %d\n", (i = sizeof (time_t)));
    printf ("gmtime () boundaries:\n");
    gm_check (gm_max,    69, 0x7fffffff);
    gm_check (gm_min, -1900,         70);
    printf ("localtime () boundaries:\n");
    lt_check (lt_max,    69, 0x7fffffff);
    lt_check (lt_min, -1900,         70);
    printf ("Configure variables:\n");
    printf ("sGMTIME_max='%ld'\n",    gm_max);
    printf ("sGMTIME_min='%ld'\n",    gm_min);
    printf ("sLOCALTIME_max='%ld'\n", lt_max);
    printf ("sLOCALTIME_min='%ld'\n", lt_min);
    return (0);
    } /* main */
