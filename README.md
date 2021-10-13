# RAW CLONER

A program to byte-by-byte clone a disk to another. Beta Version!!!

Requires:
 
* source and destination disks to be unmounted!
* obviously destination disk size > source disk size 

Performance: 
Worst Case: Omikron(n) = n(b-1)r

Average algoritmic limit Theta(n) = nbr

Best Case: Omega(n) = n/b

n: disk size, b: buffer size, r: read retry
  
## Compilation
Requires git, gcc, make
```
git clone https://github.com/illumine/raw-cloner
make
make install
```


## Tips

- Source disks must be unmounted
- Buffer must be small since it is created on continous memory with `calloc` instead of `malloc` so it requires continous free space
- If you use Image option (-I),  make sure image chunk is a multiplier of Buffer size 




## Tests

### Linux
```
root@darkstar:~/raw-cloner# cat  /proc/version
Linux version 4.15.0-142-generic (buildd@lgw01-amd64-036) (gcc version 7.5.0 (Ubuntu 7.5.0-3ubuntu1~18.04)) #146-Ubuntu SMP Tue Apr 13 01:11:19 UTC 2021

root@ddarkstar:~/raw-cloner# gcc -v
Using built-in specs.
COLLECT_GCC=gcc
COLLECT_LTO_WRAPPER=/usr/lib/gcc/x86_64-linux-gnu/7/lto-wrapper
OFFLOAD_TARGET_NAMES=nvptx-none
OFFLOAD_TARGET_DEFAULT=1
Target: x86_64-linux-gnu
Configured with: ../src/configure -v --with-pkgversion='Ubuntu 7.5.0-3ubuntu1~18.04' --with-bugurl=file:///usr/share/doc/gcc-7/README.Bugs --enable-languages=c,ada,c++,go,brig,d,fortran,objc,obj-c++ --prefix=/usr --with-gcc-major-version-only --program-suffix=-7 --program-prefix=x86_64-linux-gnu- --enable-shared --enable-linker-build-id --libexecdir=/usr/lib --without-included-gettext --enable-threads=posix --libdir=/usr/lib --enable-nls --enable-bootstrap --enable-clocale=gnu --enable-libstdcxx-debug --enable-libstdcxx-time=yes --with-default-libstdcxx-abi=new --enable-gnu-unique-object --disable-vtable-verify --enable-libmpx --enable-plugin --enable-default-pie --with-system-zlib --with-target-system-zlib --enable-objc-gc=auto --enable-multiarch --disable-werror --with-arch-32=i686 --with-abi=m64 --with-multilib-list=m32,m64,mx32 --enable-multilib --with-tune=generic --enable-offload-targets=nvptx-none --without-cuda-driver --enable-checking=release --build=x86_64-linux-gnu --host=x86_64-linux-gnu --target=x86_64-linux-gnu
Thread model: posix
gcc version 7.5.0 (Ubuntu 7.5.0-3ubuntu1~18.04)

root@ddarkstar:~/raw-cloner# file raw-cloner
raw-cloner: ELF 64-bit LSB shared object, x86-64, version 1 (SYSV), dynamically linked, interpreter /lib64/ld-linux-x86-64.so.2, for GNU/Linux 3.2.0, BuildID[sha1]=edc63cc8aa0b87776af6f942416045625855fac3, not stripped

```

### Windows System Information

```
OS Name	Microsoft Windows 10 Enterprise
Version	10.0.19042 Build 19042
Other OS Description 	Not Available
OS Manufacturer	Microsoft Corporation
System Name	WinStar
System Manufacturer	HP
System Model	HP EliteBook 850 G4
System Type	x64-based PC
System SKU	2NG04EC#ABD
Processor	Intel(R) Core(TM) i5-7300U CPU @ 2.60GHz, 2712 Mhz, 2 Core(s), 4 Logical Processor(s)
BIOS Version/Date	HP P78 Ver. 01.38, 12/29/2020

```
Compiler info

```
C:\Users\c5179796>gcc -v
Using built-in specs.
COLLECT_GCC=gcc
COLLECT_LTO_WRAPPER=C:/Program\ Files\ (x86)/Dev-Cpp/TDM-GCC/bin/../libexec/gcc/x86_64-w64-mingw32/9.2.0/lto-wrapper.exe
Target: x86_64-w64-mingw32
Configured with: ../../../src/gcc-git-9.2.0/configure --build=x86_64-w64-mingw32 --enable-targets=all --enable-languages=ada,c,c++,fortran,lto,objc,obj-c++ --enable-libgomp --enable-lto --enable-graphite --enable-cxx-flags=-DWINPTHREAD_STATIC --disable-build-with-cxx --disable-build-poststage1-with-cxx --enable-libstdcxx-debug --enable-threads=posix --enable-version-specific-runtime-libs --enable-fully-dynamic-string --enable-libstdcxx-threads --enable-libstdcxx-time --with-gnu-ld --disable-werror --disable-nls --disable-win32-registry --enable-large-address-aware --disable-rpath --disable-symvers --prefix=/mingw64tdm --with-local-prefix=/mingw64tdm --with-pkgversion=tdm64-1 --with-bugurl=http://tdm-gcc.tdragon.net/bugs
Thread model: posix
gcc version 9.2.0 (tdm64-1)
```

## Case: Raw disk  clone to another disk copy
 
/dev/sdb  is the source disk as ext4 filesystem

/dev/sdc  is the destination disk


