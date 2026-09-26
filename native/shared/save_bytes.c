#define _POSIX_C_SOURCE 200809L
#include "save_bytes.h"
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/file.h>
#include <unistd.h>

int save_bytes_lock(const char *path) {
  char name[4096];
  if (snprintf(name, sizeof(name), "%s.lock", path) >= (int)sizeof(name))
    return -1;
  int fd = open(name, O_CREAT | O_RDWR, 0600);
  if (fd < 0) return -1;
  if (flock(fd, LOCK_EX) < 0) {
    close(fd);
    return -1;
  }
  return fd;
}

int save_bytes_write(const char *path, const void *data, size_t length) {
  char temporary[4096], directory[4096];
  if (snprintf(temporary, sizeof(temporary), "%s.tmp", path) >=
      (int)sizeof(temporary)) return -1;
  int fd = open(temporary, O_CREAT | O_TRUNC | O_WRONLY, 0600);
  if (fd < 0) return -1;
  FILE *file = fdopen(fd, "w");
  if (!file) {
    close(fd);
    return -1;
  }
  int failed = fwrite(data, 1, length, file) != length;
  if (fflush(file) || fsync(fd)) failed = 1;
  if (fclose(file)) failed = 1;
  if (failed || rename(temporary, path)) return -1;
  if (snprintf(directory, sizeof(directory), "%s", path) >=
      (int)sizeof(directory)) return -1;
  char *slash = strrchr(directory, '/');
  if (!slash) return -1;
  if (slash == directory) slash[1] = 0;
  else *slash = 0;
  fd = open(directory, O_RDONLY);
  if (fd < 0) return -1;
  failed = fsync(fd);
  close(fd);
  return failed ? -1 : 0;
}
