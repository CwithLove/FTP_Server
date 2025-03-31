.PHONY: all clean

# Disable implicit rules
.SUFFIXES:

# Keep intermediate files
#.PRECIOUS: %.o

CC = gcc
CFLAGS = -Wall
LDFLAGS =
LIBS += -lpthread

# Directories
SERVERDIR = serveur
SLAVESDIR = $(SERVERDIR)/slaves
MASTERDIR = $(SERVERDIR)/master
CLIENTDIR = client
LIBDIR = libs
INCLDIR = -I$(LIBDIR)

# Source and object files
SLAVESSRC = $(wildcard $(SLAVESDIR)/*.c)
MASTERSRC = $(wildcard $(MASTERDIR)/*.c)
CLIENTSRC = $(wildcard $(CLIENTDIR)/*.c)
LIBSRC = $(wildcard $(LIBDIR)/*.c)

SLAVESOBJ = $(patsubst $(SLAVESDIR)/%.c,$(SLAVESDIR)/%.o,$(SLAVESSRC))
MASTEROBJ = $(patsubst $(MASTERDIR)/%.c,$(MASTERDIR)/%.o,$(MASTERSRC))
CLIENTOBJ = $(patsubst $(CLIENTDIR)/%.c,$(CLIENTDIR)/%.o,$(CLIENTSRC))
LIBOBJ = $(patsubst $(LIBDIR)/%.c,$(LIBDIR)/%.o,$(LIBSRC))

all: storage ftpclient ftpmaster ftpslave
 
storage:
	mkdir -p $(SERVERDIR)/storage
	mkdir -p $(SLAVESDIR)/storage

%.o: %.c $(INCLUDE)
	$(CC) $(CFLAGS) $(CPPFLAGS) $(INCLDIR) -c -o $@ $<

ftpclient: $(LIBOBJ) $(CLIENTOBJ)
	$(CC) $^ $(LIBS) -o $(CLIENTDIR)/$@ $(LDFLAGS)

ftpmaster: $(LIBOBJ) $(MASTEROBJ)
	$(CC) $^ $(LIBS) -o $(MASTERDIR)/$@ $(LDFLAGS)

ftpslave: $(LIBOBJ) $(SLAVESOBJ)
	$(CC) $^ $(LIBS) -o $(SLAVESDIR)/$@ $(LDFLAGS)

clean:
	rm -f $(LIBOBJ) $(MASTEROBJ) $(SLAVESOBJ) $(CLIENTOBJ) $(MASTERDIR)/ftpmaster $(SLAVESDIR)/ftpslave  $(CLIENTDIR)/ftpclient $(CLIENTDIR)/storage/*
