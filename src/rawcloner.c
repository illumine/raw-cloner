#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>
#include <stdint.h>

#include "logger.h"
#include "user-options.h"
#include "raw-io.h"
#include "util.h"
#include "stats.h"

#define ONE_BYTE 1
#define SOURCE 0
#define DESTINATION 1
#define EX_ERROR 1
#define EX_OK 0
#define VERSION "v1.0"
#define BUILD_DATE "13/10/2021"

UserOptions_t UserOptions;

ProgramStats_t ProgramStats;

void print_version(void) {
  printf("\nVersion %s  built on %s\n", VERSION, BUILD_DATE);
}

void usage(char * progname) {
  printf("\nUsage:");
  printf("\n%s -i <input path> -o <output path> -l <log path> -b <buffer size> -r <read retries> -s <start offset> -e <end offset> -I <image size> -t <read timeout secs> -k <log level> -Buv", progname);
  printf("\n-v version info");
  printf("\n-i <input path> source path. Can be /dev/sda or a normal file name like /home/mike/myfile.txt.");
  printf("\n-o <output path> destination path name. Can be /dev/sda or a normal file name.");
  printf("\n-l <log path> destination path name."); 
  printf("\n-b <buffer size> Read and Write buffer size in bytes."); 
  printf("\n-r <read retries> How many times to retry a read attempt. Keep it low...");   
  printf("\n-s <start offset> The source start offset in bytes.");    
  printf("\n-e <end offset> The source end offset in bytes.");   
  printf("\n-I <image chunk size> Image Chunk Size in bytes. For best performance this must be a multiplier of Buffer size.");     
  printf("\n-t <read timeout secs> if read is blocked, how meny seconds to wait before we timeout the read attempt.");  
  printf("\n-B read the disk/file backwards - from end to start BYTE BY BYTE - slow but reliable.");
  printf("\n-k <log level> Set the log level. Values 0:DEBUG,1:TRACE,2:INFO,3:ERROR,4:FATAL,5:NOTHING .");
  printf("\n-u Set the log to be sent to stdout.");
  printf("\nReturns 0 on OK , 1 on Error.");
  printf("\nAuthor: Michael Mountrakis 2021 - mike.mountrakis@gmail.com");  
  print_version();
  printf("\n");
}

void mark_bad_byte(int fdi) {
  char msg[BUFSIZ] = {'\0'};	
  off_t offset = lseek(fdi, 0, SEEK_CUR);
  sprintf(msg,"BB%ld,", offset);
  log_message( INFO, msg);
}

#ifdef __ultra_fast__
size_t read_disk(int fdi, void * buffer, size_t buf_size, char * msg) {
	return read(fdi, buffer, buf_size);
}
#endif

#ifdef __linux__
size_t read_disk(int fdi, void * buffer, size_t buf_size, char * msg) {
  fd_set rfds;
  struct timeval tv;
  size_t bytes_read = 0;
  int retval = 0, read_retries = 0;

  if (UserOptions.read_timeout_sec == WAIT_FOREVER) {
    /* this will block forever if no data, or return with data or error*/
    for (read_retries = 0; read_retries < UserOptions.read_retries; read_retries++) {
      bytes_read = safe_read(fdi, buffer, buf_size);
      if (bytes_read < 0){
      	ProgramStats.read_retries++;
      	continue;
	  }
      else
        break; 
    }
    
  } else {
    FD_ZERO( & rfds);
    FD_SET(0, & rfds);
    tv.tv_sec = UserOptions.read_timeout_sec;
    tv.tv_usec = 0;

    /* Poll descriptor for input buffer*/
    retval = select(1, & rfds, NULL, NULL, & tv);
    if (retval == -1) {
      sprintf(msg, "select() returned error on input file descriptor.");
      perror(msg);
      log_message( ERROR ,msg );
      bytes_read = -1;
    } else if (retval) {
      /*Data is available now. Read it with safe_read */
      for (read_retries = 0; read_retries < UserOptions.read_retries; read_retries++) {
        bytes_read = safe_read(fdi, buffer, buf_size);
        if (bytes_read < 0){
          ProgramStats.read_retries++;
		  continue;        	
		}
        else
          break;
      }
    } else
      /* No data within timeout */
      ProgramStats.timeout_seconds += UserOptions.read_timeout_sec;
      ProgramStats.timeouts++;
      bytes_read = -1;
  }

  return bytes_read;
}	
#else
size_t read_disk(int fdi, void * buffer, size_t buf_size, char * msg) {
  size_t bytes_read = 0;
  int read_retries = 0;

  if (UserOptions.read_timeout_sec == WAIT_FOREVER)
    bytes_read = safe_read(fdi, buffer, buf_size);
  else {
    for (read_retries = 0; read_retries < UserOptions.read_retries; read_retries++) {
      bytes_read = safe_read(fdi, buffer, buf_size);
      if (bytes_read < 0) {
        sleep(UserOptions.read_timeout_sec);
        ProgramStats.read_retries++;
        ProgramStats.timeout_seconds += UserOptions.read_timeout_sec;
        ProgramStats.timeouts++;        
        continue;
      }else
        break;
    }
  }
  return bytes_read;
}


