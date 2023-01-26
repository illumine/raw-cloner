# Project: raw-cloner
# Makefile created by Michael Mountrakis

CC       = gcc
OBJ      = rawcloner.o  user-options.o raw-io.o logger.o util.o stats.o
LINKOBJ  = rawcloner.o  user-options.o raw-io.o logger.o util.o stats.o
LIBS     =
INCS     =
CXXINCS  = -g
BIN      = raw-cloner
CXXFLAGS = $(CXXINCS)
CFLAGS   = $(INCS)
RM       = rm -f

.PHONY: all all-before all-after clean clean-custom

all: all-before $(BIN) all-after

clean: clean-custom
	${RM} $(OBJ) $(BIN)

$(BIN): $(OBJ)
	$(CC) $(LINKOBJ) -o $(BIN) $(LIBS)

util.o: util.c
	$(CC) -c util.c -o util.o $(CFLAGS)

stats.o: stats.c
	$(CC) -c stats.c -o stats.o $(CFLAGS)

raw-io.o: raw-io.c
	$(CC) -c raw-io.c -o raw-io.o $(CFLAGS)

logger.o: logger.c
	$(CC) -c logger.c -o logger.o $(CFLAGS)

user-options.o: user-options.c logger.o
	$(CC) -c user-options.c -o user-options.o $(CFLAGS)

rawcloner.o: rawcloner.c
	$(CC) -c rawcloner.c -o rawcloner.o $(CFLAGS)

rawcloner: rawcloner.o user-options.o logger.o stats.o util.o
	$(CC) -o rawcloner rawcloner.o user-options.o

copydisk: copydisk.c
	$(CC) copydisk.c -o copydisk $(CFLAGS)
	
rcopydisk: rcopydisk.c
	$(CC) rcopydisk.c -o rcopydisk $(CFLAGS)

fbcopy: fbcopy.c
	$(CC) fbcopy.c -o fbcopy $(CFLAGS)

expcopy: expcopy.c
	$(CC) expcopy.c -o expcopy $(CFLAGS)
	
install: all
	mv raw-cloner /bin/raw-cloner
