#include <stdio.h>
#include <time.h>

#include "stats.h"
#include "logger.h"


void program_stats_init(ProgramStats_t * program_stats) {
	program_stats -> bytes_read= 0;
	program_stats -> bytes_written= 0;
	program_stats -> bytes_bad = 0;
	program_stats -> read_retries= 0;
	program_stats -> buffers_read= 0;
	program_stats -> buffers_written= 0;
	program_stats -> buffers_resized= 0;
	program_stats -> timeouts= 0;
	program_stats -> timeout_seconds= 0;
	program_stats -> time_start = time(NULL);
	program_stats -> time_stop = time(NULL);
	program_stats -> clock_start = clock();
	program_stats -> clock_stop = clock();
}


void program_stats_debug(ProgramStats_t * program_stats) {
  if (program_stats == NULL) {
    printf("program_stats is NULL!");
    return;
  }
  
  char timestamp_str[26];
  struct tm * tm_info;
    
  printf("\nBytes Read             : %ld",program_stats -> bytes_read);
  printf("\nBytes Written          : %ld",program_stats -> bytes_written);
  printf("\nBytes Bad              : %ld",program_stats -> bytes_bad);  
  printf("\nRead Retries           : %ld",program_stats -> read_retries);
  printf("\nBuffers Read           : %ld",program_stats -> buffers_read);
  printf("\nBuffers Written        : %ld",program_stats -> buffers_written);    
  printf("\nBuffers Resized        : %ld",program_stats -> buffers_resized);
  printf("\nTimeouts               : %ld",program_stats -> timeouts);
  printf("\nTimeouts total seconds : %ld",program_stats -> timeout_seconds);  
  
  tm_info = localtime( & program_stats -> time_start);
  strftime(timestamp_str, 26, "%Y-%m-%d %H:%M:%S", tm_info);
  printf("\nTime started           : %s",timestamp_str);   
  
  tm_info = localtime( & program_stats -> time_stop);
  strftime(timestamp_str, 26, "%Y-%m-%d %H:%M:%S", tm_info);
  printf("\nTime stopped           : %s",timestamp_str);   
   
  printf("\nWall time              : %lf", difftime( program_stats -> time_stop , program_stats -> time_start) ); 
  
  printf("\nCPU time               : %f",  (double)(program_stats ->clock_stop - program_stats ->clock_stop )/CLOCKS_PER_SEC ); 
    
  return;
}

void program_stats_log(ProgramStats_t * program_stats) {
  if (program_stats == NULL) {
    printf("program_stats is NULL!");
    return;
  }
  
  char timestamp_str[26]= {'\0'}, msg[BUFSIZ] = {'\0'};
  struct tm * tm_info;  
  
  sprintf(msg,"Bytes Read             : %ld",program_stats -> bytes_read);
  log_message( INFO, msg );
  sprintf(msg,"Bytes Written          : %ld",program_stats -> bytes_written);
  log_message( INFO, msg );
  sprintf(msg,"Bytes Bad              : %ld",program_stats -> bytes_bad);  
  log_message( INFO, msg );
  sprintf(msg,"Read Retries           : %ld",program_stats -> read_retries);
  log_message( INFO, msg );
  sprintf(msg,"Buffers Read           : %ld",program_stats -> buffers_read);
  log_message( INFO, msg );
  sprintf(msg,"Buffers Written        : %ld",program_stats -> buffers_written);    
  log_message( INFO, msg );
  sprintf(msg,"Buffers Resized        : %ld",program_stats -> buffers_resized);
  log_message( INFO, msg );
  sprintf(msg,"Timeouts               : %ld",program_stats -> timeouts);
  log_message( INFO, msg );
  sprintf(msg,"Timeouts total seconds : %ld",program_stats -> timeout_seconds);  
  
  tm_info = localtime( & program_stats -> time_start);
  strftime(timestamp_str, 26, "%Y-%m-%d %H:%M:%S", tm_info);
  sprintf(msg,"Time started           : %s",timestamp_str);   
  log_message( INFO, msg );
    
  tm_info = localtime( & program_stats -> time_stop);
  strftime(timestamp_str, 26, "%Y-%m-%d %H:%M:%S", tm_info);
  sprintf(msg,"Time stopped           : %s",timestamp_str);   
  log_message( INFO, msg );
     
  sprintf(msg,"Wall time              : %lf", difftime( program_stats -> time_stop , program_stats -> time_start) ); 
  log_message( INFO, msg );
    
  sprintf(msg,"CPU time               : %f",  (double)(program_stats ->clock_stop - program_stats ->clock_stop )/CLOCKS_PER_SEC ); 
  log_message( INFO, msg );   
  return;
}