#endif


void program_exit( int exit_status ){
  char msg[BUFSIZ] = {'\0'};	
  sprintf(msg, "Program exits with status %d.", exit_status );
  log_message( INFO ,msg );
  log_close();
  exit(exit_status);	
}


/* Returns 0 on success */
int  copy_forward(void) {

  int fdi = -1, fdo = -1; 
  char msg[BUFSIZ] = {'\0'};
  size_t bytes_read = 0, buf_size = 0, bytes_writen = 0, curr_offset = 0;
  char * buffer = NULL;
  int  status = EX_ERROR;
  
  log_message( INFO, "copy_forward() started." );
  
  fdi = open(UserOptions.in_path, O_RDONLY);
  if (fdi < 0) {
    sprintf(msg, "Error opening %s for read.", UserOptions.in_path);
    goto EXIT;
  } else {
    sprintf(msg,"Source %s opened for read. File descriptor %d.", UserOptions.in_path, fdi);
    log_message( INFO, msg);
  }

  fdo = open(UserOptions.out_path, O_WRONLY | O_CREAT, 0644);
  if (fdo < 0) {
    sprintf(msg, "Error opening file %s for write.", UserOptions.out_path);
    goto EXIT;
  } else {
    sprintf(msg,"Destination %s opened for writing. File descriptor %d.", UserOptions.out_path, fdo);
    log_message( INFO, msg);
  }

  /* Initial Buffer Allocation according to user defined params*/
  if ((buffer = (char * ) calloc(UserOptions.buf_size, sizeof(char))) == NULL) {
    sprintf(msg, "Error allocating buffer of size %ld. Consider using a smaller buffer size.", UserOptions.buf_size);
    goto EXIT;
  }else{
  	memset (buffer, '\0', UserOptions.buf_size);
  	sprintf(msg, "Buffer of size %ld alocated at %lX and initialized.", UserOptions.buf_size, (uintptr_t)buffer);
    log_message( INFO, msg);
  }

  if (UserOptions.start_offset > 0) {
    if ((lseek(fdi, UserOptions.start_offset, SEEK_SET)) < 0) {
      sprintf(msg, "Error seeking start offset %ld in file  %s.", UserOptions.start_offset, UserOptions.in_path);
      goto EXIT;
    }
  }else{
  	sprintf(msg, "Source seeked at %ld.", UserOptions.start_offset);
    log_message( INFO, msg);  	
  }


  while (1) {
    buf_size = UserOptions.buf_size;
    bytes_read = 0;

    if (UserOptions.end_offset > 0)
      curr_offset = lseek(fdi, 0, SEEK_CUR);
    if (UserOptions.end_offset < curr_offset + buf_size)
      buf_size = UserOptions.end_offset - curr_offset;

    READ:
    /* try to read the buffer */
    bytes_read = read_disk(fdi, buffer, buf_size, msg);

    if (bytes_read < 0) {
      buf_size = (size_t)(1 + buf_size / 2);
      ProgramStats.buffers_resized++;
      
      if (buf_size == 1) {
         mark_bad_byte(fdi);
         ProgramStats.bytes_bad++;
         lseek(fdi, ONE_BYTE, SEEK_CUR);
         buf_size = UserOptions.buf_size;
         goto READ;      
      }
    } else if (bytes_read == 0)
      /* EOF */
      break;
    else {
      bytes_writen = full_write(fdo, buffer, bytes_read);
      if (bytes_writen < 0) {
        sprintf(msg, "Failed write %ld to %s.", bytes_read, UserOptions.out_path);
        goto EXIT;
      }
      
      ProgramStats.bytes_read += bytes_read;
      ProgramStats.bytes_written += bytes_writen;
      ProgramStats.buffers_read++;
      ProgramStats.buffers_written++ ;
    }

  }
  
  status = EX_OK;

/* post processing steps */
EXIT:
  if( status == EX_ERROR){
    log_message( ERROR ,msg );
    perror(msg);  	
  }
  
  if(buffer)
     free(buffer);
  if(fdi > 0 )
     safe_close(fdi);
  if( fdo > 0 )
     safe_close(fdo);  
     
  sprintf(msg,"copy_forward() ended with status %d.", status);
  log_message( INFO, msg );

  return  status;
}


