/*
*****************************
Simple Copy of Static Buffer Size

*****************************
*/


#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <time.h>
#define EX_ERROR 1
#define MAX_READ_RETRIES 3
#define ONE_BYTE 1


/* Sleep for some miliseconds */
int msleep(long msec){
	
    struct timespec ts;
    int res;

    if (msec < 0){
        errno = EINVAL;
        return -1;
    }

    ts.tv_sec = msec / 1000;
    ts.tv_nsec = (msec % 1000) * 1000000;

    do {
        res = nanosleep(&ts, &ts);
    } while (res && errno == EINTR);

    return res;
}


int cp( const char *from,  const char *to ){
  int fdi, fdo;
  char buf[BUFSIZ ] = {'\0'};
  ssize_t nread, source_size, times_read;
  int saved_errno;

    printf("Copy from %s to  %s using %d bytes buffer.\n", from, to, BUFSIZ);

    fdi = open(from, O_RDONLY);
    if (fdi < 0){
       printf("Error opening %s file to read.\n", from);
       goto out_error;
    }
        
    fdo = open(to, O_WRONLY | O_CREAT);
    if (fdo < 0){
       printf("Error opening %s file to write.\n", to);
       goto out_error;
    }
	
    /* Get the source file/disk size. Seek to fdi END of the file/disk */
    source_size = lseek(fdi, 0, SEEK_END);
    if( source_size < 0){
       printf("Error seeking source to SEEK_END.\n");
       goto out_error;
    }else{
       printf("Source File/Disk size is: %ld bytes.\n", source_size);
    }
    /* Repositioning the file to the begining */
    lseek(fdi, -source_size, SEEK_CUR);
 
    /* Reading the file loop */
    times_read=0;
    while(1){
      ssize_t nwritten, current_pos, buf_siz = sizeof buf;
      int read_retries=0;

    read_form_disk:
      /* Give some time not to overheat */
	  msleep(3);
	  		
      current_pos = lseek(fdi, 0, SEEK_CUR);  
      if( current_pos < 0 ){
         printf("Cannot lseek the input file. lseek returned %ld. Aborting.\n",current_pos);
         goto out_error;
      }
      
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
               printf("Could not write %ld bytes to %s. Aborting.\n",nwritten, to );
	           goto out_error;
            }else	       
	           printf("\r%08x Read %ld bytes, Written %ld bytes for %ld times.",current_pos, nread, nwritten, times_read);
            break;
       }
       
     }
     
     /* Problem on reading: we could not read after MAX_READ_RETRIES: Cut buffer siz in the middle and try again */
     if( read_retries == MAX_READ_RETRIES && buf_siz > ONE_BYTE ){
        printf("Read Retries %d completed for a buffer of %ld bytes failed. Making new buffer size %ld bytes and try to read again.\n", read_retries, buf_siz, (ssize_t)(buf_siz/2) );
	    buf_siz = (ssize_t)(buf_siz/2);
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
        printf("Cannot lseek the input file. lseek returned %ld. Aborting.\n",current_pos);
        goto out_error;
     }

     if( current_pos == source_size ){
        printf("Reached end of file at %ld bytes.\n",current_pos);     
        break;
     }

   }//while


   if (close(fdo) < 0){
       fdo = -1;
       goto out_error;
   }
   close(fdi);

   /* Success! */
   return 0;

  out_error:
    saved_errno = errno;

    if (fdi >= 0)
        close(fdi);
    if (fdo >= 0)
        close(fdo);

    errno = saved_errno;
    printf("Errno %d reported: %s\n",errno,strerror(errno));
    return saved_errno;
}

int main(int argc, char * argv[]) {
  int status = EX_ERROR;
  
  if(argc<3){
  	printf("You need to specify\n%s input_disk output_disk\n",argv[0]);
  	return status;
  }else{
  	return cp( argv[1], argv[2] );
  }

}
