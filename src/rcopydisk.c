 /*
*****************************
Simple Reverse Copy using a Static Buffer Size

*****************************
*/

#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>

#define EX_ERROR 1
#define MAX_READ_RETRIES 3

int rcp( const char *from,  const char *to ){
  int fdi, fdo;
  ssize_t source_size, current_pos;
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


  /* Get the source file/disk size in bytes */
  source_size = lseek(fdi, 0, SEEK_END);
  if( source_size < 0){
    printf("Error seeking source to SEEK_END.\n");
    goto out_error;
  }else{
    printf("Source File/Disk size is: %ld bytes.\n", source_size);
  }

  /* Seek SET  source file/disk to the END*/
  current_pos = lseek(fdi, source_size, SEEK_SET);
  if( current_pos < 0){
    printf("Error seting source to %ld.\n", source_size);
    goto out_error;
  }else{
    printf("Source File/Disk offset is set to: %ld bytes.\n", current_pos);
  }


    /* Reading the file/disk loop in reverese */

  while (1) {
     char buf[BUFSIZ ] = {'\0'};
     ssize_t  buf_siz = sizeof buf, bytes_to_read, nread, nwritten;
     int read_retries =0;
     
     if( buf_siz >= source_size )
       bytes_to_read = source_size;
     else if( buf_siz > source_size - current_pos )
       bytes_to_read = source_size - current_pos;
     else 
       bytes_to_read = buf_siz;
     printf("Bytes to read %ld.\n",bytes_to_read);   
     
     current_pos = lseek(fdi, -bytes_to_read, SEEK_CUR);
     if( current_pos < 0){
       printf("Error seting source File/Disk to %ld.\n", source_size);
       goto out_error;
     }else{
       printf("Source File/Disk offset is set to: %ld bytes.\n", current_pos);
     }     

     for( read_retries =0; read_retries < MAX_READ_RETRIES; read_retries++ ){   
         nread = read(fdi, buf, buf_siz);
         if( nread < 0 )
	    /* ERROR do retry read */
            continue;
         else if (nread == 0 )
	    /* EOF */	
            break;
        else{
	    /* Read OK, Write the butes */
            //nwritten = write(fdo, buf, nread);
            //write buffer
            printf("%s",buf);
            break;
    	}
	}


     current_pos = lseek(fdi, 0, SEEK_CUR);
     if( current_pos < 0){
       printf("Error getting the current offset from source File/Disk.\n");
       goto out_error;
     }else{
       printf("Current source File/Disk offset %ld bytes.\n", current_pos);
     }  

     if( current_pos == source_size ){
        printf("Reached end of file at %ld bytes.\n",current_pos);     
        break;
     }

  }//while


  return 0;

  out_error:
    saved_errno = errno;

    if (fdi >= 0)
        close(fdi);
    if (fdo >= 0)
        close(fdo);

    errno = saved_errno;
    return saved_errno;
}

int main(int argc, char * argv[]) {
  int status = EX_ERROR;

  if(argc<3){
        printf("You need to specify\n%s input_disk output_disk\n",argv[0]);
        return status;
  }else{
        return rcp( argv[1], argv[2] );
  }

}

