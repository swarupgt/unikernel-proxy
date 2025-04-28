# compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra $(INCLUDES) -MMD -MP -fPIC -pie 

# directories
BUILD_DIR = .build
SRC_DIR = src
INCLUDES = -I$(SRC_DIR) -I$(SRC_DIR)/utils
TEST_DIR = tests

# project
PROJECT_DEPS = $(SRC_DIR)

# sources
PROJECT_SOURCES = $(shell find $(PROJECT_DEPS) -type f -name "*.c")

TEST_COMMON_DEP_SOURCES = $(shell find $(PROJECT_DEPS) -type f -name "*.c"  ! -name main.c )

TEST_FILES = $(shell find tests -type f -name "*.c")

# target
PROJECT_TARGET = http_server

# objects 
# PROJECT_OBJECTS = $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(PROJECT_SOURCES))

all: clean $(PROJECT_TARGET)

$(PROJECT_TARGET): $(PROJECT_SOURCES)
	$(CC) $(CFLAGS) -o $@ $^


compile_tests: $(TEST_SOURCES)
	@for target in $(TEST_FILES); do \
		target_obj=$$(echo $$target | sed 's/\.c$$/.out/'); \
		$(CC) $(CFLAGS) -o $$target_obj $$target $(TEST_COMMON_DEP_SOURCES); \
	done

test: compile_tests
	@ echo "---------RUNNING TESTS---------" && \
	./run_tests.sh && \
	echo "---------FINISHED TESTS---------"

clean:
	@rm -rf *.d src/*.d src/*.out tests/*.d tests/*.out .build http_server
