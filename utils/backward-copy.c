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




void copy_backwards( char *in, char *out){

  int fdi = -1, fdo = -1;
  char msg[BUFSIZ] = {'\0'};
  size_t bytes_read = 0, buf_size = 0, source_size = 0, curr_offset=0;                                                                                                           
  char * buffer = NULL;

  fdi = open( in, O_RDONLY);
  if (fdi < 0) {
    perror("Error opening input for read.\n");
    goto exit_;
  } else {
    printf("Source %s opened for read.\n", in);
  }

  fdo = open(out, O_WRONLY | O_CREAT, 0644);
  if (fdo < 0) {
    perror("Error opening output for write.\n");
    goto exit_;
  } else {
    printf("Destination %s opened for writing.\n", out);
  }

  /* Initial Buffer Allocation according to user defined params*/
  if ((buffer = (char * ) calloc( BUFSIZ, sizeof(char))) == NULL) {
    perror("Error allocating buffer. Consider using a smaller buffer size.\n");                                                                                                        
    goto exit_;
  }else{
    printf("Buffer of size %d bytes was allocated\n",BUFSIZ);
  }

  /* Get the source file/disk size. Seek to fdi END of the file/disk just before EOF */
  source_size = lseek(fdi, 0, SEEK_END);
  if( source_size < 0){
    perror("Error seeking source to SEEK_END.\n");
    goto exit_;
  }
  printf("Source File/Disk size is: %ld bytes.\n", source_size);

  curr_offset = lseek(fdo, source_size, SEEK_SET);
  if( curr_offset < 0){
    perror("Error seeking destination to SEEK_SET.\n");
    goto exit_;
  }
  printf("Destination File/Disk offset is set to: %ld bytes.\n", curr_offset);
  curr_offset = lseek(fdo, 0, SEEK_CUR);
  printf("Destination File/Disk offset currently: %ld bytes.\n", curr_offset);

  size_t bytes_to_read =0;
  int i =0, nr =0,nw=0;
  char c;
  curr_offset = 0;
  while (curr_offset < source_size) {
    memset(buffer,0,BUFSIZ);
    if( curr_offset + BUFSIZ > source_size )
       bytes_to_read = source_size - curr_offset;
    else
       bytes_to_read = BUFSIZ;

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
    curr_offset+=bytes_to_read;

  } //while


exit_:
	
  /* post processing steps */
  if(buffer)
     free(buffer);
  if(fdi>0)
    close(fdi);
  if(fdo >0)
    close(fdo);
    
}






int main(int argc, char * argv[]) {

  if( argc < 3) {
      printf("\nUsage: backwards [filename] [out-filename]\n");
      return EXIT_FAILURE;
  }
  
  copy_backwards( argv[1], argv[2]);
  
  return EXIT_SUCCESS;
}

