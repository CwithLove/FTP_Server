.PHONY: all clean

# Disable implicit rules
.SUFFIXES:

# Compiler settings
CC = gcc
CFLAGS = -Wall -Werror
LDFLAGS = -lpthread

# Directory structure
CLIENTDIR = client
SERVERDIR = server
LIBSDIR = libs
INCLDIR = -I./$(LIBSDIR)

# File definitions
PROGS = $(CLIENTDIR)/client $(SERVERDIR)/server
INCLUDES = $(LIBSDIR)/csapp.h
OBJS = $(LIBSDIR)/csapp.o $(LIBSDIR)/echo.o

# Default target
all: $(PROGS)

# Pattern rules
%.o: %.c $(INCLUDES)
	@echo "Compiling $@"
	$(CC) $(CFLAGS) $(INCLDIR) -c $< -o $@

%: %.o $(OBJS)
	@echo "Linking $@"
	$(CC) $(CFLAGS) $^ $(OBJS) $(LDFLAGS) -o $@

# Clean target
clean:
	@echo "Cleaning..."
	rm -f $(PROGS) $(CLIENTDIR)/*.o $(SERVERDIR)/*.o $(LIBSDIR)/*.o