# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -g

# Executable name
TARGET = huntSimulation

# Source and object files
SRCS = main.c helpers.c
OBJS = $(SRCS:.c=.o)

# Build the final executable (only relinks if .o files changed)
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS) -lpthread

# Compile .c → .o (only rebuilds if the .c was modified)
%.o: %.c defs.h helpers.h
	$(CC) $(CFLAGS) -c $< -o $@

# Remove build outputs
clean:
	rm -f $(OBJS) $(TARGET)