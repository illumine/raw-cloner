#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>  

#include "util.h"

size_t get_file_size(const char * path){

  FILE *fp = fopen(path, "rb");
  if (!fp)
    return -1;
  
	
  if( fseek(fp, 0L, SEEK_END) < 0 )
    return -1;
  
  return ftell(fp);	
	
}

/* Sleep for some miliseconds */
int msleep(long msec){
	
    struct timespec ts;
    int res;

    if (msec < 0){
        errno = EINVAL;
        return -1;
    }

    ts.tv_sec = msec / 1000;
    ts.tv_nsec = (msec % 1000) * 1000000;

    do {
        res = nanosleep(&ts, &ts);
    } while (res && errno == EINTR);

    return res;
}
