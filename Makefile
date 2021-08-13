CC = gcc
CPPFLAGS = -MMD
CFLAGS = -Wall -Wextra -std=c99 -ggdb
LDFLAGS =
LDLIBS = 

SRC_DIR = ./src
BUILD_DIR = ./build
TEST_DIR = $(BUILD_DIR)/test
PROF_DIR = $(BUILD_DIR)/prof

TARGET = sloc
TEST_TARGET = test_$(TARGET)
PROF_TARGET = prof_$(TARGET)

SRC = $(wildcard $(SRC_DIR)/*.c)

OBJ = $(SRC:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)
DEP = $(OBJ:%.o=%.d)

TEST_OBJ = $(SRC:$(SRC_DIR)/%.c=$(TEST_DIR)/%.o)
TEST_DEP = $(TEST_OBJ:%.o=%.d)

PROF_OBJ = $(SRC:$(SRC_DIR)/%.c=$(PROF_DIR)/%.o)
PROF_DEP = $(PROF_OBJ:%.o=%.d)

.PHONY: F5 F6 F7 F8

F5: all
F6: run
F7: test
F8: debug

.PHONY: all profile cache run test debug clean

all: $(BUILD_DIR)/$(TARGET)

# FILES=`find src -name "*.c" -printf '%p '` make profile
FILES ?= $(shell find $(SRC_DIR) -type f -name '*.c')

profile: CFLAGS += -pg
profile: LDFLAGS += -pg
profile: $(PROF_DIR)/$(PROF_TARGET)
	$(PROF_DIR)/$(PROF_TARGET) $(FILES)
	gprof $(PROF_DIR)/$(PROF_TARGET) > gprof.out

cache: all
	valgrind --tool=cachegrind --cachegrind-out-file=cachegrind.out $(BUILD_DIR)/$(TARGET) $(FILES)
	cg_annotate cachegrind.out | tee cg_annotate.out

run: all
	$(BUILD_DIR)/$(TARGET) $(FILES)

test: CPPFLAGS += -DTEST
test: $(TEST_DIR)/$(TEST_TARGET)
	$(TEST_DIR)/$(TEST_TARGET)

debug: all
	gdb $(BUILD_DIR)/$(TARGET)

clean:
	rm -rf $(BUILD_DIR) *.out.*

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	mkdir -p $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c -o $@ $<

$(BUILD_DIR)/$(TARGET): $(OBJ)
	mkdir -p $(BUILD_DIR)
	$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)

-include $(DEP)

# ==== TEST ====

$(TEST_DIR)/%.o: $(SRC_DIR)/%.c
	mkdir -p $(TEST_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c -o $@ $<

$(TEST_DIR)/$(TEST_TARGET): $(TEST_OBJ)
	mkdir -p $(TEST_DIR)
	$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)

-include $(TEST_DEP)

# ==== PROF ====

$(PROF_DIR)/%.o: $(SRC_DIR)/%.c
	mkdir -p $(PROF_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c -o $@ $<

$(PROF_DIR)/$(PROF_TARGET): $(PROF_OBJ)
	mkdir -p $(PROF_DIR)
	$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)

-include $(PROF_DEP)
