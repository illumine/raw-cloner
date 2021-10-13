#ifndef RAW_IO_H
# define RAW_IO_H

/* required to support 64 raw I/O. See unistd.h */
#define _FILE_OFFSET_BITS 64


ssize_t full_write(int desc, const char * ptr, size_t len);
ssize_t safe_read(int desc, void * ptr, size_t len);
int safe_close(int fd);

#ifdef __linux__ 
int input_timeout (int filedes, unsigned int seconds);
#endif

#endif
