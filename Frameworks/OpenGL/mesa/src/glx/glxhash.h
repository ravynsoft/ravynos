#ifndef _GLX_HASH_H_
#define _GLX_HASH_H_


typedef struct __glxHashTable __glxHashTable;

/* Hash table routines */
extern __glxHashTable *__glxHashCreate(void);
extern int __glxHashDestroy(__glxHashTable * t);
extern int __glxHashLookup(__glxHashTable * t, unsigned long key,
                           void **value);
extern int __glxHashInsert(__glxHashTable * t, unsigned long key,
                           void *value);
extern int __glxHashDelete(__glxHashTable * t, unsigned long key);
extern int __glxHashFirst(__glxHashTable * t, unsigned long *key,
                          void **value);
extern int __glxHashNext(__glxHashTable * t, unsigned long *key,
                         void **value);

#endif /* _GLX_HASH_H_ */
