# Clone Algorithm

The algorithm is based on the Bisec Method for reading a source file/disk using a buffer of size B bytes and writting the file to a destination file/disk.
The algorithm is very simple and has as follows:

1. If Buffer size is 1 byte, move source disk offset 1 byte and set B = MAX_BUF_SIZE
2. Try to  read a Buffer of size B 
4. If read fails, set B = B/2 and goto 1, else write Buffer to output

The previous can be written in C style pseudocode:
```
buf[MAX_BUF_SIZE]
fdi =  open( hd_source)
fdo =  open( hd_dest  )
set_input_timeout( fdi, READ_TIMEOUT_SEC )


while(1){

     retries = 0
     buf_size = MAX_BUF_SIZE
     read_n = 0
   
READ:    
     for( retries=0 ; retries < MAX_RETRIES; retries ++){
	     read_n = read(fdi, buf, buf_size )
		 if(read_n < 0)
	        continue
	     else
	        break;
     }
		 
		 
	 // N/R SE PERIPTWSI POU DE DIABASTIKE KOVOUME STO MISO TO BUFFER   
     if(read_n < 0){
		buf_size = [1 + buf_size / 2 ]
		    
        if( buf_size == 1){
            mark_bad( lseek(fdi, 0, SEEK_CUR))
			lseek(fdi, ONE_BYTE, SEEK_CUR)
			retries = gg0
			buf_size = MAX_BUF_SIZE
			goto READ
		 }
		 
	  }else if( read_n == 0 )
          // EOF
          break;
	  else{
          write( fdo, buf, read_n)
	  }	 
}
```
## Algorithmic Performance

- Best     Omega(n)   = n/b


- Worst    Omicron(n) = n(b-1)r


- Average  Theta(n)   = bnr

Where: n - problem size,  b<<n  buffer size, r retries 

## Arithmetics of terget platform
```
root@dynatrace-dev:~/raw-cloner# cat test.c
#include<stdio.h>
#include<stdlib.h>

int main(){
  return printf("\n long long %ld, off_t %ld, size_t %ld\n", sizeof(long long), sizeof(off_t), sizeof(size_t));
}
root@dynatrace-dev:~/raw-cloner# ./a.out

 long long 8, off_t 8, size_t 8
root@dynatrace-dev:~/raw-cloner#
```