#ifndef SAVE_BYTES_H
#define SAVE_BYTES_H
#include <stddef.h>
int save_bytes_lock(const char *path);
int save_bytes_write(const char *path, const void *data, size_t length);
#endif
