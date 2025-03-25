.PHONY: all clean

# Disable implicit rules
.SUFFIXES:

# Keep intermediate files
#.PRECIOUS: %.o

CC = gcc
CFLAGS = -Wall -Werror
LDFLAGS =
LIBS += -lpthread

# Directories
SERVERDIR = serveur
CLIENTDIR = client
LIBDIR = libs
INCLDIR = -I$(LIBDIR)

# Source and object files
SERVERSRC = $(wildcard $(SERVERDIR)/*.c)
CLIENTSRC = $(wildcard $(CLIENTDIR)/*.c)
LIBSRC = $(wildcard $(LIBDIR)/*.c)
LIBOBJ = $(patsubst $(LIBDIR)/%.c,$(LIBDIR)/%.o,$(LIBSRC))
SERVOBJ = $(patsubst $(SERVERDIR)/%.c,$(SERVERDIR)/%.o,$(SERVERSRC))
CLIOBJ = $(patsubst $(CLIENTDIR)/%.c,$(CLIENTDIR)/%.o,$(CLIENTSRC))

all: ftpserver ftpclient

%.o: %.c $(INCLUDE)
	$(CC) $(CFLAGS) $(CPPFLAGS) $(INCLDIR) -c -o $@ $<

ftpclient: $(LIBOBJ) $(CLIOBJ)
	$(CC) $^ $(LIBS) -o $(CLIENTDIR)/$@ $(LDFLAGS)

ftpserver: $(LIBOBJ) $(SERVOBJ)
	$(CC) $^ $(LIBS) -o $(SERVERDIR)/$@ $(LDFLAGS)

clean:
	rm -f $(LIBOBJ) $(SERVOBJ) $(CLIOBJ) $(SERVERDIR)/ftpserver $(CLIENTDIR)/ftpclient
