 /*
*****************************
Simple Reverse Copy using a Static Buffer Size
Read file position is set to the end of the input file.
Program copies a buffer from the end to the start of the input file.
If the buffer cannot be read from inpout file, 
then BUFSIZ ASCII 0s are written to the output.

*****************************
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>

#define MAX_READ_RETRIES 3


#define VERSION "v1.1"
#define VERSION_DATE "18/11/2021"


int copy_backwards_buffered( const char *from,  const char *to ){
  int fdi=-1, fdo=-1;
  ssize_t source_size, current_pos;
  
    printf("Reads a file/disk input backwards byte by byte.\n\
Copy from %s to  %s using %d bytes buffer.\n\
If the buffer cannot be read from input after %d retries,\n\
then %d ASCII 0s are written to the output.\n", from, to, BUFSIZ,MAX_READ_RETRIES,BUFSIZ);

    fdi = open(from, O_RDONLY);
    if(fdi < 0){
      printf("Error opening %s file to read.\n", from);
      goto out_error;
    }

    fdo = open(to, O_WRONLY | O_CREAT, 0600);
    if(fdo < 0){
      printf("Error opening %s file to write.\n", to);
      goto out_error;
    }


    /* Get the source file/disk size in bytes */
    source_size = lseek(fdi, 0, SEEK_END);
    if( source_size < 0){
      printf("Error seeking source to SEEK_END.\n");
      goto out_error;
    }else{
      printf("Source size is: %ld bytes.\n", source_size);
    }

    /* Seek SET  source file/disk to the END*/
    current_pos = lseek(fdi, source_size, SEEK_SET);
    if( current_pos < 0){
      printf("Error seting source to %ld.\n", source_size);
      goto out_error;
    }else{
      printf("Source offset is set to: %ld bytes.\n", current_pos);
    }

    /* Set Destination file/disk to have same size with source. Place its position to the END*/
    current_pos = lseek(fdo, source_size, SEEK_END);
    if( source_size < 0){
      printf("Error seeking destination to %ld bytes.\n",source_size);
      goto out_error;
    }else{
      printf("Destination size is: %ld bytes.\n", source_size);
    }


    char buf[BUFSIZ] = {0};
    size_t buf_size = sizeof buf;

    size_t reads = (size_t)(source_size / buf_size);
    for( size_t i = 0 ; i < reads; i++){
       /* Initialize buffer to null*/
       memset(buf,0,buf_size);

       /* Read buffer from in */
       current_pos = lseek(fdi, -buf_size, SEEK_CUR);
       if( current_pos < 0){
          perror("Could not seek source to the right for buffer bytes.\n");
          goto out_error;
       }
     
       char could_read = 'n';
       for( int read_retries =0; read_retries < MAX_READ_RETRIES; read_retries++ ){
          if( read(fdi, buf, buf_size) < 0){
            could_read = 'n';
	        continue;
	      }else{
	        could_read = 'y';
            break;
	      }
       }
       if( could_read == 'n' ){
         printf("\r%08lx -> Could not read %ld bytes from source %s.\n", current_pos, buf_size, from);
       } 
       current_pos = lseek(fdi, -buf_size, SEEK_CUR);
       if( current_pos < 0){
         perror("Could not re-seek source to the right for buffer bytes.\n");
         goto out_error;
       }

       /* Write buffer to out */
       current_pos = lseek(fdo, -buf_size, SEEK_CUR);
       if( write(fdo, buf, buf_size) < 0){
         printf("\r%08lx -> Could not write %ld bytes to %s.\n", current_pos, buf_size, to);
	     goto  out_error;
       } 
       lseek(fdo, -buf_size, SEEK_CUR);
       
       /* Give some microseconds sleep period so disk/cpu won't overheat */
       usleep(2);
  }

  /* Read and write the remaining bytes of the file */
  memset(buf,0,buf_size);
 
  buf_size = source_size - reads * buf_size;
  
  current_pos = lseek(fdi, -buf_size, SEEK_CUR);
  if( current_pos < 0){
    perror("Could not seek source to the right for buffer bytes.\n");
    goto out_error;
  }
  
  char could_read = 'n';
  for( int read_retries =0; read_retries < MAX_READ_RETRIES; read_retries++ ){
     if( read(fdi, buf, buf_size) < 0){
        could_read = 'n';
        continue;
     }else{
        could_read = 'y';
        break;
     }
  }
  
  if( could_read == 'n' ){
     printf("\r%08lx -> Could not read %ld bytes from source %s.\n", current_pos, buf_size, from);
  }
  current_pos = lseek(fdi, -buf_size, SEEK_CUR);
  if( current_pos < 0){
    perror("Could not re-seek source to the right for buffer bytes.\n");
    goto out_error;
  }
  
  /* Write the remaining bytes to out */
  current_pos = lseek(fdo, -buf_size, SEEK_CUR);
  if( write(fdo, buf, buf_size) < 0){
    printf("\r%08lx -> Could not write %ld bytes to %s.\n", current_pos, buf_size, to);
    goto  out_error;
  }
  printf("Completed after %ld reads.\n",reads+1);

  if(close(fdo) < 0){
    printf("Could not close %s.\n",to);
    fdo=-1;
    goto out_error;
  }
  
  if( close(fdi) < 0){
    printf("Could not close %s.\n",from);
    fdi=-1;      
    goto out_error;   	
  }

  /* Success! */
  return 0;

	
  out_error:
    if(fdi >= 0)
      close(fdi);
    if(fdo >= 0)
       close(fdo);

   return -1;
}

int main(int argc, char * argv[]) {
  int status = EXIT_FAILURE;
  
  printf("%s %s %s\n",argv[0], VERSION, VERSION_DATE );
  if(argc<3){
        printf("You need to specify\n%s input_disk output_disk\n",argv[0]);
        return status;
  }else{
        return copy_backwards_buffered( argv[1], argv[2] );
  }

}

