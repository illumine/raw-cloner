#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "user-options.h"
#include "logger.h"

int user_options_parse_from_cli_arguments(UserOptions_t * user_options, int argc, char * argv[]) {
  int i, opt;
  opterr = 0;

  if (argc == 1) 
    return 1;

  while ((opt = getopt(argc, argv, "i:o:l:b:r:s:e:I:t:k:m:Bvun")) != -1) {
    switch (opt) {
    case 'i':
      user_options -> in_path = strdup(optarg);
      break;
    case 'o':
      user_options -> out_path = strdup(optarg);
      break;
    case 'l':
      user_options -> log_path = strdup(optarg);
      break;
    case 'b':
      user_options -> buf_size = atol(optarg);
      break;
    case 'r':
      user_options -> read_retries = atoi(optarg);
      break;
    case 's':
      user_options -> start_offset = atol(optarg);
      break;
    case 'e':
      user_options -> end_offset = atol(optarg);
      break;
    case 'I':
      user_options -> image_chunk_size = atol(optarg);
      break;
    case 't':
      user_options->read_timeout_sec = atoi(optarg);
      break;      
    case 'k':
      user_options->log_level = atoi(optarg);
      break;
    case 'm':
      user_options->stats_every_minutes = atoi(optarg);
      break;   
      /* switches */
    case 'B':
      user_options -> direction = BACKWARD;
      break;
    case 'v':
      user_options -> print_version_only = 1;
      return 0;
    case 'n':
      user_options -> stats_every_minutes = DONT_LOG_STATS;
      return 0;      
    case 'u':
      user_options -> log_stdout = 0;
      return 0;	        
    default:
      /* Unknown option: return error  */
      return 1;
    }
  }
  //debug_user_options(user_options);    

  for (i = optind; i < argc; i++)
    printf("\nNon-option argument %sn", argv[i]);

  return 0;
}

void user_options_init(UserOptions_t * user_options) {
  user_options -> print_version_only = 0;
  user_options -> in_path = NULL;
  user_options -> out_path = NULL;
  user_options -> log_path = NULL;
  user_options -> buf_size = BUFSIZ;
  user_options -> read_retries = 1;
  user_options -> start_offset = 0;
  user_options -> end_offset = -1;
  user_options -> image_chunk_size = NO_IMAGE;
  user_options -> direction = FORWARD;
  user_options -> read_timeout_sec = WAIT_FOREVER;
  user_options -> log_level = 0;    /* set default log level to DEBUG */
  user_options -> log_stdout = 1;   /* Log also to std out */
  user_options -> stats_every_minutes = 5;
}

void user_options_debug(UserOptions_t * user_options) {
  if (user_options == NULL) {
    printf("user_options is NULL!");
    return;
  }
  printf("\nInput path: %s", user_options -> in_path);
  printf("\nOutput path: %s", user_options -> out_path);
  printf("\nLogfile path: %s", user_options -> log_path);
  printf("\nRead Buffer size: %ld", user_options -> buf_size);
  printf("\nRead Retries Attemps: %d", user_options -> read_retries);
  printf("\nStart Offset: %ld", user_options -> start_offset);
  printf("\nEnd Offset: %ld", user_options -> end_offset);
  printf("\nImage Chunk Size: %ld", user_options -> image_chunk_size);
  printf("\nRead Direction: %c", user_options -> direction);
  printf("\nRead timeout: %d", user_options -> read_timeout_sec);
  printf("\nLog Level: %d", user_options -> log_level);
  printf("\nLog also to stdout: %d", user_options -> log_stdout);
  printf("\nLog stats every X minutes: %d", user_options -> stats_every_minutes);
  printf("\n");

  return;
}


int  user_options_check( const UserOptions_t * user_options, char *msg ){
  int status = 1;
   if( user_options->start_offset < 0 )
       sprintf(msg, "\nStart offset cannot be %ld\n", user_options->start_offset);
   else if( user_options->end_offset < -1 )
       sprintf(msg, "\nEnd offset cannot be %ld\n", user_options->end_offset);
   else if( user_options->end_offset > 0 &&  user_options->end_offset < user_options->start_offset )
       sprintf(msg, "\nEnd offset %ld cannot be less than start offset %ld\n", user_options->start_offset, user_options->end_offset);
   else
       status = 0;
  
  
   return status;
}

void user_options_log( const UserOptions_t * user_options ){

  char msg[BUFSIZ] = {'\0'};

   	  log_message( INFO, "User Options:" );
      sprintf(msg,"Input path: %s", user_options -> in_path);
      log_message( INFO, msg );
      sprintf(msg,"Output path: %s", user_options -> out_path);
      log_message( INFO, msg );
      sprintf(msg,"Logfile path: %s", user_options -> log_path);
      log_message( INFO, msg );
      sprintf(msg,"Read Buffer size: %ld", user_options -> buf_size);
      log_message( INFO, msg );
      sprintf(msg,"Read Retries Attemps: %d", user_options -> read_retries);
      log_message( INFO, msg );
      sprintf(msg,"Start Offset: %ld", user_options -> start_offset);
      log_message( INFO, msg );
      sprintf(msg,"End Offset: %ld", user_options -> end_offset);
      log_message( INFO, msg );
      sprintf(msg,"Image Chunk Size: %ld", user_options -> image_chunk_size);
      log_message( INFO, msg );
      sprintf(msg,"Read Direction: %c", user_options -> direction);
      log_message( INFO, msg );
      sprintf(msg,"Read timeout: %d", user_options -> read_timeout_sec);
      log_message( INFO, msg );
      sprintf(msg,"Log Level: %d", user_options -> log_level);
      log_message( INFO, msg );      
      sprintf(msg,"Log also to stdout: %d", user_options -> log_stdout);    
      log_message( INFO, msg );	    
      sprintf(msg,"Log stats every X minutes: %d", user_options -> stats_every_minutes); 
      log_message( INFO, msg );	       
      log_message( INFO, "User Options finished." );
}
