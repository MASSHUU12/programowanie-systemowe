CC = gcc
CFLAGS = -Wall -Werror
LDFLAGS = -ldl

MAIN_SRC = main.c
LIB_SRC = usergroups.c

MAIN_OBJ = $(MAIN_SRC:.c=.o)
LIB_OBJ = $(LIB_SRC:.c=.o)

TARGET = loggedusers
SHARED_LIB = libusergroups.so

all: $(TARGET) $(SHARED_LIB)

$(SHARED_LIB): $(LIB_OBJ)
	$(CC) -shared -o $@ $^

$(LIB_OBJ): $(LIB_SRC)
	$(CC) -fPIC $(CFLAGS) -c $< -o $@

# Compile main.c to object file
$(MAIN_OBJ): $(MAIN_SRC)
	$(CC) $(CFLAGS) -c $< -o $@

# Link the final executable
$(TARGET): $(MAIN_OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# Clean up build artifacts
clean:
	rm -f $(MAIN_OBJ) $(LIB_OBJ) $(SHARED_LIB) $(TARGET)

debug: CFLAGS += -g
debug: clean all

.PHONY: all clean debug
