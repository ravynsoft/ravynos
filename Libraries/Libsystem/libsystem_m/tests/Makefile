CLANG ?= clang
CFLAGS ?= -O2 -g
CFLAGS += -std=c11 -Wall -Wextra -Werror -Wno-unused-parameter \
	-fno-builtin -frounding-math -ftrapping-math
LDLIBS += -lm

ROOT = ..
BUILD_DIR ?= build

.PHONY: all check clean

all: $(BUILD_DIR)/fmod_test

$(BUILD_DIR):
	mkdir -p $@

$(BUILD_DIR)/math.o: $(ROOT)/math.c host_compat.h | $(BUILD_DIR)
	$(CLANG) $(CFLAGS) -include host_compat.h -c $(ROOT)/math.c -o $@

$(BUILD_DIR)/fmod_test.o: fmod_test.c | $(BUILD_DIR)
	$(CLANG) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/fmod_test: $(BUILD_DIR)/math.o $(BUILD_DIR)/fmod_test.o
	$(CLANG) $(CFLAGS) $^ $(LDLIBS) -o $@

check: all
	./$(BUILD_DIR)/fmod_test

clean:
	rm -rf $(BUILD_DIR)
