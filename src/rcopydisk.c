 /*
*****************************
Simple Reverse Copy using a Static Buffer Size

*****************************
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>

#define MAX_READ_RETRIES 3

int rcp( const char *from,  const char *to ){
  int fdi=-1, fdo=-1;
  ssize_t source_size, current_pos;
  int saved_errno; 
  
    printf("Reverse Copy from %s to  %s using %d bytes buffer.\n", from, to, BUFSIZ);

    fdi = open(from, O_RDONLY);
    if (fdi < 0){
       saved_errno = errno;    	
       printf("Error opening %s file to read.\n", from);
       goto out_error;
    }

    fdo = open(to, O_WRONLY | O_CREAT, 0600);
    if (fdo < 0){
       saved_errno = errno;    	
       printf("Error opening %s file to write.\n", to);
       goto out_error;
    }


  /* Get the source file/disk size in bytes */
   source_size = lseek(fdi, 0, SEEK_END);
   if( source_size < 0){
      saved_errno = errno;  	
      printf("Error seeking source to SEEK_END.\n");
      goto out_error;
  }else{
      printf("Source File/Disk size is: %ld bytes.\n", source_size);
  }

  /* Seek SET  source file/disk to the END*/
  current_pos = lseek(fdi, source_size, SEEK_SET);
  if( current_pos < 0){
      saved_errno = errno;  	
      printf("Error seting source to %ld.\n", source_size);
      goto out_error;
  }else{
      printf("Source File/Disk offset is set to: %ld bytes.\n", current_pos);
  }

  /* Set Destination file/disk to have same size with source. Place its position to the END*/
  current_pos = lseek(fdo, source_size, SEEK_END);
  if( source_size < 0){
      saved_errno = errno;  	
      printf("Error seeking destination to %ld bytes.\n",source_size);
      goto out_error;
  }else{
      printf("Destination File/Disk size is: %ld bytes.\n", source_size);
  }


  char buf[BUFSIZ] = {0};
  size_t buf_size = sizeof buf;

  size_t reads = (size_t)(source_size / buf_size);
  for( size_t i = 0 ; i < reads; i++){
     /* Initialize buffer to null*/
     memset(buf,0,buf_size);

     /* Read buffer from in */
     current_pos = lseek(fdi, -buf_size, SEEK_CUR);
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
     lseek(fdi, -buf_size, SEEK_CUR);
     if( could_read == 'n' ){
        printf("\r%08lx -> Could not read %ld bytes from %s.\n", current_pos, buf_size, from);
     } 

     /* Write buffer to out */
     current_pos = lseek(fdo, -buf_size, SEEK_CUR);
     if( write(fdo, buf, buf_size) < 0){
     	saved_errno = errno;
        printf("\r%08lx -> Could not write %ld bytes to %s.\n", current_pos, buf_size, to);
	    goto  out_error;
     } 
     lseek(fdo, -buf_size, SEEK_CUR);
     
     /* Give some microseconds sleep period so disk/cpu won't overheat */
     usleep(5);
  }

  /* Read and write the remaining bytes of the file */
  memset(buf,0,buf_size);
 
  buf_size = source_size - reads * buf_size;
  
  current_pos = lseek(fdi, -buf_size, SEEK_CUR);
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
  lseek(fdi, -buf_size, SEEK_CUR);
  if( could_read == 'n' ){
     printf("\r%08lx -> Could not read %ld bytes from %s.\n", current_pos, buf_size, from);
  }
  /* Write the remaining bytes to out */
  current_pos = lseek(fdo, -buf_size, SEEK_CUR);
  if( write(fdo, buf, buf_size) < 0){
      saved_errno = errno;
      printf("\r%08lx -> Could not write %ld bytes to %s.\n", current_pos, buf_size, to);
      goto  out_error;
  }


   if(close(fdo) < 0){
      saved_errno = errno;
      printf("Could not close %s.\n",to);
      fdo=-1;
      goto out_error;
   }
   if( close(fdi) < 0){
      saved_errno = errno;
      printf("Could not close %s.\n",from);
      fdi=-1;      
      goto out_error;   	
   }

  /* Success! */
  return 0;

	
  out_error:
    if (fdi >= 0)
        close(fdi);
    if (fdo >= 0)
        close(fdo);

    printf("System Error %d reported: %s\n",saved_errno,strerror(saved_errno));
    return saved_errno;
}

int main(int argc, char * argv[]) {
  int status = EXIT_FAILURE;

  if(argc<3){
        printf("You need to specify\n%s input_disk output_disk\n",argv[0]);
        return status;
  }else{
        return rcp( argv[1], argv[2] );
  }

}

