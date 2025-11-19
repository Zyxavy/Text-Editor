CC = gcc
CFLAGS = -Wall -Wextra -pedantic -std=c99 -Iinclude

SRC_DIR = src
CORE_DIR = $(SRC_DIR)/core
OP_DIR = $(SRC_DIR)/operations
BUILD_DIR = build

TARGET = $(BUILD_DIR)/kilo

SRCS = $(wildcard $(CORE_DIR)/*.c) \
       $(wildcard $(OP_DIR)/*.c)

OBJS = $(SRCS:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(TARGET)
	@echo "Build complete → $(TARGET)"

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@) 
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR)
	mkdir $(BUILD_DIR)

.PHONY: all clean
