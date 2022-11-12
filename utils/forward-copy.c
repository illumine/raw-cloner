/*
*****************************
Simple Copy of Static Buffer Size

*****************************
*/


#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#define EX_ERROR 1



int copy_forward( const char *from,  const char *to ){
  int fdi, fdo;
  char buffer[BUFSIZ ] = {'\0'};
  ssize_t nread;

  printf("Copy from %s to  %s using %d bytes buffer.\n", from, to, BUFSIZ);

  fdi = open( from, O_RDONLY);
  if (fdi < 0) {
    perror("Error opening input for read.\n");
    goto out_error;
  } else {
    printf("Source %s opened for read.\n", from);
  }

  fdo = open(to, O_WRONLY | O_CREAT, 0644);
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
  int status = EX_ERROR;

  if(argc<3){
        printf("You need to specify\n%s input_disk output_disk\n",argv[0]);
        return status;
  }else{
        return copy_forward( argv[1], argv[2] );
  }

}

