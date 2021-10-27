# Linux Handling of Devices

## Read Blocking Devices


https://www.kernel.org/doc/Documentation/block/

https://www.kernel.org/doc/Documentation/block/queue-sysfs.txt


Can find all the device info in /proc


For example for `sdb` you can track information on:
```
root@darkstar:~/raw-cloner/utils# cat   /sys/block/sdb/queue/physical_block_size
512
root@darkstar:~/raw-cloner/utils# cat   /sys/block/sdb/queue/optimal_io_size
0
root@darkstar:~/raw-cloner/utils# cat   /sys/block/sdb/queue/minimum_io_size
512
root@darkstar:~/raw-cloner/utils# cat   /sys/block/sdb/queue/logical_block_size
512
root@darkstar:~/raw-cloner/utils# cat   /sys/block/sdb/queue/hw_sector_size
512

show all files that are Read Only - produced by kernel
to support the device sdb 

root@darkstar:~/raw-cloner/utils# ls -lht  /sys/block/sdb/queue/ | grep -v "rw"
total 0
-r--r--r-- 1 root root 4.0K Oct 16 07:10 max_hw_sectors_kb
-r--r--r-- 1 root root 4.0K Oct 16 07:10 hw_sector_size
-r--r--r-- 1 root root 4.0K Oct 16 07:10 discard_granularity
-r--r--r-- 1 root root 4.0K Oct 16 07:10 write_zeroes_max_bytes
-r--r--r-- 1 root root 4.0K Oct 16 07:10 max_discard_segments
-r--r--r-- 1 root root 4.0K Oct 16 07:10 chunk_sectors
-r--r--r-- 1 root root 4.0K Oct 16 07:10 optimal_io_size
-r--r--r-- 1 root root 4.0K Oct 16 07:10 discard_max_hw_bytes
-r--r--r-- 1 root root 4.0K Oct 16 07:10 max_segment_size
-r--r--r-- 1 root root 4.0K Oct 16 07:10 logical_block_size
-r--r--r-- 1 root root 4.0K Oct 16 07:10 physical_block_size
-r--r--r-- 1 root root 4.0K Oct 16 07:10 dax
-r--r--r-- 1 root root 4.0K Oct 16 07:10 max_segments
-r--r--r-- 1 root root 4.0K Oct 16 07:10 write_same_max_bytes
-r--r--r-- 1 root root 4.0K Oct 16 07:10 minimum_io_size
-r--r--r-- 1 root root 4.0K Oct 16 07:10 discard_zeroes_data
-r--r--r-- 1 root root 4.0K Oct 16 07:10 zoned
-r--r--r-- 1 root root 4.0K Oct 16 07:10 max_integrity_segments


```