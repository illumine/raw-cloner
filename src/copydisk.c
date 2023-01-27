/*
*****************************
Simple File/Disk Forward Copy of using a Buffer

It retries  times to read the buffer. If it fails, to read it then makes buffer = buffer/2 the half of initial buffer
Until buffer becomes 1 byte. If this byte cannot be read, the filepos is moved one byte forward.

*****************************
*/
#define _FILE_OFFSET_BITS 64
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <assert.h>


#define ONE_BYTE 1

#define VERSION "v1.3"
#define VERSION_DATE "29/11/2022"


int forward_copy_with_bisect_buffer( const char *from,  const char *to, size_t from_offset, ssize_t to_offset, size_t buffer_size, int retries){
  int fdi=-1, fdo=-1;
  int saved_errno = 0;
  char * buffer = NULL;

    printf("Copy from %s from offset %ld to offset %ld to destination %s using %ld bytes buffer with Bisect method.\n\
Program tries to read the buffer from the source for %d times.\n\
If it fails, then the buffer is cut to the half and try to read from the same offset.\n\
Procedure repeats until buffer has size of 1 byte.\n\
If byte cannot be read then the filepos is moved after that byte and a new buffer of %ld bytes is used.\n", 
    from, from_offset, to_offset, to, buffer_size, retries, buffer_size);


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
	
    /* Get the source file/disk size. Seek to fdi END of the file/disk */
    ssize_t source_size = lseek(fdi, 0, SEEK_END);
    if( source_size < 0){
       saved_errno = errno;    	
       printf("Error setting source File/Disk to SEEK_END.\n");
       goto out_error;
    }else{
       printf("Source File/Disk size is: %ld bytes.\n", source_size);
    }
 
    printf("Start offset is: %ld bytes.\n", from_offset);
    if( to_offset == -1)
       to_offset=source_size;
    printf("End offset is: %ld bytes.\n", to_offset);
	
	 
    /* Validate user input */
    assert(source_size > from_offset);
    assert(source_size >= to_offset);      
    assert(retries > 0 );
    assert(to_offset >= from_offset);


    ssize_t current_pos = lseek(fdi, from_offset, SEEK_SET);	
    if( current_pos >= 0 ){
        printf("Positioning source File/Disk to offset: %ld bytes.\n", from_offset);
    }else{
        saved_errno = errno;		 	
        printf("Error setting source to offset: %ld bytes.\n", from_offset);
        goto out_error;		 	
    }


    /* Reading the file loop */
    size_t  times_read=0;
    while(1){
      ssize_t nread, nwritten;
      int read_retries=0;

    read_form_disk:
      /* Give some microseconds sleep period so disk/cpu won't overheat */
      usleep(2);
	  		
      for( read_retries =0; read_retries < retries ; read_retries++ ){
         times_read++;     
         nread = read(fdi, buffer, buffer_size);
         if( nread < 0 )
	    /* ERROR do retry read */
            continue;
         else if (nread == 0 )
	    /* EOF */	
            break;
        else{
	    /* Read OK, Write the butes */
            nwritten = write(fdo, buffer, nread);
            if( nwritten < 0 ){ 
               saved_errno = errno;            
               printf("Could not write %ld bytes to %s. Aborting.\n",nwritten, to );
	       goto out_error;
            }else	       
	       printf("\r%08ld Read %ld bytes, Written %ld bytes for %ld times.",current_pos, nread, nwritten, times_read);
            break;
       }
     }// retries = MAX_READ_RETRIES
     
     /* Problem on reading: we could not read after MAX_READ_RETRIES: Cut buffer siz in the middle and try again */
     if( read_retries == retries && buffer_size > ONE_BYTE ){
        printf("Read Retries %d completed for a buffer of %ld bytes failed. Making new buffer size %ld bytes and try to read again.\n", read_retries, buffer_size, (ssize_t)(1+buffer_size/2) );
        buffer_size = buffer_size/2;
        goto read_form_disk;
     }

    /* Problem on reading: we could not read after MAX_READ_RETRIES with a buffer of 1 byte. Move the file position 1 byte forward */
     if( read_retries == retries && buffer_size == ONE_BYTE ){
        printf("%08ld Read Retries %d completed for a buffer of ONE BYTE failed. Making new buffer size %ld bytes and moving file position ONE BYTE Forward.\n",current_pos, read_retries, sizeof buffer );
        current_pos = lseek(fdi, ONE_BYTE, SEEK_CUR);      
        printf("File position moved to %ld.\n", current_pos);
        buffer_size = sizeof buffer;
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

   if(buffer)
      free(buffer);

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
  	
    if(buffer)
      free(buffer);    	
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
  if(argc<7){
   	 printf("You need to specify\n%s input_disk output_disk from_offset to_offset  buffer_size  retries\n",argv[0]);
	 printf("You can specify -1 for end of file in to_offset\n");
  	 return status;
  }else{
  	return forward_copy_with_bisect_buffer( argv[1], argv[2], atol(argv[3]), atol(argv[4]), atol(argv[5]), atoi(argv[6]) );
  }

}
