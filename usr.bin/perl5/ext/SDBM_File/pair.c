/*
 * sdbm - ndbm work-alike hashed database library
 * based on Per-Aake Larson's Dynamic Hashing algorithms. BIT 18 (1978).
 * author: oz@nexus.yorku.ca
 * status: public domain.
 *
 * page-level routines
 */

#include "config.h"
#ifdef __CYGWIN__
# define EXTCONST extern const
#else
# include "EXTERN.h"
#endif
#include "sdbm.h"
#include "tune.h"
#include "pair.h"

#define exhash(item)    sdbm_hash((item).dptr, (item).dsize)

/* 
 * forward 
 */
static int seepair(char *, int, const char *, int);

/*
 * page format:
 *      +------------------------------+
 * ino  | n | keyoff | datoff | keyoff |
 *      +------------+--------+--------+
 *      | datoff | - - - ---->         |
 *      +--------+---------------------+
 *      |        F R E E A R E A       |
 *      +--------------+---------------+
 *      |  <---- - - - | data          |
 *      +--------+-----+----+----------+
 *      |  key   | data     | key      |
 *      +--------+----------+----------+
 *
 * calculating the offsets for free area:  if the number
 * of entries (ino[0]) is zero, the offset to the END of
 * the free area is the block size. Otherwise, it is the
 * nth (ino[ino[0]]) entry's offset.
 */

int
fitpair(char *pag, int need)
{
        int n;
        int off;
        int free;
        short *ino = (short *) pag;

        off = ((n = ino[0]) > 0) ? ino[n] : PBLKSIZ;
        free = off - (n + 1) * sizeof(short);
        need += 2 * sizeof(short);

        debug(("free %d need %d\n", free, need));

        return need <= free;
}

void
putpair(char *pag, datum key, datum val)
{
        int n;
        int off;
        short *ino = (short *) pag;

        off = ((n = ino[0]) > 0) ? ino[n] : PBLKSIZ;
/*
 * enter the key first
 */
        off -= key.dsize;
        (void) memcpy(pag + off, key.dptr, key.dsize);
        ino[n + 1] = off;
/*
 * now the data
 */
        off -= val.dsize;
        (void) memcpy(pag + off, val.dptr, val.dsize);
        ino[n + 2] = off;
/*
 * adjust item count
 */
        ino[0] += 2;
}

datum
getpair(char *pag, datum key)
{
        int i;
        int n;
        datum val;
        short *ino = (short *) pag;

        if ((n = ino[0]) == 0)
                return nullitem;

        if ((i = seepair(pag, n, key.dptr, key.dsize)) == 0)
                return nullitem;

        val.dptr = pag + ino[i + 1];
        val.dsize = ino[i] - ino[i + 1];
        return val;
}

int
exipair(char *pag, datum key)
{
        short *ino = (short *) pag;

        if (ino[0] == 0)
                return 0;

        return (seepair(pag, ino[0], key.dptr, key.dsize) != 0);
}

#ifdef SEEDUPS
int
duppair(char *pag, datum key)
{
        short *ino = (short *) pag;
        return ino[0] > 0 && seepair(pag, ino[0], key.dptr, key.dsize) > 0;
}
#endif

datum
getnkey(char *pag, int num)
{
        datum key;
        int off;
        short *ino = (short *) pag;

        num = num * 2 - 1;
        if (ino[0] == 0 || num > ino[0])
                return nullitem;

        off = (num > 1) ? ino[num - 1] : PBLKSIZ;

        key.dptr = pag + ino[num];
        key.dsize = off - ino[num];

        return key;
}

