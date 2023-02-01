 /*
*****************************
Simple Reverse Copy using a  Buffer of a given Size
Read file position is set to the ending offset of the input file.
Program copies a buffer from the ending offset to the starting offset of the input file.
If the buffer cannot be read from inpout file after a number of retries, 
then buffer_size ASCII 0s are written to the output.

*****************************
*/
#define _FILE_OFFSET_BITS 64
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <assert.h>


#define VERSION "v1.3"
#define VERSION_DATE "01/02/2023"


int copy_backwards_buffered( const char *from,  const char *to, size_t from_offset, ssize_t to_offset, size_t buffer_size, int retries ){
  int fdi=-1, fdo=-1;
  int saved_errno = 0;  
  char * buffer = NULL;
  ssize_t source_size, current_pos;
  
    printf("Reads a file/disk input backwards: from the ending offset to the starting offset.\n\
Copy from %s source from ending offset  %ld to starting offset %ld to %s using %ld bytes buffer.\n\
If the buffer cannot be read from input after %d retries,\n\
then %ld ASCII 0s are written to the output and program tries to read the next buffer to the left.\n", from, to_offset, from_offset, to, buffer_size ,retries,buffer_size);

    /* Initial Buffer Allocation according to user defined params*/
    if((buffer = (char * ) calloc(buffer_size, sizeof(char))) == NULL) {
       saved_errno = errno;  	
       printf("Error allocating buffer of size %ld. Consider using a smaller buffer size.\n", buffer_size);
       goto out_error;
    }else{
       printf("Allocated buffer of size %ld.\n", buffer_size);
    }


    fdi = open(from, O_RDONLY);
    if(fdi < 0){
      saved_errno = errno;    	
      printf("Error opening %s file to read.\n", from);
      goto out_error;
    }

    fdo = open(to, O_WRONLY | O_CREAT, 0600);
    if(fdo < 0){
      saved_errno = errno;    	
      printf("Error opening %s file to write.\n", to);
      goto out_error;
    }


    /* Get the source file/disk size in bytes */
    source_size = lseek(fdi, 0, SEEK_END);
    if( source_size < 0){
      saved_errno = errno; 
      printf("Error seeking source %s to SEEK_END.\n", to);
      goto out_error;
    }else{
      printf("Source size is: %ld bytes.\n", source_size);
    }


    /* Validate user input */    
    assert(retries > 0 );
    assert(to_offset >= from_offset);
    assert(source_size > from_offset);
    assert(source_size >= to_offset);

    printf("Start offset is: %ld bytes.\n", from_offset);
    if( to_offset == -1)
       to_offset=source_size;
    printf("End offset is: %ld bytes.\n", to_offset);
    

    /* Seek SET  source file/disk to the END*/
    current_pos = lseek(fdi, to_offset, SEEK_SET);
    if( current_pos < 0){
      saved_errno = errno; 
      printf("Error seting source to %ld.\n", to_offset);
      goto out_error;
    }else{
      printf("Source offset is set to: %ld bytes.\n", to_offset);
    }

    /* Set Destination file/disk to have same size with source. Place its position to the END*/
    current_pos = lseek(fdo, to_offset, SEEK_END);
    if( source_size < 0){
      saved_errno = errno;
      printf("Error seeking destination to %ld bytes.\n",to_offset);
      goto out_error;
    }else{
      printf("Destination size is: %ld bytes.\n", to_offset);
    }

    printf("About to read/write: %ld bytes.\n", to_offset-from_offset);

    size_t reads = (size_t)( (to_offset - from_offset) / buffer_size);
    printf("About to perform: %ld reads.\n", reads+1);
    for( size_t i = 0 ; i < reads; i++){
       /* Initialize buffer to null*/
       memset(buffer,0,buffer_size);

       /* Read buffer from in */
       current_pos = lseek(fdi, -buffer_size, SEEK_CUR);
       if( current_pos < 0){
          saved_errno = errno;       	
          printf("Could not seek source to the right for buffer bytes.\n");
          goto out_error;
       }

       char could_read = 'n';
       for( int read_retries =0; read_retries < retries; read_retries++ ){
          if( read(fdi, buffer, buffer_size) < 0){
            could_read = 'n';
	        continue;
	      }else{
	        could_read = 'y';
            break;
          }
       }

       if( could_read == 'n' ){
         printf("\n%08lx -> Could not read %ld bytes from source %s.\n", current_pos, buffer_size, from);
       }

       current_pos = lseek(fdi, -buffer_size, SEEK_CUR);
       if( current_pos < 0){
         saved_errno = errno;       	
         printf("Could not re-seek source to the right for buffer bytes.\n");
         goto out_error;
       }

       /* Write buffer to out */
       current_pos = lseek(fdo, -buffer_size, SEEK_CUR);
       if( write(fdo, buffer, buffer_size) < 0){
         saved_errno = errno;
         printf("\n%08lx -> Could not write %ld bytes to %s.\n", current_pos, buffer_size, to);
	     goto  out_error;
       } 
       lseek(fdo, -buffer_size, SEEK_CUR);
       
       /* Give some microseconds sleep period so disk/cpu won't overheat */
       usleep(2);
  }
  /* Read and write the remaining bytes of the file */
  memset(buffer,0,buffer_size);
  if( reads == 0 )
     buffer_size = to_offset - from_offset;
  else
     buffer_size = to_offset - ( reads * buffer_size + from_offset);

  current_pos = lseek(fdi, -buffer_size, SEEK_CUR);
  if( current_pos < 0){
    saved_errno = errno;
    printf("End Could not seek source to the right for buffer bytes.\n");
    goto out_error;
  }
  
  char could_read = 'n';
  for( int read_retries =0; read_retries < retries; read_retries++ ){
     if( read(fdi, buffer, buffer_size) < 0){
        could_read = 'n';
        continue;
     }else{
        could_read = 'y';
        break;
     }
  }
  
  if( could_read == 'n' ){
     printf("\r%08lx -> Could not read %ld bytes from source %s.\n", current_pos, buffer_size, from);
  }

  /* Write the remaining bytes to out */
  current_pos = lseek(fdo, -buffer_size, SEEK_CUR);
  if( write(fdo, buffer, buffer_size) < 0){
  	saved_errno = errno;
    printf("\r%08lx -> Could not write %ld bytes to %s.\n", current_pos, buffer_size, to);
    goto  out_error;
  }
  printf("Completed after %ld reads.\n",reads+1);

  if(buffer)
    free(buffer);

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
   if(buffer)
      free(buffer);
	    	
   if(fdi >= 0)
      close(fdi);
   if(fdo >= 0)
      close(fdo);

   printf("System Error %d reported: %s\n",saved_errno,strerror(saved_errno));
   return saved_errno;
}

int main(int argc, char * argv[]) {
  int status = EXIT_FAILURE;
  
  printf("%s %s %s\n",argv[0], VERSION, VERSION_DATE );
  if(argc<7){
   	 printf("You need to specify\n%s input_disk output_disk from_offset to_offset  buffer_size  retries\n",argv[0]);
	 printf("You can specify -1 for end of file in to_offset\n");
     return status;
  }else{
     return copy_backwards_buffered( argv[1], argv[2], atol(argv[3]), atol(argv[4]), atol(argv[5]), atoi(argv[6]) );
  }

}


