# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -O2
DEBUG_FLAGS = -Wall -Wextra -g -DDEBUG

# Targets
TARGET = aac
GRAPHGEN = graphgen

# Source files
SRCS = main.c graph.c subiso.c gmext.c exact_extension.c
GRAPHGEN_SRCS = graphgen.c

# Object files
OBJS = $(SRCS:.c=.o)
GRAPHGEN_OBJS = $(GRAPHGEN_SRCS:.c=.o)

# Header files
HEADERS = graph.h subiso.h gmext.h exact_extension.h

# Default target
all: $(TARGET) $(GRAPHGEN)

# Main executable
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS)

# Graph generator executable
$(GRAPHGEN): $(GRAPHGEN_OBJS)
	$(CC) $(CFLAGS) -o $@ $(GRAPHGEN_OBJS)

# Compile source files to object files
%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

# Debug build
debug: CFLAGS = $(DEBUG_FLAGS)
debug: clean $(TARGET)

# Clean build files
clean:
	del /Q *.o $(TARGET).exe $(GRAPHGEN).exe 2>nul || true

# Help
help:
	@echo Available targets:
	@echo   all       - Build aac and graphgen (default)
	@echo   aac       - Build main program only
	@echo   graphgen  - Build graph generator only
	@echo   debug     - Build with debug symbols
	@echo   clean     - Remove build files
	@echo   help      - Show this help message

.PHONY: all clean debug help
