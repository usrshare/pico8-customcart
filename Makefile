OBJDIR=obj
SRCDIR=src

CC=gcc
LD=ld
CFLAGS=-std=c11
LDSTATIC = $(shell libpng-config --ldflags )

_OBJS = main.o

ifdef RELEASE
DEFS += -xSSE3 -O3 -DNDEBUG
else
DEFS += -g
endif

OBJS = $(patsubst %,$(OBJDIR)/%,$(_OBJS))

p8png: $(OBJS)
	$(CC) -o $@ $^ $(LDSTATIC)

$(OBJDIR)/%.o: $(SRCDIR)/%.c $(OBJDIR)
	$(CC) -c -o $@ $< $(CFLAGS) $(DEFS)

$(OBJDIR):
	mkdir $(OBJDIR)

clean:
	rm -rf $(OBJDIR)/*.o p8png
