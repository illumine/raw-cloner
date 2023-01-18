/*
*****************************
Simple Fιle/Disk Forward Copy of Static Buffer Size

Copy input file/disk from offset  to offset to destination file/disk using  bytes buffer.
Program tries to read the buffer from the source for  times and if it fails, it moves on to next buffer read after skip_bytes.

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


#define VERSION "v1.1"
#define VERSION_DATE "17/01/2023"


int forward_copy_buffer( const char *from,  const char *to, size_t from_offset, ssize_t to_offset, size_t buffer_size, int retries, size_t skip_bytes){
  int fdi=-1, fdo=-1;
  int saved_errno = 0;
  char * buffer = NULL;

    printf("Copy from %s from offset %ld to offset %ld to destination %s using %ld bytes buffer.\n\
Program tries to read the buffer from the source for %d times and if it fails, it skips %ld bytes and moves on to next buffer read.\n", 
    from, from_offset, to_offset, to, buffer_size, retries, skip_bytes);


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
    assert(retries > 0 );
    assert(to_offset >= from_offset);
    assert(source_size > from_offset);
    assert(source_size >= to_offset);
    assert(source_size > skip_bytes );

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
      int read_retries;
      ssize_t nread, nwritten;
      memset(buffer,0,buffer_size);

      /* Give some microseconds sleep period so disk/cpu won't overheat */
      usleep(1);
      if( current_pos + buffer_size > to_offset )
         buffer_size = to_offset - current_pos + 1;
	     
      for( read_retries =0; read_retries < retries; read_retries++ ){
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
               printf("\r%08ld Read %ld bytes, Written %ld bytes for %ld times %d retries.",current_pos, nread, nwritten, times_read, read_retries);
            break;
        }
     }// retries = MAX_READ_RETRIES
     
     /* Problem on reading: we could not read after MAX_READ_RETRIES, move the offset to buffer_size*/
     if( read_retries == retries  ){
        printf("\n%08ld Read Retries %d completed for a buffer of %ld bytes failed. Moving forward %ld bytes.\n",current_pos, read_retries, buffer_size, buffer_size );
        if( skip_bytes > 0 )
		   current_pos = lseek(fdi, skip_bytes, SEEK_CUR);
	    else
	       current_pos = lseek(fdi, buffer_size, SEEK_CUR);
        if( current_pos < 0 ){
           saved_errno = errno;
           printf("Cannot lseek the input file. lseek returned %ld. Aborting.\n",current_pos);
           goto out_error;
        }else
           printf("File position moved to %ld.\n", current_pos);
     }


     current_pos = lseek(fdi, 0, SEEK_CUR);
     if( current_pos < 0 ){
     	saved_errno = errno;
        printf("Cannot lseek the input file. lseek returned %ld. Aborting.\n",current_pos);
        goto out_error;
     }
     
     if( current_pos == source_size ){
        printf("Reached end of file/disk at %ld bytes.\n",current_pos);     
        break;
     }
     if( current_pos > to_offset ){
        printf("Reached end offset at %ld bytes.\n",current_pos);
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
  if(argc<8){
   	 printf("You need to specify\n%s input_disk output_disk from_offset to_offset  buffer_size  retries  skip_bytes\n",argv[0]);
	 printf("You can specify -1 for end of file in to_offset\n");
	 printf("If skip_bytes = 0 then program moves current offset to +buffer_size. Ifskip_bytes > 0 then program moves current offset to +skip_bytes \n");
  	 return status;
  }else{
  	 return forward_copy_buffer( argv[1], argv[2], atol(argv[3]), atol(argv[4]), atol(argv[5]), atoi(argv[6]), atol(argv[7]) );
  }

}

