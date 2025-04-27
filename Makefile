# compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra $(INCLUDES) -MMD -MP -fPIC -pie 

# directories
BUILD_DIR = .build
SRC_DIR = src
INCLUDES = -I$(SRC_DIR)

# project
PROJECT_DEPS = $(SRC_DIR)

# sources
PROJECT_SOURCES = $(shell find $(PROJECT_DEPS) -type f -name "*.c")

# target
PROJECT_TARGET = http_server

# objects 
# PROJECT_OBJECTS = $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(PROJECT_SOURCES))

all: clean $(PROJECT_TARGET)

$(PROJECT_TARGET): $(PROJECT_SOURCES)
	$(CC) $(CFLAGS) -o $@ $^


# $(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
# 	@mkdir -p $(dir $@)
# 	$(CC) $(CFLAGS) -c -o $@ $< 


clean:
	@rm -rf .build http_server
