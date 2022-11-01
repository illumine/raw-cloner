/*
*****************************
Simple Reverse Copy of Static Buffer Size

*****************************
*/


#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#define EX_ERROR 1


int rcp( const char *from,  const char *to ){
  int fdi, fdo;
  char buffer[BUFSIZ ] = {'\0'};
  ssize_t source_size, curr_offset, bytes_to_read, bytes_written, nr, nw, i, c;
  int saved_errno;

    printf("Copy from %s to  %s using %d bytes buffer.\n", from, to, BUFSIZ);

    fdi = open(from, O_RDONLY);
    if (fdi < 0){
        printf("Error opening %s file to read.\n", from);
        goto out_error;
    }

    fdo = open(to, O_WRONLY | O_CREAT);
    if (fdo < 0){
        printf("Error opening %s file to write.\n", to);
        goto out_error;
    }


  /* Get the source file/disk size. Seek to fdi END of the file/disk */
  source_size = lseek(fdi, 0, SEEK_END);
  if( source_size < 0){
    printf("Error seeking source to SEEK_END.\n");
    goto out_error;
  }else{
    printf("Source File/Disk size is: %ld bytes.\n", source_size);
  }

  curr_offset = lseek(fdo, source_size, SEEK_SET);
  if( curr_offset < 0){
    printf("Error seting destination to %ld.\n", source_size);
    goto out_error;
  }else{
    printf("Destination File/Disk offset is set to: %ld bytes.\n", curr_offset);
  }

  curr_offset = lseek(fdo, 0, SEEK_CUR);
  printf("Destination File/Disk offset currently: %ld bytes.\n", curr_offset);

  curr_offset = 0;
  while (curr_offset < source_size) {

    if( curr_offset + BUFSIZ > source_size )
       bytes_to_read = source_size - curr_offset;
    else
       bytes_to_read = BUFSIZ;

    /* Read source and reposition after read*/
    lseek(fdi, -bytes_to_read, SEEK_CUR);
    for(i=0; i< bytes_to_read; i++){
       nr = read( fdi,&c, 1);
       if( nr > 0 ){
          buffer[i] = c;
       }else if( nr == 0 ){
          break; //EOF
       }else
          continue;  //ERROR byte
    }
    lseek(fdi, -bytes_to_read, SEEK_CUR);

    /* Write Destination and reposition after write */
    lseek(fdo, -bytes_to_read, SEEK_CUR);
    nw = write(fdo, buffer, bytes_to_read);
    if( nw < 0){
       printf("Failed write %ld to %s.\n", bytes_to_read, to);
        goto out_error;
    }else
       bytes_written += bytes_to_read;
    lseek(fdo, -bytes_to_read, SEEK_CUR);

    /* Re-initialize buffer and increment offset */
    memset (buffer, '\0', bytes_to_read);
    curr_offset+=bytes_to_read;

  }


  return 0;

  out_error:
    saved_errno = errno;

    if (fdi >= 0)
        close(fdi);
    if (fdo >= 0)
        close(fdo);

    errno = saved_errno;
    return saved_errno;
}

int main(int argc, char * argv[]) {
  int status = EX_ERROR;

  if(argc<3){
        printf("You need to specify\n%s input_disk output_disk\n",argv[0]);
        return status;
  }else{
        return rcp( argv[1], argv[2] );
  }

}
