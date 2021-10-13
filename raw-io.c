#include <stdio.h>
#include <unistd.h>

#include <errno.h>
#include <sys/types.h>
#include <sys/time.h>

#include "raw-io.h"



ssize_t full_write(int desc, const char * ptr, size_t len) {
  size_t total_written;

  total_written = 0;
  while (len > 0) {
    int written = write(desc, ptr, len);
    /* write on an old Slackware Linux 1.2.13 returns zero when
	 I try to write more data than there is room on a floppy disk.
	 This puts dd into an infinite loop.  Reproduce with
	 dd if=/dev/zero of=/dev/fd0.  If you have this problem,
	 consider upgrading to a newer kernel.  */
    if (written < 0) {
      #ifdef EINTR
      if (errno == EINTR)
        continue;
      #endif
      return written;
    }
    total_written += written;
    ptr += written;
    len -= written;
  }
  return total_written;
}

ssize_t safe_read(int desc, void * ptr, size_t len) {
  ssize_t n_chars;

  if (len <= 0)
    return len;

  #ifdef EINTR
  do {
    n_chars = read(desc, ptr, len);
  } while (n_chars < 0 && errno == EINTR);
  #else
  n_chars = read(desc, ptr, len);
  #endif

  return n_chars;
}

int safe_close(int fd) {
  if (close(fd) != 0)
    do
      if (errno != EINTR)
        return -1;
    while (close(fd) != 0 && errno != EBADF);

  return 0;
}

#ifdef __linux__ 
int input_timeout (int filedes, unsigned int seconds)
{
  fd_set set;
  struct timeval timeout;

  /* Initialize the file descriptor set. */
  FD_ZERO (&set);
  FD_SET (filedes, &set);

  /* Initialize the timeout data structure. */
  timeout.tv_sec = seconds;
  timeout.tv_usec = 0;

  /* select returns 0 if timeout, 1 if input available, -1 if error. */
  return select (1, &set, NULL, NULL, &timeout);
}
#endif 
