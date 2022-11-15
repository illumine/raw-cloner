CC     = gcc
OPTS   = -Wall -O6
ALL    = avgtime backward-copy forward-copy rcopydisk


all: $(ALL)

clean:
	rm -f $(ALL)

avgtime:
	$(CC)  $(OPTS) avgtime.c  -o avgtime
backward-copy:
	$(CC)  $(OPTS) backward-copy.c -o backward-copy
forward-copy:
	$(CC)  $(OPTS)  forward-copy.c -o forward-copy
rcopydisk:
	$(CC)  $(OPTS) rcopydisk.c -o rcopydisk
	
