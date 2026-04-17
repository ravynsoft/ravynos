#ifndef _hfs_iterate_hfs_metadata_h
#define _hfs_iterate_hfs_metadata_h

/*
 * Given a device name, a function pointer, and
 * a context pointer, call the function pointer for
 * each metadata extent in the HFS+ filesystem.
 */
extern int iterate_hfs_metadata(char *, int (*)(int, off_t, off_t, void*), void *);

#endif
