#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>

#ifdef __linux__
#include <sys/select.h>
#endif

#define E_TIMEOUT (-1)
#define E_SELECT  (-2)
#define E_RETRIES (-3)


#define VERSION "v1.0"
#define VERSION_DATE "16/11/2021"

int read_with_timeout_and_retries( int fdi, char * buffer, size_t buffer_size, int retries, time_t microseconds){
#ifdef __linux__
  fd_set set;
  struct timeval timeout;
  int rv;
  ssize_t nread;

  for(int i=0; i<retries; i++){
     /*
      * man select: Thus, if using select() within a loop, the
      * sets must be reinitialized before each call.
      */
     FD_ZERO(&set); /* clear the set */
     FD_SET(fdi, &set); /* add our file descriptor to the set */

     timeout.tv_sec = 0;
     timeout.tv_usec = microseconds;


     rv = select(fdi +1, &set, NULL, NULL, &timeout);
     if(rv == -1)
        return E_SELECT; /* an error accured with select()*/
     else if(rv == 0)
        return E_TIMEOUT; /* a timeout occured */
     else{
        /* there was data to read,will return 0 on EOF*/
        nread = read( fdi, buffer, buffer_size );
        if( nread < 0 )
           continue;   /* error, retry */
        else
           return nread;  /* return nread bytes or 0 (EOF) */
     }
  }
  /* could not read after retries */
  return E_RETRIES;
#endif
#ifndef __linux__
  return read( fdi, buffer, buffer_size );
#endif
}


int main(int argc, char * argv[]) {

  int fdi=-1;

  printf("%s %s %s\n",argv[0], VERSION, VERSION_DATE );
  if(argc<2){
     printf("You need to specify\n%s input_disk\n",argv[0]);
     exit(EXIT_FAILURE);
  }

  printf("Performs a read with timeout 400 micros and 3 retries from %s using %d buffersize\n", argv[1], BUFSIZ);

  fdi = open(argv[1], O_RDONLY);
  if(fdi < 0){
     printf("Error opening %s source to read.\n", argv[1]);
     exit(EXIT_FAILURE);
  }
  printf("File descriptor (fdi) %d\n",fdi);

  ssize_t nread;
  size_t  reads=0; errors =0;
  char buffer[BUFSIZ];
  while(  nread = read_with_timeout_and_retries(fdi, buffer, sizeof buffer, 3, 100) , nread != 0 ){
     reads++;
     if( nread > 0 )
        source_size += nread;
     else
     errors++;
  }

  if (fdi >= 0)
    close(fdi);

  printf("Completed. Read %ld bytes wiith %ld reads, read errors (timeouts + retries) %ld\n", source_size, reads, errors);

  exit(EXIT_SUCCESS);
}