int  copy_backwards( void ){

  int fdi = -1, fdo = -1;
  char msg[BUFSIZ] = {'\0'};
  size_t source_size=0, curr_offset=0, bytes_to_read=0;
  int i =0, nr =0,nw=0;
  char c;
  char * buffer = NULL;
  int  status = EX_ERROR;

  log_message( INFO, "copy_backwards() started." );
  fdi = open(UserOptions.in_path, O_RDONLY);
  if (fdi < 0) {
    sprintf(msg, "Error opening %s for read.", UserOptions.in_path);
    goto EXIT;
  } else {
    sprintf(msg,"Source %s opened for read. File descriptor %d.", UserOptions.in_path, fdi);
    log_message( INFO, msg);
  }

  fdo = open(UserOptions.out_path, O_WRONLY | O_CREAT, 0644);
  if (fdo < 0) {
    sprintf(msg, "Error opening file %s for write.", UserOptions.out_path);
    goto EXIT;
  } else {
    sprintf(msg,"Destination %s opened for writing. File descriptor %d.", UserOptions.out_path, fdo);
    log_message( INFO, msg);
  }

  /* Initial Buffer Allocation according to user defined params*/
  if ((buffer = (char * ) calloc(UserOptions.buf_size, sizeof(char))) == NULL) {
    sprintf(msg, "Error allocating buffer of size %ld. Consider using a smaller buffer size.", UserOptions.buf_size);
    goto EXIT;
  }else{
  	memset (buffer, '\0', UserOptions.buf_size);
  	sprintf(msg, "Buffer of size %ld alocated at %lX and initialized.", UserOptions.buf_size, (uintptr_t)buffer);
    log_message( INFO, msg);
  }


  /* Get the source file/disk size. Seek to fdi END of the file/disk */
  source_size = lseek(fdi, 0, SEEK_END);
  if( source_size < 0){
    sprintf(msg, "Error seeking source to SEEK_END.");
    goto EXIT;
  }else{
  	sprintf(msg,"Source File/Disk size is: %ld bytes.", source_size);
  	log_message( INFO, msg);
  }

  curr_offset = lseek(fdo, source_size, SEEK_SET);
  if( curr_offset < 0){
    sprintf(msg, "Error seeking destination to %ld.", source_size);
    goto EXIT;
  }else{
  	sprintf(msg,"Destination File/Disk offset is set to: %ld bytes.", curr_offset);
  	log_message( INFO, msg);
  }
  
  curr_offset = lseek(fdo, 0, SEEK_CUR);
  sprintf(msg,"Destination File/Disk offset currently: %ld bytes.", curr_offset);
  log_message( INFO, msg);
  
  curr_offset = 0;
  while (curr_offset < source_size) {

    if( curr_offset + UserOptions.buf_size > source_size )
       bytes_to_read = source_size - curr_offset;
    else
       bytes_to_read = UserOptions.buf_size;

    /* Read source and reposition after read*/
    lseek(fdi, -bytes_to_read, SEEK_CUR);
    for(i=0; i< bytes_to_read; i++){
       nr = read( fdi,&c,ONE_BYTE);
       if( nr > 0 ){
          buffer[i] = c;
          ProgramStats.bytes_read++;
	   }else if( nr > 0 ){
	   	  ProgramStats.bytes_bad++;
	   	  mark_bad_byte( fdi);
	   }else
	      continue;
    }
    lseek(fdi, -bytes_to_read, SEEK_CUR);

    /* Write Destination and reposition after write */
    lseek(fdo, -bytes_to_read, SEEK_CUR);
    nw = write(fdo, buffer, bytes_to_read);
    if( nw < 0){
        sprintf(msg, "Failed write %ld to %s.", bytes_to_read, UserOptions.out_path);
        goto EXIT;    	
	}else{
		ProgramStats.bytes_written += bytes_to_read;
	}
    lseek(fdo, -bytes_to_read, SEEK_CUR);
    
    /* Re-initialize buffer and increment offset */
    memset (buffer, '\0', bytes_to_read);
    curr_offset+=bytes_to_read;

  } 

  status = EX_OK;

/* post processing steps */
EXIT:
  if( status == EX_ERROR){
    log_message( ERROR ,msg );
    perror(msg);  	
  }
  
  if(buffer)
     free(buffer);
  if(fdi > 0 )
     safe_close(fdi);
  if( fdo > 0 )
     safe_close(fdo);  
     
  sprintf(msg,"copy_backwards() ended with status %d.", status);
  log_message( INFO, msg );

  return  status;

}


