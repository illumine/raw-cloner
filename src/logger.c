#include <stdio.h>

#include <time.h>

#include "logger.h"


void log_set_stdout(void) {
  log_stdout = 1;
}

void log_unset_stdout(void) {
  log_stdout = 0;
}

FILE * log_get_stream(void) {
  return log_fp;
}

void log_set_loglevel(LogMessage_t level) {
  log_level = level;
}

LogMessage_t log_get_loglevel(void) {
  return log_level;
}

int log_open(const char * path) {
  log_level = INFO;
  log_stdout = 0;

  log_fp = fopen(path, "w+");
  if (!log_fp)
    return -E_STREAM;
  else
    return fileno(log_fp);
}

int log_message(LogMessage_t lt, const char * usr_msg) {

  if (log_level == NOTHING)
    return -E_NO_LOG;

  if (!log_fp)
    return -E_STREAM;

  time_t now;
  char msg[BUFSIZ] = {'\0'};
  char timestamp_str[26];
  struct tm * tm_info;

  now = time(NULL);
  tm_info = localtime( & now);
  strftime(timestamp_str, 26, "%Y-%m-%d %H:%M:%S", tm_info);

  switch (lt) {
  case DEBUG:
    sprintf(msg, "%s DEBUG %s", timestamp_str, usr_msg);
    break;
  case TRACE:
    sprintf(msg, "%s TRACE %s", timestamp_str, usr_msg);
    break;
  case INFO:
    sprintf(msg, "%s INFO  %s", timestamp_str, usr_msg);
    break;
  case ERROR:
    sprintf(msg, "%s ERROR %s", timestamp_str, usr_msg);
    break;
  case FATAL:
    sprintf(msg, "%s TRACE %s", timestamp_str, usr_msg);
    break;
  default:
    sprintf(msg, "%s TRACE %s", timestamp_str, usr_msg);
    break;
  }
  if (log_stdout)
    printf("\n%s",msg);

  if (lt >= log_level)
    return fprintf(log_fp, "\n%s", msg);
  else
    return -E_NO_LOG;
}

int log_close(void) {
  if (log_fp)
    return fclose(log_fp);
  else
    return -E_STREAM;
}
