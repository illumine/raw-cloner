 /*
 
Simple Fιle/Disk Forward Copy of variable Buffer Size

Copy input file/disk from offset  to offset to destination file/disk using  variable bytes buffer.
Program tries to read sections of disk that contain DATA rather than reading the entire drive or large sections of HOLES

It works with linux Kernel version 3.1 and above.
 
See lseek manual:

Seeking file data and holes
Since version 3.1, Linux supports the following additional values
for whence:
       SEEK_DATA
              Adjust the file offset to the next location in the file
              greater than or equal to offset containing data.  If
              offset points to data, then the file offset is set to
              offset.

       SEEK_HOLE
              Adjust the file offset to the next hole in the file
              greater than or equal to offset.  If offset points into
              the middle of a hole, then the file offset is set to
              offset.  If there is no hole past offset, then the file
              offset is adjusted to the end of the file (i.e., there is
              an implicit hole at the end of any file).

       In both of the above cases, lseek() fails if offset points past
       the end of the file.
*****************************
*/
#define _FILE_OFFSET_BITS 64
#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <assert.h>

#define VERSION "v1.0"
#define VERSION_DATE "27/01/2023"


int forward_copy_with_datahole_method( 
       const char *from,  const char *to, 
       size_t from_offset, ssize_t to_offset, int retries,
       size_t buffer_size){                                     
       
          
  int fdi=-1, fdo=-1;
  int saved_errno = 0;
  char * buffer = NULL;


    printf("Copy from %s from offset %ld to offset %ld to destination %s using %ld bytes buffer with DATA/HOLE method:\n",  from, from_offset, to_offset, to, buffer_size_initial);
    printf("It moves the file pointer to the next section where DATA are present.\n");   
    printf("It retries %d times to read the data sextion using a buffer of buffer_size.\n",retries);
    printf("It moves the file pointer to the next section where HOLE is present and skips it.\n");


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
    assert(source_size >= from_offset);

    ssize_t current_pos = lseek(fdi, from_offset, SEEK_SET);   
    if( current_pos >= 0 ){
        printf("Positioning source File/Disk to offset: %ld bytes.\n", from_offset);
    }else{
        saved_errno = errno;          
        printf("Error setting source to offset: %ld bytes.\n", from_offset);
        goto out_error;          
    }


   ssize_t target_pos;

   while(1){
      ssize_t nread, nwritten;
      int read_retries=0;

      usleep(2);
      
      
      current_pos = lseek(fdi, current_pos, SEEK_DATA);   
      if( current_pos >= 0 ){
         printf("Positioning source File/Disk to SEEK_DATA at offset: %ld bytes.\n", current_pos);
      }else{
         saved_errno = errno;          
         printf("Error setting source to SEEK_DATA.\n");
         goto out_error;          
      } 
      
      
      target_pos = lseek(fdi, current_pos+1, SEEK_HOLE);   
      if( current_pos >= 0 ){
         printf("Positioning source File/Disk to SEEK_HOLE at offset: %ld bytes.\n", target_pos);
      }else{
         saved_errno = errno;          
         printf("Error setting source to SEEK_HOLE.\n");
         goto out_error;          
      }      
      
      size_t bytes_read_count = 0, bytes_to_read = target_pos - current_pos;
      
      
      if( bytes_to_read < buffer_size )
         buffer_size = bytes_to_read;
      
      char eof = 'F';
      size_t times_read = 0;
      for( bytes_read_count = 0; bytes_read_count < bytes_to_read;  ){
      	
         memset(buffer,0,buffer_size);
         for( read_retries =0; read_retries < retries ; read_retries++ ){
            times_read++;     
            nread = read(fdi, buffer, buffer_size);
            if( nread < 0 )
            /* ERROR do retry read */
               continue;
            else if (nread == 0 ){
            /* EOF */                  
               eof = 'T';
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
                  
                bytes_read_count += nread; 
                break;
            }
         }//for retries
		
         if( eof == 'T' || read_retries == retries )
            break;
      }
    
      current_pos = target_pos + 1;
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
       printf("You need to specify\n%s input_disk output_disk from_offset to_offset retries buffer_size\n",argv[0]);
    printf("You can specify -1 for end of file in to_offset\n");
      return status;
  }else{
     
     return forward_copy_with_datahole_method( argv[1], argv[2], atol(argv[3]), atol(argv[4]), atoi(argv[5]), atol(argv[6]) );
  }

}



