SRC := src/
TST := t/
INC := include/
OBJ := obj/
LIB := lib/
BIN := bin/

CC      := gcc
CCFLAGS := -Wall -Wextra -O2
SOFLAGS := -fPIC -shared
LDFLAGS := -ldl -lpthread

obj-y       := argument error runtime  
lib-pthread := -lpthread -lrt


V ?= 1
ifeq ($(V),1)
  define print
    @echo '$(1)'
  endef
endif
ifneq ($(V),2)
  Q := @
endif


default: all

all: $(LIB)pin.so
check: $(LIB)pin.so $(BIN)pthread
	$(call print,  CHECK   $(TST)check.sh)
	$(Q)./$(TST)check.sh $(LIB)pin.so $(BIN)


$(LIB)pin.so: $(patsubst %, $(OBJ)%.so, $(obj-y)) | $(LIB)
	$(call print,  LD      $@)
	$(Q)$(CC) $(SOFLAGS) $^ -o $@ $(LDFLAGS)

$(BIN)%: $(TST)%.c | $(BIN)
	$(call print,  CCLD    $@)
	$(Q)$(CC) $(CCFLAGS) $< -o $@ $(lib-$(patsubst $(BIN)%,%,$@))


$(OBJ)%.so: $(SRC)%.c | $(OBJ)
	$(call print,  CC      $@)
	$(Q)$(CC) -c $(CCFLAGS) -I$(INC) $(SOFLAGS) $< -o $@


$(OBJ) $(LIB) $(BIN):
	$(call print,  MKDIR   $@)
	$(Q)mkdir $@


clean:
	$(call print,  CLEAN)
	$(Q)-rm -rf $(OBJ) $(LIB) $(BIN)
