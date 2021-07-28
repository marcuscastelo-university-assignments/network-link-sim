# Ensures all is the default
all: main

run: main
	./bin/main

# Compiler alternatives, options and flags
LD_FLAGS := -pthread -I./src/main
DEBUG_FLAGS := -O0 -ggdb
WARNING_FLAGS := -Wall -Wno-unused-variable

CXX := g++
CXX_FLAGS := $(WARNING_FLAGS) $(DEBUG_FLAGS) $(LD_FLAGS) 
#

# Binaries and it's dependencies
RULES := main
OBJS := main/main.o main/tui.o main/crc_32.o main/frame.o main/mac.o main/peers.o main/tests.o
#

# Project structure
SRC_DIR := ./src
OBJ_DIR := ./objs
BIN_DIR := ./bin
DEP_DIR := ./deps
#

# Project structure auxiliary variables
SUBDIRS := $(SRC_DIR) $(OBJ_DIR) $(BIN_DIR) $(DEP_DIR)

SOURCES := $(shell find . -name "*.cpp")
DEPS := $(SOURCES:$(SRC_DIR)/%.cpp=$(DEP_DIR)/%.d)

BINARIES := $(addprefix $(BIN_DIR)/,$(RULES))

OBJS := $(addprefix $(OBJ_DIR)/,$(OBJS))
#

# [GLOBAL] Assure subdirectories exist
$(shell mkdir -p $(SUBDIRS))

# Aliases to bin/server, bin/client and bin/test
.PHONY: main

main: .EXTRA_PREREQS = ./bin/main

ifeq (run, $(filter run,$(MAKECMDGOALS)))
.PHONY: $(RULES)
$(RULES):
	@./bin/$@
endif

# Inform which objects are used by each binary
./bin/main: $(OBJS)

# Create binary out of objects
$(BINARIES):
	$(CXX) $^ -o $@ $(CXX_FLAGS)

.PHONY: run
run:
	@echo >/dev/null

# Convert cpp to obj files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp 
	mkdir -p $(@D)
	$(CXX) $< -c $(CXX_FLAGS) -o $@

# Make obj be recompiled after .hpp changed (include all .d files)
ifeq (,$(filter clean,$(MAKECMDGOALS)))

include $(DEPS)

endif
# Rule to create/update .d files
$(DEP_DIR)/%.d: $(SRC_DIR)/%.cpp
	mkdir -p $(@D)
#	Gets all includes of a .cpp file
	$(CXX) $(LD_FLAGS) -MM -MT '$@ $(OBJ_DIR)/$*.o' $< > $@
	

# Delete output subfolders
.PHONY: clean
clean: 
	rm -rf $(OBJ_DIR) $(DEP_DIR) $(BIN_DIR)