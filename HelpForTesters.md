# Help for the Testers

## New Version with Disk Offset - For Mr Saltiel...
Example of usage on a 2G virtual disk
```
root@ddarkstar:~/raw-cloner/src# df -h /dev/sdb
Filesystem      Size  Used Avail Use% Mounted on
devtmpfs        2.0G     0  2.0G   0% /dev

root@ddarkstar:~/raw-cloner/src# time ./copydisk /dev/sdb /dev/sdc 102424562
Copy from /dev/sdb to  /dev/sdc using 8192 bytes buffer.
Source File/Disk size is: 1073741824 bytes.
Positioning  File/Disk to offset: 102424562 bytes.
3ffffff2 Read 14 bytes, Written 14 bytes for 118570 times.Reached end of file at 1073741824 bytes.

real    0m23.117s
user    0m0.410s
sys     0m2.416s
```

## Testing Very Large Raw Binary Files
Read:

https://www.computernetworkingnotes.com/linux-tutorials/generate-or-create-a-large-text-file-in-linux.html


Try to create a very large binary file and copy it with `rcopydisk`:
```
root@ddarkstar:~/raw-cloner/src# dd if=/dev/zero of=500kib-file bs=1024 count=500
500+0 records in
500+0 records out
512000 bytes (512 kB, 500 KiB) copied, 0.00174119 s, 294 MB/s
```
Then try to copy this large file with `rdiskcopy` command:

```
root@ddarkstar:~/raw-cloner/src# ./rcopydisk 500kib-file out
Copy from 500kib-file to  out using 8192 bytes buffer.
Source File/Disk size is: 512000 bytes.
Destination File/Disk offset is set to: 512000 bytes.
Destination File/Disk offset currently: 512000 bytes.
```

Compare the results:
```
root@ddarkstar:~/raw-cloner/src# ls -l 500kib-file out
-rw-r--r-- 1 root root 512000 Nov  1 15:14 500kib-file
---------- 1 root root 512000 Nov  1 15:14 out          <----------- SAME SIZE
root@ddarkstar:~/raw-cloner/src# diff  500kib-file out
root@ddarkstar:~/raw-cloner/src#                    <----------- EMPTY SET = NO DIFFERENCES

```


## Testing Large Text Files
```
for i in {1..10000}; do  echo "0123456789" >> in ; done

root@ddarkstar:~/raw-cloner/src# ./rcopydisk in out
Copy from in to  out using 8192 bytes buffer.
Source File/Disk size is: 122111 bytes.
Destination File/Disk offset is set to: 122111 bytes.
Destination File/Disk offset currently: 122111 bytes.
root@ddarkstar:~/raw-cloner/src# ls -l in out
-rw-r--r-- 1 root root 122111 Nov  1 15:01 in
---------- 1 root root 122111 Nov  1 15:03 out <----------- SAME SIZE for in/out!!!!

root@ddarkstar:~/raw-cloner/src# diff in out
root@ddarkstar:~/raw-cloner/src#          <----------- EMPTY SET = NO DIFFERENCES between in/out!!!!!!


```

## Testing  Disks instead of Files
Supposing we have /dev/sdb - the source disk and /dev/sdc the destination disk. 

First step is to copy the contents of / to /dev/sdb.


```
 su -i
 mkdir /source
 mount /dev/sdb /source -t ext4 -o rw,relatime,data=ordered
 mkfs.ext4 /dev/sdb
 mount /dev/sdb /mnt -t ext4 -o rw,relatime,data=ordered
 cp /* /source
 ls -lt /source
 umount /dev/sdb 
```

Second step is to test our program by copying /dev/sdb to /dev/sdc

```
root@ddarkstar:~/raw-cloner/src# time ./copydisk /dev/sdb /dev/sdc
Copy from /dev/sdb to  /dev/sdc using 8192 bytes buffer.

real    0m4.901s
user    0m0.141s
sys     0m1.761s
root@ddarkstar:~/raw-cloner/src# df -h /dev/sdb
Filesystem      Size  Used Avail Use% Mounted on
devtmpfs        2.0G     0  2.0G   0% /dev
root@ddarkstar:~/raw-cloner/src# df -h /dev/sdc
Filesystem      Size  Used Avail Use% Mounted on
devtmpfs        2.0G     0  2.0G   0% /dev


```