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

If $n$ - disk size in bytes size,  $b\ll n$ is the buffer size in bytes, $r$ is the read retries and $e$ is the number of total error bytes,
the performance of the algorithm has as follows:

- Best     $\Omega(n)   = n/b$ This is the case the disk has no errors. In that case, program will read $n/b$ times. 


- Worst    $O(n) = nrlog(b)$ This is the case, the disk is full of error bytes.


- Average  $\Theta(n)   = (n-e)/b + erlog(b)$  This is the case disk has a $e$ error bytes, or $e/n$ is the probability of an error byte in the disk.
For example if this percentage is 5%, then Average limit is : $(n-.05n)/b + 0.05nlog(b)$

Those are the *theoritical limits of the algorithm*, however in practice, one has to encount also the sleep time before reads so that the disk
will not be more damaged due to very fast concurent reads and CPU heat is increased.

To give you an idea, if we set 1ms sleep interval, the time expected is multiplied by 1000....

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