 /*
*****************************
File/Disk Forward Copy of using a Buffer

It retries times to read the buffer of buffer_size. 
If read failed: it skips skip_bytes = exp(skip_bytes,i) and sets j=0, i++. Where i is the i-th continous time it FAILED to read. 
Else it makes the buffer_size=exp(buffer_size,j) and sets i=0, j++. Where j is the j-th continous time it SUCCED to read.
integer i : [2  Imax],  j : [2, Jmax]

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

#define VERSION "v1.0"
#define VERSION_DATE "26/01/2023"


int forward_copy_with_exponential_method( 
       const char *from,  const char *to, 
       size_t from_offset, ssize_t to_offset, int retries,
       size_t buffer_size_initial,  size_t buffer_size_limit,                                   
       size_t skip_bytes_initial,    size_t skip_bytes_limit   ){
          
  int fdi=-1, fdo=-1;
  int saved_errno = 0;
  char * buffer = NULL;

    printf("Copy from %s from offset %ld to offset %ld to destination %s using %ld bytes initial buffer with exponential method:\n",  from, from_offset, to_offset, to, buffer_size_initial);
    printf("It retries %d times to read the buffer of buffer_size.\n",retries);
    printf("If read failed: it skips bytes = exp(%ld,i) and sets j=0, i++. Where i is the i-th continous time it FAILED to read.\n", buffer_size_initial);
    printf("Else it makes the buffer_size=exp(%ld,j) and sets i=0, j++. Where j is the j-th continous time it SUCCEED to read.\n", skip_bytes_initial );
    printf("Integer failed-i : [1  %ld],  succeded-j : [1, %ld]\n",  skip_bytes_limit, buffer_size_limit);   

    /* Initial Buffer Allocation according to user defined params*/
    if((buffer = (char * ) calloc(buffer_size_initial, sizeof(char))) == NULL) {
       saved_errno = errno;     
       printf("Error allocating buffer of size %ld. Consider using a smaller buffer size.\n", buffer_size_initial);
       goto out_error;
    }else{
        printf("Allocated buffer of size %ld.\n", buffer_size_initial);
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
    assert(source_size >= from_offset);
    assert(buffer_size_limit >= buffer_size_initial );
    assert(skip_bytes_limit >= skip_bytes_initial);

    ssize_t current_pos = lseek(fdi, from_offset, SEEK_SET);   
    if( current_pos >= 0 ){
        printf("Positioning source File/Disk to offset: %ld bytes.\n", from_offset);
    }else{
        saved_errno = errno;          
        printf("Error setting source to offset: %ld bytes.\n", from_offset);
        goto out_error;          
    }

   char success = 'F';
   size_t  times_read=0;
   size_t buffer_size = buffer_size_initial;
   size_t skip_bytes  = skip_bytes_initial;
   
   while(1){
      ssize_t nread, nwritten;
      int read_retries=0;

      usleep(2);
      success = 'F';
     
      memset(buffer,0,buffer_size);
      for( read_retries =0; read_retries < retries ; read_retries++ ){
         times_read++;     
         nread = read(fdi, buffer, buffer_size);
         if( nread < 0 )
            /* ERROR do retry read */
            continue;
         else if (nread == 0 ){
            /* EOF */   
            success = 'T';
            break;
         }else{
            /* Read OK, Write the bytes */
            nwritten = write(fdo, buffer, nread);
            if( nwritten < 0 ){ 
               saved_errno = errno;            
               printf("Could not write %ld bytes to %s. Aborting.\n",nwritten, to );
               goto out_error;
            }else          
               printf("\r%08ld Read %ld bytes, Written %ld bytes for %ld times.",current_pos, nread, nwritten, times_read);
            success = 'T';     
            break;
         }
      }//for
     
      if( success == 'F' ){
         printf("Read Retries %d completed for a buffer of %ld bytes failed. Skipping %ld bytes and try to read again.\n", read_retries, buffer_size, skip_bytes );
         if( skip_bytes > skip_bytes_limit )
            skip_bytes = skip_bytes_limit;
         current_pos = lseek(fdi,0, SEEK_CUR); 
         if( current_pos < 0 ){
            saved_errno = errno;
            printf("Cannot lseek the input file. lseek returned %ld. Aborting.\n",current_pos);
            goto out_error;
         }         
         
         if( current_pos + skip_bytes > to_offset ){
            printf("Reached to_offset : %ld\n",to_offset);
            break;
	 }else{
            current_pos = lseek(fdi, skip_bytes, SEEK_CUR);
            if( current_pos < 0 ){
               saved_errno = errno;
               printf("Cannot lseek the input file. lseek returned %ld. Aborting.\n",current_pos);
               goto out_error;
            } 
         }
         skip_bytes = skip_bytes * skip_bytes_initial;
         buffer_size = buffer_size_initial;
		 
      }else{
         printf("Read Retries %d completed for a buffer of %ld bytes succeeded.\n", read_retries, buffer_size );
         buffer_size = buffer_size * buffer_size_initial;   
         if(  buffer_size > buffer_size_limit )
            buffer_size = buffer_size_limit;
         if(buffer)
            free(buffer);
         if((buffer = (char * ) calloc(buffer_size, sizeof(char))) == NULL) {
            saved_errno = errno;     
            printf("Error allocating buffer of size %ld. Consider using a smaller buffer size.\n", buffer_size);
            goto out_error;
         }
      }

     current_pos = lseek(fdi, 0, SEEK_CUR);
     if( current_pos < 0 ){
     	saved_errno = errno;
        printf("Cannot lseek the input file. lseek returned %ld. Aborting.\n",current_pos);
        goto out_error;
     }

     if( current_pos >= to_offset ){
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
  if(argc<10){
       printf("You need to specify\n%s input_disk output_disk from_offset to_offset retries buffer_size_initial buffer_size_limit skip_bytes_initial skip_bytes_limit\n",argv[0]);
    printf("You can specify -1 for end of file in to_offset\n");
      return status;
  }else{
     
     return forward_copy_with_exponential_method( argv[1], argv[2], atol(argv[3]), atol(argv[4]), atoi(argv[5]),
                                                  atol(argv[6]), atol(argv[7]), atol(argv[8]), atol(argv[9]));
  }

}


