 /*
 
Simple File/Disk Forward Copy of variable Buffer Size
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

File traversal originally written here:
https://gist.github.com/xkikeg/4645373

*/
#define _FILE_OFFSET_BITS 64

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <assert.h>
#include <errno.h>
#include <time.h>

#define VERSION "v1.1"
#define VERSION_DATE "10/02/2023"

#ifndef SEEK_DATA
#warning "SEEK_DATA is undeclared and manually defined."
#define SEEK_DATA	3	/* seek to the next data */
#endif
#ifndef SEEK_HOLE
#warning "SEEK_HOLE is undeclared and manually defined."
#define SEEK_HOLE	4	/* seek to the next hole */
#endif

int forward_copy_with_datahole_method( 
       const char *from,  const char *to, 
       size_t from_offset, ssize_t to_offset, int retries,
       size_t buffer_size){                                     
       
          
  int fdi=-1, fdo=-1;
  int saved_errno = 0;
  char * buffer = NULL;

   printf("Copy from %s from offset %ld to offset %ld to destination %s using %ld bytes buffer with DATA/HOLE method:\n",  from, from_offset, to_offset, to, buffer_size);
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


   struct stat status;
   if(fstat(fdi, &status) < 0 ){
     saved_errno = errno;       
     printf("Error fstat File/Disk to check for file size.\n");
     goto out_error;
   }
   const off_t source_size =status.st_size; 
   printf("Source File/Disk size is: %ld bytes.\n", source_size);
  
  
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
   

  do {
  	
    usleep(2);
  	
    off_t beg=lseek(fdi, current_pos, SEEK_DATA);
    if(beg == -1) {
      perror("SEEK_DATA failed\n");
      exit(EXIT_FAILURE);
    }
    off_t end=lseek(fdi, beg, SEEK_HOLE);
    if(end == -1) {
      perror("SEEK_HOLE failed\n");
      exit(EXIT_FAILURE);
    }
    lseek(fdi, beg, SEEK_SET);
    /* mike Code from here */
    for( int read_retries =0; read_retries < retries ; read_retries++ ){
   
       ssize_t nread = read(fdi, buffer, buffer_size);
       if( nread < 0 )
       /* ERROR do retry read */
          continue;
       else if (nread == 0 ){
       /* EOF */                  
          break;
       }else{
       /* Read OK, Write the bytes */
          ssize_t nwritten = write(fdo, buffer, nread);
          if( nwritten < 0 ){ 
             saved_errno = errno;            
             printf("Could not write %ld bytes to %s. Aborting.\n",nwritten, to );
             goto out_error;
          }else          
             printf("\r%08ld Read %ld bytes, Written %ld bytes for %ld read_retries.",beg, nread, nwritten, read_retries);   
             break;
        }
    }//for retries
    
    
     /* mike Code stops here */
    fprintf(stderr, "0x%llx 0x%llx\n",
            (unsigned long long)beg,
            (unsigned long long)end);
    current_pos = end;
  } while (current_pos < to_offset);
  
  
  

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



