#ifndef STATS_H
# define STATS_H

#define TDIFF(tstart,tend)  (((tend.tv_sec) + (1.0e-9)*(tend.tv_nsec)) - ((tstart.tv_sec) + (1.0e-9)*(tstart.tv_nsec)))

typedef struct ProgramStats_t {

  size_t bytes_read;
  size_t bytes_written;
  size_t bytes_bad;
  size_t read_retries;
  size_t buffers_read;
  size_t buffers_written;
  size_t buffers_resized;
  size_t timeouts;
  size_t timeout_seconds;
  time_t  time_start,  time_stop;
  clock_t clock_start, clock_stop;
}
ProgramStats_t;


void program_stats_init(ProgramStats_t * program_stats);
void program_stats_debug(ProgramStats_t * program_stats);
void program_stats_log(ProgramStats_t * program_stats);

#endif
