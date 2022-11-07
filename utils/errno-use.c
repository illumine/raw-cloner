/*
------------------------------------------------------
Program to demonstrate the use of errno

Read 
https://www.tutorialspoint.com/cprogramming/c_error_handling.htm

Can I ever assume that a C stdlib function doesn't use errno?
Answer https://stackoverflow.com/questions/4391026/can-i-ever-assume-that-a-c-stdlib-function-doesnt-use-errno

From Linux Man page for errno
https://man7.org/linux/man-pages/man3/errno.3.html
See section:  A common mistake is to do

 where errno no longer needs to have the value it had upon return
       from somecall() (i.e., it may have been changed by the
       printf(3))
       
for  __FILE__,__LINE__ 
See: https://en.cppreference.com/w/c/program/EXIT_status       
---------------------------------------------------------
*/
#include <stdio.h>
#include <errno.h>
#include <string.h>

extern int errno ;

int main () {

   FILE * pf;
   int errnum;
   pf = fopen ("unexist.txt", "rb");
	
   if (pf == NULL) {
   
      errnum = errno;
      fprintf(stderr, "DEBUG: fopen() failed in file %s at line # %d", __FILE__,__LINE__);
      fprintf(stderr, "Value of errno: %d\n", errno);
      perror("Error printed by perror");
      fprintf(stderr, "Error opening file: %s\n", strerror( errnum ));
   } else {
   
      fclose (pf);
   }
   
   return 0;
}
/**************************  OUT ***************************
Value of errno: 2
Error printed by perror: No such file or directory
Error opening file: No such file or directory
************************************************************
*/

