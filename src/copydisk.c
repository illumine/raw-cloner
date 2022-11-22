/*
*****************************
Simple FIle/Disk Forward Copy of Static Buffer Size

It retries 3 times to read the buffer. If it fails, to read it then makes buffer = buffer/2 the half of initial buffer
Until buffer becomes 1 byte. If this byte cannot be read, the filepos is moved one byte forward.

*****************************
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <assert.h>


#define MAX_READ_RETRIES 3
#define ONE_BYTE 1

#define VERSION "v1.2"
#define VERSION_DATE "22/11/2021"


int forward_copy_with_bisect_buffer( const char *from,  const char *to, size_t offset){
  int fdi=-1, fdo=-1;
  int saved_errno = 0;

    printf("Copy from %s from offset %ld to %s using %d bytes buffer with Bisect method.\n\
Program tries to read the buffer from the source for %d times.\n\
If it fails, then the buffer is cut to the half and try to read from the same offset.\n\
Procedure repeats until buffer has size of 1 byte.\n\
If byte cannot be read then the filepos is moved after that byte and a new buffer of %d bytes is used.\n", 
    from, offset, to, BUFSIZ, MAX_READ_RETRIES, BUFSIZ);

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
	
	
	// WHY????
    /* Get the source file/disk size. Seek to fdi END of the file/disk */
    ssize_t source_size = lseek(fdi, 0, SEEK_END);
    if( source_size < 0){
       saved_errno = errno;    	
       printf("Error setting source File/Disk to SEEK_END.\n");
       goto out_error;
    }else{
       printf("Source File/Disk size is: %ld bytes.\n", source_size);
    }
 
    /* Validate user offset */
    assert(source_size > offset);
    


    ssize_t current_pos = lseek(fdi, offset, SEEK_SET);	
    if( current_pos >= 0 ){
        printf("Positioning source File/Disk to offset: %ld bytes.\n", offset);
    }else{
        saved_errno = errno;		 	
        printf("Error setting source to offset: %ld bytes.\n", offset);
        goto out_error;		 	
    }


    /* Reading the file loop */
    size_t  times_read=0;
    while(1){
      ssize_t nread, nwritten;
      char buf[BUFSIZ] = {0};
      size_t  buf_siz = sizeof buf;
      int read_retries=0;

    read_form_disk:
      /* Give some microseconds sleep period so disk/cpu won't overheat */
      usleep(2);
	  		
      for( read_retries =0; read_retries < MAX_READ_RETRIES; read_retries++ ){
         times_read++;     
         nread = read(fdi, buf, buf_siz);
         if( nread < 0 )
	    /* ERROR do retry read */
            continue;
         else if (nread == 0 )
	    /* EOF */	
            break;
        else{
	    /* Read OK, Write the butes */
            nwritten = write(fdo, buf, nread);
            if( nwritten < 0 ){ 
               saved_errno = errno;            
               printf("Could not write %ld bytes to %s. Aborting.\n",nwritten, to );
	       goto out_error;
            }else	       
	       printf("\r%08lx Read %ld bytes, Written %ld bytes for %ld times.",current_pos, nread, nwritten, times_read);
            break;
       }
     }// retries = MAX_READ_RETRIES
     
     /* Problem on reading: we could not read after MAX_READ_RETRIES: Cut buffer siz in the middle and try again */
     if( read_retries == MAX_READ_RETRIES && buf_siz > ONE_BYTE ){
        printf("Read Retries %d completed for a buffer of %ld bytes failed. Making new buffer size %ld bytes and try to read again.\n", read_retries, buf_siz, (ssize_t)(1+buf_siz/2) );
        buf_siz = buf_siz/2;
        goto read_form_disk;
     }

    /* Problem on reading: we could not read after MAX_READ_RETRIES with a buffer of 1 byte. Move the file position 1 byte forward */
     if( read_retries == MAX_READ_RETRIES && buf_siz == ONE_BYTE ){
        printf("Read Retries %d completed for a buffer of ONE BYTE failed. Making new buffer size %ld bytes and moving file position ONE BYTE Forward.\n", read_retries, sizeof buf );
        current_pos = lseek(fdi, ONE_BYTE, SEEK_CUR);      
        printf("File position moved to %ld.\n", current_pos);
        buf_siz = sizeof buf;
        goto read_form_disk;
     }

     current_pos = lseek(fdi, 0, SEEK_CUR);
     if( current_pos < 0 ){
     	saved_errno = errno;
        printf("Cannot lseek the input file. lseek returned %ld. Aborting.\n",current_pos);
        goto out_error;
     }

     if( current_pos == source_size ){
        printf("Reached end of file at %ld bytes.\n",current_pos);     
        break;
     }

   }//while


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
  
  printf("%s %s %s\n",argv[0], VERSION, VERSION_DATE );
  if(argc<4){
  	printf("You need to specify\n%s input_disk output_disk offset\n",argv[0]);
  	return status;
  }else{
  	return forward_copy_with_bisect_buffer( argv[1], argv[2], atol(argv[3]) );
  }

}
