CC = gcc
CFLAGS = -Wall -Werror
AR = ar
ARFLAGS = rcs

SRCS = usergroups.c main.c
OBJS = $(SRCS:.c=.o)
LIB = libusergroups.a
TARGET = loggedusers

all: $(TARGET)

# Build the static library
$(LIB): usergroups.o
	$(AR) $(ARFLAGS) $@ $^

# Compile source files into object files
%.o: %.c usergroups.h
	$(CC) $(CFLAGS) -c $< -o $@

# Link the final executable
$(TARGET): $(OBJS) $(LIB)
	$(CC) $(CFLAGS) -o $@ $(OBJS) -L. -lusergroups

# Clean up build artifacts
clean:
	rm -f $(OBJS) $(LIB) $(TARGET)

debug: CFLAGS += -g
debug: clean all

# Dependency tracking
%.o: %.c
	$(CC) $(CFLAGS) -MMD -c $< -o $@

-include $(OBJS:.o=.d)

.PHONY: all clean debug
