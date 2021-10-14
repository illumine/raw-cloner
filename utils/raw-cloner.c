#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <pthread.h>
#include <unistd.h>
#include <errno.h>

/* required to support 64 raw I/O. See unistd.h */
#define _FILE_OFFSET_BITS 64

#define FORWARD 'F'
#define BACKWARD 'B'
#define NO_IMAGE 0
#define ONE_BYTE 1
#define EX_ERROR 1
#define EX_OK 0
#define VERSION "v1.0"
#define BUILD_DATE "7/10/2021"

/* Input from the Command line - User Options */
typedef struct UserOptions_t {
  char * in_path;
  char * out_path;
  char * log_path;
  size_t buf_size;
  int read_retries;
  size_t start_offset;
  size_t end_offset;
  size_t image_chunk_size;

  char direction;
}
UserOptions_t;

UserOptions_t UserOptions;

void print_version(void) {
  printf("\nVery old Version %s  built on %s\n", VERSION, BUILD_DATE);
}

void usage(char * progname) {
  printf("\nThis is a very old version. Used only for developer reference. Do not use it!");	
  printf("\n%s -i <input path> -o <output path> -l <log path> -b <buffer size> -r <read retries> -s <start offset> -e <end offset> -I <image size> -wv", progname);
  printf("\n-v version info");
  printf("\n-w read the disk/file backwards - from end to start.");
  print_version();
  printf("\n");
}

int parse_cli_arguments(UserOptions_t * user_options, int argc, char * argv[]) {
  int i, opt;
  opterr = 0;

  if (argc == 1) {
    usage(argv[0]);
    exit(EX_OK);
  }

  while ((opt = getopt(argc, argv, "i:o:l:b:r:s:e:I:wv")) != -1) {
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
      /* switches */
    case 'w':
      user_options -> direction = BACKWARD;
      break;
    case 'v':
      print_version();
      exit(EX_OK);
    default:
      usage(argv[0]);
      abort();
    }
  }
  //debug_user_options(user_options);    

  for (i = optind; i < argc; i++)
    printf("\nNon-option argument %sn", argv[i]);

  return 0;
}

void init_user_options(UserOptions_t * user_options) {
  user_options -> in_path = NULL;
  user_options -> out_path = NULL;
  user_options -> log_path = NULL;
  user_options -> buf_size = BUFSIZ;
  user_options -> read_retries = 1;
  user_options -> start_offset = 0;
  user_options -> end_offset = -1;
  user_options -> image_chunk_size = NO_IMAGE;
  user_options -> direction = FORWARD;
}

void debug_user_options(UserOptions_t * user_options) {
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
  printf("\nRead Direction %c", user_options -> direction);
  printf("\n");

  return;
}

void mark_bad_byte(int fdi) {
  off_t offset = lseek(fdi, 0, SEEK_CUR);
  printf("B%ld,", offset);
}

ssize_t full_write(int desc,
  const char * ptr, size_t len) {
  size_t total_written;

  total_written = 0;
  while (len > 0) {
    int written = write(desc, ptr, len);
    /* write on an old Slackware Linux 1.2.13 returns zero when
	 I try to write more data than there is room on a floppy disk.
	 This puts dd into an infinite loop.  Reproduce with
	 dd if=/dev/zero of=/dev/fd0.  If you have this problem,
	 consider upgrading to a newer kernel.  */
    if (written < 0) {
      #ifdef EINTR
      if (errno == EINTR)
        continue;
      #endif
      return written;
    }
    total_written += written;
    ptr += written;
    len -= written;
  }
  return total_written;
}

ssize_t safe_read(int desc, void * ptr, size_t len) {
  ssize_t n_chars;

  if (len <= 0)
    return len;

  #ifdef EINTR
  do {
    n_chars = read(desc, ptr, len);
  } while (n_chars < 0 && errno == EINTR);
  #else
  n_chars = read(desc, ptr, len);
  #endif

  return n_chars;
}

int safe_close(int fd) {
  if (close(fd) != 0)
    do
      if (errno != EINTR)
        return -1;
    while (close(fd) != 0 && errno != EBADF);

  return 0;
}

