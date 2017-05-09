INC=

CC = mpicc
CFLAGS = -fopenmp -O2 -g $(INC)

LIBDIRS =
LIBS =

LDFLAGS = $(LIBDIRS) $(LIBS)

PROJ = Oblig2 check
OBJS = Oblig2.o

all : $(PROJ)

Oblig2 : $(OBJS) io.o
	$(CC) $(CFLAGS) io.o $< -o $@ $(LDFLAGS)

io.o: io.c
	$(CC) $(CFLAGS) -c io.c

check: io.o check.o
	$(CC) $(CFLAGS) check.o $< -o $@ $(LDFLAGS)

check.o: check.c
	$(CC) $(CFLAGS) -c check.c	

obj-clean:
	$(RM) *.o

exec-clean:
	$(RM) $(PROJ)

autosave-clean:
	$(RM) *~

clean:
	$(MAKE) obj-clean
	$(MAKE) exec-clean
	$(MAKE) autosave-clean