int  copy_to_image( void ) {

  int fdi = -1, fdo = -1, fdl = -1;
  char msg[BUFSIZ]= {'\0'};
  size_t bytes_read = 0, buf_size = 0, bytes_writen = 0, curr_offset = 0;
  char * buffer = NULL;

  size_t current_image_chunk_size = 0;
  int current_image_chunk_n = 1;
  char current_image_chunk_name[BUFSIZ] = {'\0'};
  int  status = EX_ERROR;

  log_message( INFO, "copy_to_image() started." );
  fdi = open(UserOptions.in_path, O_RDONLY);
  if (fdi < 0) {
    sprintf(msg, "Error opening %s for read.", UserOptions.in_path);
    goto EXIT;
  } else {
    sprintf(msg,"Source %s opened for read. File descriptor %d.", UserOptions.in_path, fdi);
    log_message( INFO, msg);
  }

  fdo = open(UserOptions.out_path, O_WRONLY | O_CREAT, 0644);
  if (fdo < 0) {
    sprintf(msg, "Error opening file %s for write.", UserOptions.out_path);
    goto EXIT;
  } else {
    sprintf(msg,"Destination %s opened for writing. File descriptor %d.", UserOptions.out_path, fdo);
    log_message( INFO, msg);
  }

  /* Initial Buffer Allocation according to user defined params*/
  if ((buffer = (char * ) calloc(UserOptions.buf_size, sizeof(char))) == NULL) {
    sprintf(msg, "Error allocating buffer of size %ld. Consider using a smaller buffer size.", UserOptions.buf_size);
    goto EXIT;
  }else{
  	memset (buffer, '\0', UserOptions.buf_size);
  	sprintf(msg, "Buffer of size %ld alocated at %lX and initialized.", UserOptions.buf_size, (uintptr_t)buffer);
    log_message( INFO, msg);
  }


  if (UserOptions.start_offset > 0) {
    if ((lseek(fdi, UserOptions.start_offset, SEEK_SET)) < 0) {
      sprintf(msg, "Error seeking start offset %ld in file  %s.", UserOptions.start_offset, UserOptions.in_path);
      goto EXIT;
    }
  }

  current_image_chunk_size = 0;
  current_image_chunk_n = 1;
  current_image_chunk_name[0] = '\0';
  while (1) {
    buf_size = UserOptions.buf_size;
    bytes_read = 0;

    sprintf(current_image_chunk_name, "%s%05d.raw", UserOptions.out_path, current_image_chunk_n);
    fdo = open(current_image_chunk_name, O_WRONLY | O_CREAT, 0644);
    if (fdo < 0) {
      sprintf(msg, "Error opening file %s for write.", current_image_chunk_name);
      goto EXIT;
    } else {
      sprintf(msg, "Destination %s opened for writing. File descriptor is %d.", current_image_chunk_name, fdo);
      log_message( INFO, msg);
    }

    while (current_image_chunk_size + UserOptions.buf_size <= UserOptions.image_chunk_size) {

      if (UserOptions.end_offset > 0)
        curr_offset = lseek(fdi, 0, SEEK_CUR);
      if (UserOptions.end_offset < curr_offset + buf_size)
        buf_size = UserOptions.end_offset - curr_offset;
        ProgramStats.buffers_resized++;
      

      READ:
        /* try to read the buffer */
      bytes_read = read_disk(fdi, buffer, buf_size, msg);
      if (bytes_read < 0) {
        buf_size = (size_t)(1 + buf_size / 2);
        ProgramStats.buffers_resized++;
        
        if (buf_size == 1) {        	
          mark_bad_byte(fdi);
          ProgramStats.bytes_bad++;
          lseek(fdi, ONE_BYTE, SEEK_CUR);
          buf_size = UserOptions.buf_size;
          goto READ;         
        }
      } else if (bytes_read == 0)
        /* EOF */
        break;
      else {
        bytes_writen = full_write(fdo, buffer, bytes_read);
        if (bytes_writen < 0) {
          sprintf(msg, "Failed write %ld to %s.", bytes_read, current_image_chunk_name);
          goto EXIT;
        }
        ProgramStats.bytes_read += bytes_read;
        ProgramStats.bytes_written += bytes_writen;
        ProgramStats.buffers_read++;
        ProgramStats.buffers_written++ ;        
        
        current_image_chunk_size += bytes_writen;
        sprintf(msg, "current_image_chunk_size %07ld bytes_writen %07ld UserOptions.image_chunk_size %07ld", current_image_chunk_size, bytes_writen, UserOptions.image_chunk_size);
        log_message( DEBUG, msg);
      }

    } //image loop

    safe_close(fdo);
    current_image_chunk_size = 0;
    current_image_chunk_n++;
    current_image_chunk_name[0] = '\0';

    if (bytes_read == 0)
      break;

  }

/* post processing steps */
EXIT:
  if( status == EX_ERROR){
    log_message( ERROR ,msg );
    perror(msg);  	
  }
  
  if(buffer)
     free(buffer);
  if(fdi > 0 )
     safe_close(fdi);
  if( fdo > 0 )
     safe_close(fdo);  
     
  sprintf(msg,"copy_to_image() ended with status %d.", status);
  log_message( INFO, msg );

  return  status;

}