void copy_forward(void) {

  int fdi = -1, fdo = -1, fdl = -1;
  char msg[BUFSIZ] = {
    '\0'
  };
  size_t bytes_read = 0, buf_size = 0, bytes_writen = 0;
  char * buffer = NULL;
  int read_retries = 0;

  fdi = open(UserOptions.in_path, O_RDONLY);
  if (fdi < 0) {
    sprintf(msg, "\nError opening %s for read.\n", UserOptions.in_path);
    perror(msg);
    exit(EX_ERROR);
  } else {
    printf("\nSource %s opened for read.\n", UserOptions.in_path);
  }

  fdo = open(UserOptions.out_path, O_WRONLY | O_CREAT, 0644);
  if (fdo < 0) {
    sprintf(msg, "\nError opening file %s for write.\n", UserOptions.out_path);
    perror(msg);
    exit(EX_ERROR);
  } else {
    printf("\nDestination %s opened for writing.\n", UserOptions.out_path);
  }

  /* Initial Buffer Allocation according to user defined params*/
  if ((buffer = (char * ) calloc(UserOptions.buf_size, sizeof(char))) == NULL) {
    sprintf(msg, "\nError allocating buffer of size %ld. Consider using a smaller buffer size.\n", UserOptions.buf_size);
    perror(msg);
    exit(EX_ERROR);
  }

  while (1) {
    read_retries = 0;
    buf_size = UserOptions.buf_size;
    bytes_read = 0;

    READ:
      /* try to read the buffer */
      for (read_retries = 0; read_retries < UserOptions.read_retries; read_retries++) {
        bytes_read = safe_read(fdi, buffer, buf_size);
        if (bytes_read < 0)
          continue;
        else
          break;
      }

    if (bytes_read < 0) {
      buf_size = (size_t)(1 + buf_size / 2);

      if (buf_size == 1) {
        mark_bad_byte(fdi);
        lseek(fdi, ONE_BYTE, SEEK_CUR);
        read_retries = 0;
        buf_size = UserOptions.buf_size;
        goto READ;
      }
    } else if (bytes_read == 0)
      /* EOF */
      break;
    else {
      bytes_writen = full_write(fdo, buffer, bytes_read);
      if (bytes_writen < 0) {
        sprintf(msg, "\nFailed write %ld to %s.\n", bytes_read, UserOptions.out_path);
        perror(msg);
      }
    }

  }

  /* post processing steps */
  free(buffer);
  safe_close(fdi);
  safe_close(fdo);
  return;
}

void copy_to_image() {

  int fdi = -1, fdo = -1, fdl = -1;
  char msg[BUFSIZ] = {
    '\0'
  };
  size_t bytes_read = 0, buf_size = 0, bytes_writen = 0;
  char * buffer = NULL;
  int read_retries = 0;

  size_t current_image_chunk_size = 0;
  int current_image_chunk_n = 1;
  char current_image_chunk_name[BUFSIZ] = {
    '\0'
  };

  fdi = open(UserOptions.in_path, O_RDONLY);
  if (fdi < 0) {
    sprintf(msg, "\nError opening %s for read.\n", UserOptions.in_path);
    perror(msg);
    exit(EX_ERROR);
  } else {
    printf("\nSource %s opened for read.\n", UserOptions.in_path);
  }

  /* Initial Buffer Allocation according to user defined params*/
  if ((buffer = (char * ) calloc(UserOptions.buf_size, sizeof(char))) == NULL) {
    sprintf(msg, "\nError allocating buffer of size %ld. Consider using a smaller buffer size.\n", UserOptions.buf_size);
    perror(msg);
    exit(EX_ERROR);
  }

  current_image_chunk_size = 0;
  current_image_chunk_n = 1;
  current_image_chunk_name[0] = '\0';
  while (1) {
    read_retries = 0;
    buf_size = UserOptions.buf_size;
    bytes_read = 0;

    sprintf(current_image_chunk_name, "%s%05d.raw", UserOptions.out_path, current_image_chunk_n);
    fdo = open(current_image_chunk_name, O_WRONLY | O_CREAT, 0644);
    if (fdo < 0) {
      sprintf(msg, "\nError opening file %s for write.\n", current_image_chunk_name);
      perror(msg);
      exit(EX_ERROR);
    } else {
      printf("\nDestination %s opened for writing.\n", current_image_chunk_name);
    }

    while (current_image_chunk_size + UserOptions.buf_size <= UserOptions.image_chunk_size) {

      READ:
        /* try to read the buffer */
        for (read_retries = 0; read_retries < UserOptions.read_retries; read_retries++) {
          bytes_read = safe_read(fdi, buffer, buf_size);
          if (bytes_read < 0)
            continue;
          else
            break;
        }

      if (bytes_read < 0) {
        buf_size = (size_t)(1 + buf_size / 2);

        if (buf_size == 1) {
          mark_bad_byte(fdi);
          lseek(fdi, ONE_BYTE, SEEK_CUR);
          read_retries = 0;
          buf_size = UserOptions.buf_size;
          goto READ;
        }
      } else if (bytes_read == 0)
        /* EOF */
        break;
      else {
        bytes_writen = full_write(fdo, buffer, bytes_read);
        if (bytes_writen < 0) {
          sprintf(msg, "\nFailed write %ld to %s.\n", bytes_read, current_image_chunk_name);
          perror(msg);
        }
        current_image_chunk_size += bytes_writen;
        printf("\ncurrent_image_chunk_size %07ld bytes_writen %07ld UserOptions.image_chunk_size %07ld", current_image_chunk_size, bytes_writen, UserOptions.image_chunk_size);
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
  free(buffer);
  safe_close(fdi);
  return;

}

int main(int argc, char * argv[]) {

  init_user_options( & UserOptions);

  if (parse_cli_arguments( & UserOptions, argc, argv)) {
    fprintf(stderr, "\nError while parsing arguments!\n");
    exit(EX_ERROR);
  }
  debug_user_options( & UserOptions);

  if (UserOptions.image_chunk_size == NO_IMAGE)
    copy_forward();
  else
    copy_to_image();

  exit(EX_OK);
}
