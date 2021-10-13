#include <stdio.h>
#include <unistd.h>

#include "util.h"

size_t get_file_size(const char * path){

  FILE *fp = fopen(path, "rb");
  if (!fp)
    return -1;
  
	
  if( fseek(fp, 0L, SEEK_END) < 0 )
    return -1;
  
  return ftell(fp);	
	
}
