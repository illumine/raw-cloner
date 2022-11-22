/*
*****************************
Simple Copy of a file using a Static Buffer Size.
It brakes on first error

*****************************
*/

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>

#define VERSION "v1.0"
#define VERSION_DATE "16/11/2021"

pthread_mutex_t mutex_fdi = PTHREAD_MUTEX_INITIALIZER;
int fdi=-1, fdo=-1;
int state = -1;
time_t now, timestamp;

#define READ  1
#define WRITE 2
#define TIMEOUT 1

void *function_read( void * work);

int copy_forward( const char *from,  const char *to ){
  int rc1;
  pthread_t thread1;
   
  printf("Copy from %s to  %s using %d bytes buffer. It brakes on first read error!\n", from, to, BUFSIZ);

  fdi = open( from, O_RDONLY);
  if (fdi < 0) {
    perror("Error opening input for read.\n");
    goto out_error;
  } else {
    printf("Source %s opened for read.\n", from);
  }

  fdo = open(to, O_WRONLY | O_CREAT, 0600);
  if (fdo < 0) {
    perror("Error opening output for write.\n");
    goto out_error;
  } else {
    printf("Destination %s opened for writing.\n", to);
  }


  if( (rc1=pthread_create( &thread1, NULL, &function_read, NULL)) ){
      perror("Thread creation failed:\n");
  }

  
  while(1){
  
    now = time(NULL);	
  	if( state == READ && timestamp + TIMEOUT < now ){
  	   lseek(fdi,buf_size,SEEK_CUR);	
	}

  }




  pthread_join( thread1, NULL); 

  if (fdi >= 0)
      close(fdi);
  if (fdo >= 0)
    close(fdo);
  return 0;
out_error:
  if (fdi >= 0)
      close(fdi);
  if (fdo >= 0)
    close(fdo);
  return -1;
}


void *function_read( void * work){
   ssize_t nread = 0;
   char buffer[BUFSIZ] = {0};
   size_t buf_size = sizeof buffer;
   
   while(1){
   	  memset(buffer,0,buf_size);
   	  timestamp = time(NULL);
   	  nread = read(fdi, buffer, buf_size),
   	  if( nread == 0)
   	    break;
   	  else if( nread < 0){
   	  	perror("Could not read from file\n");
   	  	break;
	  }else{
	  	write(fdo, buffer, nread);
	  }
   }	
	
   pthread_mutex_lock( &mutex1 );
   counter++;
   printf("Counter value: %d\n",counter);
   pthread_mutex_unlock( &mutex1 );
}




int main(int argc, char * argv[]) {

  printf("%s %s %s\n",argv[0], VERSION, VERSION_DATE );
  if(argc<3){
        printf("You need to specify\n%s input_disk output_disk\n",argv[0]);
        return EXIT_FAILURE;
  }else{
        return copy_forward( argv[1], argv[2] );
  }

}

