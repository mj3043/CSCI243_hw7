# Makefile for hw7
# Use the course-provided header.mak for compiler flags
include header.mak

# Object files
OBJS = mcrypt.o KStream.o

# Default target
all: mcrypt

# Build mcrypt
mcrypt: $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o mcrypt $(CLIBFLAGS)

# Compile .c → .o
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Cleanup
clean:
	rm -f *.o mcrypt
