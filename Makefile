# Makefile for mylib with dependencies on Jansson, libcbor, and libyang

# Compiler
CC = gcc

# Compiler flags
CFLAGS = -Wall -Wextra -pedantic -std=c11 -Werror

# Library name
LIB_NAME = ccoreconf

# Source directory
SRC_DIR = src
HEADER_DIR = include

# Object directory
OBJ_DIR = obj

# External libraries
JANSSON_LIB = -ljansson
LIBCBOR_LIB = -lcbor
LIBYANG_LIB = -lyang

# Include directories for external libraries
INCLUDE_DIRS =   -I/home/valentina/projects/lpwan_examples/build_libcbor/install/include/ -I/home/valentina/projects/lpwan_examples/build_libyang/install/include/ -I/home/valentina/projects/lpwan_examples/build_libjansson/install/include/

# Library directories for external libraries
LIB_DIRS =  -L/home/valentina/projects/lpwan_examples/build_libcbor/install/lib/ -L/home/valentina/projects/lpwan_examples/build_libyang/install/lib/ -L/home/valentina/projects/lpwan_examples/build_libjansson/install/lib/

# Source files
SRC_FILES = $(wildcard $(SRC_DIR)/*.c)

# Object files
OBJ_FILES = $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRC_FILES))

# Header files
HEADER_FILES = $(wildcard $(HEADER_DIR)/*.h)

# Build rule for the library
LIBS+= -ljansson -lcbor -lyang
$(LIB_NAME).a: $(OBJ_FILES)
	ar rcs $@ $^ 

EXEC_NAME = example
EXEC_OUTPUT = examples
# Build rule for exec
$(EXEC_NAME): $(OBJ_FILES) examples/demo_functionalities.c
	$(CC) $(CFLAGS) $(INCLUDE_DIRS) -o $@ $^ $(LIB_DIRS) $(JANSSON_LIB) $(LIBCBOR_LIB) $(LIBYANG_LIB)


# Build rule for object files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c $(HEADER_FILES)
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $(INCLUDE_DIRS) -c $< -o $@


# Clean rule
clean:
	rm -rf $(LIB_NAME).a $(OBJ_DIR)


# Phony target to prevent conflicts with files of the same name
.PHONY: clean

