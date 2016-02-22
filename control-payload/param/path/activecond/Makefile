LIB     := lib/
BIN     := bin/

CC      := gcc
CCFLAGS := -Wall -Wextra -g
LDFLAGS := -lpthread -lrt

LIBNAME  := activecond.so

PROGNAME := test
VERSION  := 1.0.0
AUTHOR   := Gauthier Voron
MAILTO   := gauthier.voron@lip6.fr

DEFINES  := -DPROGNAME='"$(PROGNAME)"' -DVERSION='"$(VERSION)"' \
            -DAUTHOR='"$(AUTHOR)"'     -DMAILTO='"$(MAILTO)"'   \


V ?= 1
ifeq ($(V),1)
  define cmd-print
    @echo '$(1)'
  endef
endif
ifneq ($(V),2)
  Q := @
endif

LOAD ?= 0000


default: check

all: $(LIB)$(LIBNAME)


check: check-mutex check-signal check-broadcast

check-mutex: script/mutex.sh $(BIN)mutex $(LIB)$(LIBNAME)
	$(call cmd-print,  CALL    $<)
	$(Q)echo "  type    glibc     active"
	$(Q)printf "  lopar   " ; ./$^ 300000$(LOAD)  50 1000
	$(Q)printf "  hipar   " ; ./$^ 600000$(LOAD) 100 1000
	$(Q)printf "  nopar   " ; ./$^  30000$(LOAD)   5 1000
	$(Q)printf "  nocont  " ; ./$^ 200000$(LOAD)  25 10000
	$(Q)printf "  hicont  " ; ./$^ 300000$(LOAD)  50 100

check-signal: script/signal.sh $(BIN)signal $(LIB)$(LIBNAME)
	$(call cmd-print,  CALL    $<)
	$(Q)echo "  type    glibc     active"
	$(Q)printf "  lopar   " ; ./$^ 300000$(LOAD)  50  50 1000
	$(Q)printf "  hipar   " ; ./$^ 600000$(LOAD) 100   0 1000
	$(Q)printf "  nopar   " ; ./$^  30000$(LOAD)   5   0 1000
	$(Q)printf "  nocont  " ; ./$^ 200000$(LOAD)  25  25 10000
	$(Q)printf "  hicont  " ; ./$^ 300000$(LOAD)  50   5 100

check-broadcast: script/broadcast.sh $(BIN)broadcast $(LIB)$(LIBNAME)
	$(call cmd-print,  CALL    $<)
	$(Q)echo "  type    glibc     active"
	$(Q)printf "  lopar   " ; ./$^ 200000$(LOAD)  10  50 1000
	$(Q)printf "  hipar   " ; ./$^ 100000$(LOAD) 100 100    0
	$(Q)printf "  nopar   " ; ./$^ 30000$(LOAD)  100   0    0
	$(Q)printf "  hicont  " ; ./$^ 10000$(LOAD)   10 100    1

check-barrier: script/barrier.sh $(BIN)barrier $(LIB)$(LIBNAME)
	$(call cmd-print,  CALL    $<)
	$(Q)echo "  type    glibc     active"
	$(Q)printf "  lopar   " ; ./$^ 200000$(LOAD)  10  50 1000


$(LIB)$(LIBNAME): $(wildcard src/*.c) version.ld | $(LIB)
	$(call cmd-print,  CCLD    $@)
	$(Q)$(CC) $(CCFLAGS) -Iinclude/ -fPIC -shared $(filter %.c, $^) \
	    -o $@  -Wl,--version-script -Wl,version.ld $(LDFLAGS)

$(BIN)mutex: t/mutex.c | $(BIN)
	$(call cmd-print,  CCLD    $@)
	$(Q)$(CC) $(CCFLAGS) $(DEFINES) $^ -o $@ $(LDFLAGS)

$(BIN)signal: t/signal.c | $(BIN)
	$(call cmd-print,  CCLD    $@)
	$(Q)$(CC) $(CCFLAGS) $(DEFINES) $^ -o $@ $(LDFLAGS)

$(BIN)broadcast: t/broadcast.c | $(BIN)
	$(call cmd-print,  CCLD    $@)
	$(Q)$(CC) $(CCFLAGS) $(DEFINES) $^ -o $@ $(LDFLAGS)

$(BIN)barrier: t/barrier.c | $(BIN)
	$(call cmd-print,  CCLD    $@)
	$(Q)$(CC) $(CCFLAGS) -fopenmp $(DEFINES) $^ -o $@ $(LDFLAGS)


$(LIB) $(BIN) $(RES):
	$(call cmd-print,  MKDIR   $@)
	$(Q)mkdir $@

clean:
	$(call cmd-print,  CLEAN)
	$(Q)rm -rf $(patsubst %/, `readlink %`, $(LIB) $(BIN) $(RES)) \
                   $(patsubst %/,           % , $(LIB) $(BIN) $(RES)) \
            2>/dev/null || true
