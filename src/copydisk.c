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



int cp( const char *from,  const char *to ){
  int fd_to, fd_from;
  char buf[BUFSIZ ] = {'\0'};
  ssize_t nread;
  int saved_errno;

    printf("Copy from %s to  %s using %d bytes buffer.\n", from, to, BUFSIZ);

    fd_from = open(from, O_RDONLY);
    if (fd_from < 0){
        printf("Error opening %s file to read.\n", from);
        goto out_error;
        }

    fd_to = open(to, O_WRONLY | O_CREAT);
    if (fd_to < 0){
        printf("Error opening %s file to write.\n", to);
        goto out_error;
        }

    while (nread = read(fd_from, buf, sizeof buf), nread > 0){
        char *out_ptr = buf;
        ssize_t nwritten;

        do {
            nwritten = write(fd_to, out_ptr, nread);

            if (nwritten >= 0){
                nread -= nwritten;
                out_ptr += nwritten;
            }else if (errno != EINTR){
                goto out_error;
            }
        } while (nread > 0);
    }

    if (nread == 0){
        if (close(fd_to) < 0){
            fd_to = -1;
            goto out_error;
        }
        close(fd_from);

        /* Success! */
        return 0;
    }

  out_error:
    saved_errno = errno;

    if (fd_from >= 0)
        close(fd_from);
    if (fd_to >= 0)
        close(fd_to);

    errno = saved_errno;
    return saved_errno;
}

int main(int argc, char * argv[]) {
  int status = EX_ERROR;

  if(argc<3){
        printf("You need to specify\n%s input_disk output_disk\n",argv[0]);
        return status;
  }else{
        return cp( argv[1], argv[2] );
  }

}

