# Makefile for mylib with dependencies on Jansson, libcbor, and libyang

# Compiler
CC = gcc

# Compiler flags
CFLAGS = -Wall -Wextra -pedantic -std=c11 -Werror -Werror=strict-prototypes -Werror=format -Werror=old-style-definition -Werror=cast-align

# Library name
LIB_NAME = ccoreconf

# Source directory
SRC_DIR = src
HEADER_DIR = include

# Object directory
OBJ_DIR = obj

# External libraries
JANSSON_LIB = -ljansson
LIBNANOCBOR_LIB = -lnanocbor

# Include directories for external libraries
INCLUDE_DIRS =  -I$(NANOCBOR_INCLUDE) -I$(LIBJANSSON_INCLUDE)

# Library directories for external libraries
LIB_DIRS =  -L$(NANOCBOR_BUILD) -L$(LIBJANSSON_BUILD)


# Source files
SRC_FILES = $(wildcard $(SRC_DIR)/*.c)

# Object files
OBJ_FILES = $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRC_FILES))

# Header files
HEADER_FILES = $(wildcard $(HEADER_DIR)/*.h)

# Build rule for the library
$(LIB_NAME).a: $(OBJ_FILES)
	ar rcs $@ $^ 

EXEC_NAME = example
EXEC_OUTPUT = examples
# Build rule for exec
$(EXEC_NAME): $(OBJ_FILES) examples/demo_functionalities_coreconf.c
	$(CC) $(CFLAGS) $(INCLUDE_DIRS) -o $@ $^ $(LIB_DIRS) $(JANSSON_LIB) $(LIBNANOCBOR_LIB) 


# Build rule for object files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c $(HEADER_FILES)
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $(INCLUDE_DIRS) -c $< -o $@


# Clean rule
clean:
	rm -rf $(LIB_NAME).a $(OBJ_DIR)


# Phony target to prevent conflicts with files of the same name
.PHONY: clean

