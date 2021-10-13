#ifndef LOGGER_H
# define LOGGER_H

#include <stdio.h>


typedef enum{
	DEBUG = 0,
	TRACE,
	INFO,
	ERROR,
	FATAL,
	NOTHING
} LogMessage_t;

typedef enum{
	SUCCESS = 0,  /* everything ok, log was printed */
	E_STREAM,     /* problem with the log stream*/	
	E_NO_LOG     /* log was not printed since log level is higher*/
} LogError_t;

/* The Stram */
FILE *       log_fp;
/* If 1 then log to stdout also */
int          log_stdout;
/* log messages only above this log_level */
LogMessage_t log_level;

/* returns -1 on error, or the filenumber of the log stream */
int          log_open   ( const char * path );
/* returns the number of characters printed on success or a negative number */
int          log_message( LogMessage_t lt , const char * msg );
/* returns -1 on error, 0 on success  */
int          log_close  ( void );

/* set logger to print also to STDOUT */
void         log_set_stdout( void );
/* do not print to STDOUT */
void         log_unset_stdout( void );
/* returns the FILE * pointer to the log file */
FILE *       log_get_stream( void );
/* Set the log level */
void         log_set_loglevel( LogMessage_t level);
/* returns the log level */
LogMessage_t log_get_loglevel( void );

#endif
