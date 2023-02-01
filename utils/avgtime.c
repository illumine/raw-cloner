 /*
*****************************
Calculates average disk read time
*****************************
*/

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <assert.h>
#include <fcntl.h>
#include <sys/time.h>
#include <unistd.h>

#define VERSION "v1.0"
#define VERSION_DATE "01/02/2023"

/* We define this as a macro since we need to have it inline for making calculation faster from a function*/
#define MICRO_TDIFF(tstart,tend)  (((1000000 )*(tend.tv_sec) + (tend.tv_usec)) - ((1000000 )*(tstart.tv_sec) + (tstart.tv_usec)))


int get_average_read_time( const char *from,  size_t buffer_size,  size_t segment_size){
  int fdi=-1;
  char * buffer = NULL;
  int saved_errno=0; 

  
    printf("Calculate average read time from %s \n", from);
    printf("Using read buffer of %ld bytes and skip/segment bytes %ld.\n",   buffer_size, segment_size);

    fdi = open(from, O_RDONLY);
    if (fdi < 0){
       saved_errno = errno;      	
       printf("Error opening %s source to read.\n", from);
       goto out_error;
    }

    /* Get the source file/disk size in bytes */
    ssize_t source_size = lseek(fdi, 0, SEEK_END);
    if( source_size < 0){
       saved_errno = errno;  	
       printf("Error seeking source to SEEK_END.\n");
       goto out_error;
    }else{
       printf("Source File/Disk size is: %ld bytes.\n", source_size);
    }

    /* Check if user input is valid */
    assert( buffer_size > 1 );
    assert( segment_size > 1 );
    assert( buffer_size < segment_size);
    assert( source_size > buffer_size); 
    assert( source_size > segment_size); 
    assert( source_size > buffer_size +  segment_size);
	
	  
    /* Initial Buffer Allocation according to user defined params
	   The UNIX 98 standard requires malloc(), calloc(), and realloc() to set errno to ENOMEM upon failure.
	*/
    if ((buffer = (char * ) calloc(buffer_size, sizeof(char))) == NULL) {
       saved_errno = errno;     	
       printf("Error allocating buffer of size %ld. Consider using a smaller buffer size.\n", buffer_size);
       goto out_error;
    }
    /* set FD to the begining of the file again */
    lseek(fdi, 0, SEEK_SET);


    size_t failed_reads=0;
    ssize_t current_pos=0;
    size_t sample_size = (size_t)(source_size / (buffer_size +  segment_size));
    struct timeval  tstart, tend;
    time_t total_time_read = 0;

    
    printf("Sample size - reads is %.0ld\n",sample_size);	
    for(size_t i=0; i<sample_size; i++){
       ssize_t nread;
	   	
       gettimeofday(&tstart,NULL); 
       nread = read(fdi,buffer,buffer_size);
       if(nread < 0 ){
       	   failed_reads++;
       }
       gettimeofday(&tend,NULL); 
       total_time_read += MICRO_TDIFF( tstart, tend);
       //printf("Read time %ld microsecs\n",MICRO_TDIFF( tstart, tend));
        
	   /* Seek SET file/disk current +segment_size*/
       current_pos = lseek(fdi, segment_size, SEEK_CUR);
       if( current_pos < 0){
  	      saved_errno = errno; 
          printf("Error settting current file +segment_size.\n");
          goto out_error;
       }else{
          //printf("Current offset is set to %ld bytes.\n", current_pos);
       }
  
    }

    double  avg_read_time =  (double) total_time_read/ (double)sample_size ;
    printf("Completed. Average per read time is %f microseconds for a sample of %ld reads\n", avg_read_time, sample_size);


    if (fdi >= 0)
      close(fdi);
    if(buffer)
      free(buffer);

    /* Success! */
    return 0;



  out_error: 	

    if (fdi >= 0)
        close(fdi);
    if(buffer)
      free(buffer);
      
    errno = saved_errno;

    return saved_errno;
}

int main(int argc, char * argv[]) {
  int status = EXIT_FAILURE;

  printf("%s %s %s\n",argv[0], VERSION, VERSION_DATE );
  if(argc<3){
        printf("You need to specify\n%s input_disk read_buffer_bytes skip_bytes\n",argv[0]);
        return status;
  }else{
        return get_average_read_time( argv[1], atol(argv[2]), atol(argv[3]) );
  }

}