```
root@darkstar# ls -l /dev/sdb
brw-rw---- 1 root disk 8, 16 Oct  7 10:47 /dev/sdb
root@dynatrace-dev:~# ls -l /dev/sdc
brw-rw---- 1 root disk 8, 32 Oct 13 10:13 /dev/sdc

```

 
Mount, format and fill the source disk commands:
```	
 su -i
 mkdir /mnt
 mount /dev/sdb /mnt -t ext4 -o rw,relatime,data=ordered
 mkfs.ext4 /dev/sdb
 mount /dev/sdb /mnt -t ext4 -o rw,relatime,data=ordered
 cp /* /mnt
 ls -lt /mnt
 umount /dev/sdb
```
 
 Fire the raw-cloner command:
``` 
 ./raw-cloner -i /dev/sdb -o /dev/sdc  -l log.txt -b 1048576  -r 1
 
root@darkstar:~/raw-cloner# ./raw-cloner -i /dev/sdb -o /dev/sdc  -l log.txt -b 1024   -r 1
2021-10-13 15:58:12 INFO  User Options:
2021-10-13 15:58:12 INFO  Input path: /dev/sdb
2021-10-13 15:58:12 INFO  Output path: /dev/sdc
2021-10-13 15:58:12 INFO  Logfile path: log.txt
2021-10-13 15:58:12 INFO  Read Buffer size: 1048576
2021-10-13 15:58:12 INFO  Read Retries Attemps: 1
2021-10-13 15:58:12 INFO  Start Offset: 0
2021-10-13 15:58:12 INFO  End Offset: -1
2021-10-13 15:58:12 INFO  Image Chunk Size: 0
2021-10-13 15:58:12 INFO  Read Direction F
2021-10-13 15:58:12 INFO  Read timeout -1
2021-10-13 15:58:12 INFO  Log Level 0
2021-10-13 15:58:12 INFO  Log also to stdout 1
2021-10-13 15:58:12 INFO  User Options finished.
2021-10-13 15:58:12 INFO  copy_forward() started.
2021-10-13 15:58:12 INFO  Source /dev/sdb opened for read. File descriptor 4.
2021-10-13 15:58:12 INFO  Destination /dev/sdc opened for writing. File descriptor 5.
2021-10-13 15:58:12 INFO  Buffer of size 1048576 alocated at 7FBDF6838010 and initialized.
2021-10-13 15:58:12 INFO  Source seeked at 0.
2021-10-13 15:58:16 INFO  copy_forward() ended with status 0.
2021-10-13 15:58:16 INFO  Program exits with status 0.
```
Disk /dev/sdb of size 1GB copied to /dev/sdc in 4 seconds.


Check Results on the destination
```
mount /dev/sdc /mnt -t ext4 -o rw,relatime,data=ordered
ls -lt /mnt
```


## Case: Clone disk backwards

```
root@dynatrace-dev:~# date ; raw-cloner -i /dev/sdb -o /dev/sdc  -l log.txt -b 1048576  -B ; date
Wed Oct 13 10:22:40 UTC 2021

Input path: /dev/sdb
Output path: /dev/sdc
Logfile path: log.txt
Read Buffer size: 1048576
Read Retries Attemps: 1
Start Offset: 0
End Offset: -1
Image Chunk Size: 0
Read Direction B
Read timeout -1

Source /dev/sdb opened for read.

Destination /dev/sdc bytes opened for writing.

Buffer of size 1048576 bytes was allocated

Source File/Disk size is: 1073741824 bytes.

Destination File/Disk offset is set to: 1073741824 bytes.

Destination File/Disk offset currently: 1073741824 bytes.

Wed Oct 13 10:36:34 UTC 2021
```
For 1GB reverse byte clone took 14 mins. 

## Debug
On Ubuntu

Compile with debug flag:
```
gcc raw-cloner.c -o raw-cloner -g
```

Install GDB

```
su -i
apt get gdb
```

Run GDB
See also https://www.cs.umd.edu/~srhuang/teaching/cmsc212/gdb-tutorial-handout.pdf
```
root@dynatrace-dev:~/raw-cloner# gdb
GNU gdb (Ubuntu 8.1.1-0ubuntu1) 8.1.1
Copyright (C) 2018 Free Software Foundation, Inc.
....


(gdb) file raw-cloner
Reading symbols from raw-cloner...done.

(gdb) run -i raw-cloner.c -o raw  -l log.txt -b 1024   -r 1  -I 5
Starting program: /root/raw-cloner/raw-cloner -i raw-cloner.c -o raw  -l log.txt -b 1024   -r 1  -I 5

Program received signal SIGSEGV, Segmentation fault.
__GI_____strtol_l_internal (nptr=0x0, endptr=0x0, base=10, group=<optimized out>, loc=0x7ffff7bbb560 <_nl_global_locale>) at ../stdlib/strtol_l.c:292
292     ../stdlib/strtol_l.c: No such file or directory.
(gdb) where
#0  __GI_____strtol_l_internal (nptr=0x0, endptr=0x0, base=10, group=<optimized out>, loc=0x7ffff7bbb560 <_nl_global_locale>) at ../stdlib/strtol_l.c:292
#1  0x0000555555554e89 in parse_cli_arguments (user_options=0x555555757080 <UserOptions>, argc=13, argv=0x7fffffffe578) at raw-cloner.c:95
#2  0x0000555555555845 in main (argc=13, argv=0x7fffffffe578) at raw-cloner.c:410
(gdb)

```
