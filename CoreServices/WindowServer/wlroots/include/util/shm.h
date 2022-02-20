#ifndef UTIL_SHM_H
#define UTIL_SHM_H

#include <stdbool.h>

int create_shm_file(void);
int allocate_shm_file(size_t size);
bool allocate_shm_file_pair(size_t size, int *rw_fd, int *ro_fd);

#endif
