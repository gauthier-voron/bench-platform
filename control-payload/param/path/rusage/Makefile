CC      := gcc
CCFLAGS := -Wall -Wextra -pedantic -O2 -g
LDFLAGS := -lrt

PROGNAME := rusage
VERSION  := 0.0.1
AUTHOR   := Gauthier Voron
MAILTO   := gauthier.voron@lip6.fr

DEFINES  := -DPROGNAME='"$(PROGNAME)"' -DVERSION='"$(VERSION)"' \
            -DAUTHOR='"$(AUTHOR)"'     -DMAILTO='"$(MAILTO)"'   \


default: all

all: $(PROGNAME)

$(PROGNAME): $(wildcard *.c) $(wildcard *.h)
	$(CC) $(CCFLAGS) $(DEFINES) $(filter %.c, $^) -o $@ $(LDFLAGS)

clean:
	rm *~ *.o $(PROGNAME) 2>/dev/null || true