int
delpair(char *pag, datum key)
{
        int n;
        int i;
        short *ino = (short *) pag;

        if ((n = ino[0]) == 0)
                return 0;

        if ((i = seepair(pag, n, key.dptr, key.dsize)) == 0)
                return 0;
/*
 * found the key. if it is the last entry
 * [i.e. i == n - 1] we just adjust the entry count.
 * hard case: move all data down onto the deleted pair,
 * shift offsets onto deleted offsets, and adjust them.
 * [note: 0 < i < n]
 */
        if (i < n - 1) {
                int m;
                char *dst = pag + (i == 1 ? PBLKSIZ : ino[i - 1]);
                char *src = pag + ino[i + 1];
                int   zoo = dst - src;

                debug(("free-up %d ", zoo));
/*
 * shift data/keys down
 */
                m = ino[i + 1] - ino[n];
#ifdef DUFF
#define MOVB    *--dst = *--src

                if (m > 0) {
                        int loop = (m + 8 - 1) >> 3;

                        switch (m & (8 - 1)) {
                        case 0: do {
                                MOVB; /* FALLTHROUGH */ case 7: MOVB; /* FALLTHROUGH */
                        case 6: MOVB; /* FALLTHROUGH */ case 5: MOVB; /* FALLTHROUGH */
                        case 4: MOVB; /* FALLTHROUGH */ case 3: MOVB; /* FALLTHROUGH */
                        case 2: MOVB; /* FALLTHROUGH */ case 1: MOVB;
                                } while (--loop);
                        }
                }
#else
                dst -= m;
                src -= m;
                memmove(dst, src, m);
#endif
/*
 * adjust offset index up
 */
                while (i < n - 1) {
                        ino[i] = ino[i + 2] + zoo;
                        i++;
                }
        }
        ino[0] -= 2;
        return 1;
}

/*
 * search for the key in the page.
 * return offset index in the range 0 < i < n.
 * return 0 if not found.
 */
static int
seepair(char *pag, int n, const char *key, int siz)
{
        int i;
        int off = PBLKSIZ;
        short *ino = (short *) pag;

        for (i = 1; i < n; i += 2) {
                if (siz == off - ino[i] &&
                    memEQ(key, pag + ino[i], siz))
                        return i;
                off = ino[i + 1];
        }
        return 0;
}

void
splpage(char *pag, char *New, long int sbit)
{
        datum key;
        datum val;

        int n;
        int off = PBLKSIZ;
        char cur[PBLKSIZ];
        short *ino = (short *) cur;

        (void) memcpy(cur, pag, PBLKSIZ);
        (void) memset(pag, 0, PBLKSIZ);
        (void) memset(New, 0, PBLKSIZ);

        n = ino[0];
        for (ino++; n > 0; ino += 2) {
                key.dptr = cur + ino[0]; 
                key.dsize = off - ino[0];
                val.dptr = cur + ino[1];
                val.dsize = ino[0] - ino[1];
/*
 * select the page pointer (by looking at sbit) and insert
 */
                (void) putpair((exhash(key) & sbit) ? New : pag, key, val);

                off = ino[1];
                n -= 2;
        }

        debug(("%d split %d/%d\n", ((short *) cur)[0] / 2, 
               ((short *) New)[0] / 2,
               ((short *) pag)[0] / 2));
}

/*
 * check page sanity: 
 * number of entries should be something
 * reasonable, and all offsets in the index should be in order.
 * this could be made more rigorous.
 */
/*
  Layout of a page is:
  Top of block:
  number of keys/values (short)
  Array of (number of keys/values) offsets, alternating between key offsets
  and value offsets (shorts)
  End of block:
   - value/key data, last key ends at end of block (bytes)

  So:
    N key0off val0off key1off val1off ... val1 key1 val0 key0

  Be careful to note N is the number of offsets, *not* the number of keys.
 */
int
chkpage(char *pag)
{
        int n;
        int off;
        short *ino = (short *) pag;

        if ((n = ino[0]) < 0 || n > (int)(PBLKSIZ / sizeof(short)))
                return 0;

        if (n > 0) {
                off = PBLKSIZ;
                for (ino++; n > 0; ino += 2) {
                        if (ino[0] > off || ino[1] > off ||
                            ino[1] > ino[0] || ino[1] <= 0)
                                return 0;
                        off = ino[1];
                        n -= 2;
                }
                /* there must be an even number of offsets */
                if (n != 0)
                    return 0;
                /* check the key/value offsets don't overlap the key/value data */
                if ((char *)ino > pag + off)
                    return 0;
        }
        return 1;
}
