LIB     := lib/
BIN     := bin/
RES     := res/

CC      := gcc
CCFLAGS := -Wall -Wextra -O2 -g
LDFLAGS := -lpthread -lrt

LIBNAME  := chronometer.so

PROGNAME := locks
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


default: all

all: $(LIB)$(LIBNAME)

check: ./script/check $(BIN)$(PROGNAME) $(LIB)$(LIBNAME) | $(RES)
	$(call cmd-print,  CALL    $<)
	$(Q)$^ $(RES)results.csv


$(LIB)$(LIBNAME): $(wildcard src/*.c) version.ld | $(LIB)
	$(call cmd-print,  CCLD    $@)
	$(Q)$(CC) $(CCFLAGS) -Iinclude/ -fPIC -shared $(filter %.c, $^) \
	    -o $@  -Wl,--version-script -Wl,version.ld $(LDFLAGS) -ldl

# $(BIN)$(PROGNAME): $(wildcard t/*.c) | $(BIN)
# 	$(call cmd-print,  CCLD    $@)
# 	$(Q)$(CC) $(CCFLAGS) $(DEFINES) $^ -o $@ $(LDFLAGS)

$(BIN)gomp: t/gomp.c | $(BIN)
	$(call cmd-print,  CCLD    $@)
	$(Q)$(CC) $(CCFLAGS) -fopenmp $< -o $@


$(LIB) $(BIN) $(RES):
	$(call cmd-print,  MKDIR   $@)
	$(Q)mkdir $@

clean:
	$(call cmd-print,  CLEAN)
	$(Q)rm -rf $(patsubst %/, `readlink %`, $(LIB) $(BIN) $(RES)) \
                   $(patsubst %/,           % , $(LIB) $(BIN) $(RES)) \
            2>/dev/null || true
