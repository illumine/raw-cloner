# Project: raw-cloner
# Makefile created by Michael Mountrakis

CC       = gcc
WINDRES  = windres.exe
OBJ      = rawcloner.o  user-options.o raw-io.o logger.o
LINKOBJ  = rawcloner.o  user-options.o raw-io.o logger.o
LIBS     =
INCS     =
CXXINCS  =
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


raw-io.o: raw-io.c
	$(CC) -c raw-io.c -o raw-io.o $(CFLAGS)
        
logger.o: logger.c
	$(CC) -c logger.c -o logger.o $(CFLAGS)

user-options.o: user-options.c logger.o
	$(CC) -c user-options.c -o user-options.o $(CFLAGS)

rawcloner.o: rawcloner.c
	$(CC) -c rawcloner.c -o rawcloner.o $(CFLAGS)

rawcloner: rawcloner.o user-options.o logger.o
	$(CC) -o rawcloner rawcloner.o user-options.o

install: all
	mv raw-cloner /bin/raw-cloner

