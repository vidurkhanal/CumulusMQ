#ifndef IO_UTILS_H
#define IO_UTILS_H

#include <cassert>
#include <cstdio>
#include <unistd.h>

namespace IOUtils {

void msg(const char *msg);

/*
The read() syscall just returns whatever data is available in the kernel, or
blocks if there is none. It’s up to the application to handle insufficient data.
The read_full() function reads from the kernel until it gets exactly n bytes.
*/
int32_t read_exact(int fd, char *buf, size_t n);

/*
The write() syscall may return successfully with partial data written if the
kernel buffer is full, we must keep trying when the write() returns fewer bytes
than we need.
*/
int32_t write_all(int fd, const char *buf, size_t n);
} // namespace IOUtils
#endif // !DEBUG
