#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>


#define BS 100

#define ONE_BYTE 1
#define SOURCE 0
#define DESTINATION 1
#define EX_ERROR 1
#define EX_OK 0

void copy_backwards( char *in, char *out){

  int fdi = -1, fdo = -1;
  char msg[BUFSIZ] = {'\0'};
  size_t bytes_read = 0, buf_size = 0, bytes_writen = 0, curr_offset = 0, source_size =0;                                                                                                           _size =0;
  char * buffer = NULL;


  fdi = open( in, O_RDONLY);
  if (fdi < 0) {
    sprintf(msg, "\nError opening %s for read.\n", in);
    perror(msg);
    exit(EX_ERROR);
  } else {
    printf("\nSource %s opened for read.\n", in);
  }

  fdo = open(out, O_WRONLY | O_CREAT, 0644);
  if (fdo < 0) {
    sprintf(msg, "\nError opening file %s for write.\n", out);
    perror(msg);
    exit(EX_ERROR);
  } else {
    printf("\nDestination %s bytes opened for writing.\n", out);
  }

  /* Initial Buffer Allocation according to user defined params*/
  if ((buffer = (char * ) calloc( BS, sizeof(char))) == NULL) {
    sprintf(msg, "\nError allocating buffer of size %d. Consider using a smaller buffer size.\n", BS);                                                                                                          buffer size.\n", BS);
    perror(msg);
    exit(EX_ERROR);
  }else{
    bzero(buffer,BS);
    printf("\nBuffer of size %d bytes was allocated\n",BS);
  }

  /* Get the source file/disk size. Seek to fdi END of the file/disk just before                                                                                                              EOF */
  //source_size = info_lseek(fdi, -1, SEEK_END, SOURCE, msg, 1);
  source_size = lseek(fdi, 0, SEEK_END);
  if( source_size < 0){
    sprintf(msg, "\nError seeking source to SEEK_END.\n");
    perror(msg);
    exit(EX_ERROR);
  }
  printf("\nSource File/Disk size is: %ld bytes.\n", source_size);

  curr_offset = lseek(fdo, source_size, SEEK_SET);
  if( curr_offset < 0){
    sprintf(msg, "\nError seeking destination to SEEK_SET.\n");
    perror(msg);
    exit(EX_ERROR);
  }
  printf("\nDestination File/Disk offset is set to: %ld bytes.\n", curr_offset);
  curr_offset = lseek(fdo, 0, SEEK_CUR);
  printf("\nDestination File/Disk offset currently: %ld bytes.\n", curr_offset);

  size_t bytes_to_read =0;
  int i =0, nr =0,nw=0;
  char c;
  curr_offset = 0;
  while (curr_offset < source_size) {

    if( curr_offset + BS > source_size )
       bytes_to_read = source_size - curr_offset;
    else
       bytes_to_read = BS;

    lseek(fdi, -bytes_to_read, SEEK_CUR);
    for(i=0; i< bytes_to_read; i++){
       nr = read( fdi,&c,1);
       if( nr > 0 )
          buffer[i] = c;
    }
    lseek(fdi, -bytes_to_read, SEEK_CUR);

    lseek(fdo, -bytes_to_read, SEEK_CUR);
    nw = write(fdo, buffer, bytes_to_read);
    lseek(fdo, -bytes_to_read, SEEK_CUR);
    bzero(buffer,bytes_to_read);
    curr_offset+=bytes_to_read;

  } //while


  /* post processing steps */
  free(buffer);
  close(fdi);
  close(fdo);
  return;

}






int main(int argc, char * argv[]) {

  if( argc < 3) {
      printf("\nUsage: backwards [filename] [out-filename]\n");
      return 1;
  }
  copy_backwards( argv[1], argv[2]);
  return 0;
}

