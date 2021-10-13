#ifndef USER_OPTIONS_H
# define USER_OPTIONS_H

#define FORWARD  'F'
#define BACKWARD 'B'
#define NO_IMAGE 0
#define WAIT_FOREVER -1

typedef struct UserOptions_t {
  int print_version_only;
  char * in_path;
  char * out_path;
  char * log_path;
  size_t buf_size;
  int read_retries;
  size_t start_offset;
  size_t end_offset;
  size_t image_chunk_size;
  int    read_timeout_sec; 
  int    log_level;
  int    log_stdout;
  char direction;
}
UserOptions_t;


int  user_options_parse_from_cli_arguments(UserOptions_t * user_options, int argc, char * argv[]);
void user_options_init(UserOptions_t * user_options);
void user_options_debug(UserOptions_t * user_options);
int  user_options_check(const UserOptions_t * user_options, char *msg);
void user_options_log( const UserOptions_t * user_options );
#endif
