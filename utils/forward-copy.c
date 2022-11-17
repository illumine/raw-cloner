/*
*****************************
Simple Copy of a file using a Static Buffer Size.
It brakes on first error

*****************************
*/

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>


#define VERSION "v1.0"
#define VERSION_DATE "16/11/2021"

int copy_forward( const char *from,  const char *to ){
  int fdi=-1, fdo=-1;
  char buffer[BUFSIZ ] = {'\0'};
  ssize_t nread;

  printf("Copy from %s to  %s using %d bytes buffer. It brakes on first read error!\n", from, to, BUFSIZ);

  fdi = open( from, O_RDONLY);
  if (fdi < 0) {
    perror("Error opening input for read.\n");
    goto out_error;
  } else {
    printf("Source %s opened for read.\n", from);
  }

  fdo = open(to, O_WRONLY | O_CREAT, 0600);
  if (fdo < 0) {
    perror("Error opening output for write.\n");
    goto out_error;
  } else {
    printf("Destination %s opened for writing.\n", to);
  }

  while (nread = read(fdi, buffer, sizeof buffer), nread > 0){
     char *out_ptr = buffer;
     ssize_t nwritten;

        do {
            nwritten = write(fdo, out_ptr, nread);

            if (nwritten >= 0){
                nread -= nwritten;
                out_ptr += nwritten;
            }else if (errno != EINTR){
                goto out_error;
            }
        } while (nread > 0);
    }

    if (nread == 0){
        if (close(fdo) < 0){
            fdo = -1;
            goto out_error;
        }
        close(fdi);

        /* Success! */
        printf("Completed\n");
        return 0;
    }

    
  out_error:

    if (fdi >= 0)
        close(fdi);
    if (fdo >= 0)
        close(fdo);
    return -1;
}

int main(int argc, char * argv[]) {

  printf("%s %s %s\n",argv[0], VERSION, VERSION_DATE );
  if(argc<3){
        printf("You need to specify\n%s input_disk output_disk\n",argv[0]);
        return EXIT_FAILURE;
  }else{
        return copy_forward( argv[1], argv[2] );
  }

}

