CC ?= cc
CPPFLAGS += -Iheaders
CFLAGS ?= -O2 -g
CFLAGS += -std=gnu11 -Wall -Wextra
BUILD_DIR ?= tmp/build

SOURCES := $(wildcard sources/*.c)
OBJECTS := $(patsubst sources/%.c,$(BUILD_DIR)/%.o,$(SOURCES))
TARGET := $(BUILD_DIR)/RemarkableLamyEraser

.PHONY: all check compdb clean
all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@

$(BUILD_DIR)/%.o: sources/%.c | $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) -MMD -MP -c $< -o $@

$(BUILD_DIR):
	mkdir -p $@

compdb:
	bear -- $(MAKE) -B all

clean:
	rm -f $(TARGET) $(OBJECTS) $(OBJECTS:.o=.d) compile_commands.json

-include $(OBJECTS:.o=.d)