int main(int argc, char * argv[]) {
  char msg[BUFSIZ] = {'\0'};
  int status = EX_ERROR;

  user_options_init( & UserOptions);
  if (user_options_parse_from_cli_arguments( & UserOptions, argc, argv)) {
    fprintf(stderr, "\nError while parsing arguments!\n");
    usage(argv[0]);
    exit(EX_ERROR);
  }
  user_options_debug( & UserOptions);
  if (user_options_check( & UserOptions, msg)) {
    fprintf(stderr, "%s", msg);
    exit(EX_ERROR);
  }

  if (UserOptions.print_version_only) {
    usage(argv[0]);
    exit(EX_OK);
  }


  if( log_open( UserOptions.log_path) < 0){
      sprintf(msg, "\nError opening log file %s for write.\n", UserOptions.log_path);
      perror(msg);
      exit(EX_ERROR);	
  }
  log_set_stdout();
  log_set_loglevel(DEBUG);
  user_options_log( &UserOptions);
  
  
  program_stats_init( &ProgramStats);
  
  

  if(UserOptions.direction == FORWARD){
    if (UserOptions.image_chunk_size == NO_IMAGE)
      status = copy_forward();
    else
      status = copy_to_image();
  }else if(UserOptions.direction == BACKWARD){
  	status = copy_backwards();
  }
  
  program_stats_log( &ProgramStats);
  
  program_exit(status);
}
