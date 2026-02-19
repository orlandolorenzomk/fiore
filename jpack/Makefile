CC       = cc
INCDIR   = include
BINDIR   = bin
BUILDDIR = build
TARGET   = $(BINDIR)/jpack

CFLAGS  += -Wall -Wextra -I$(INCDIR) -I/usr/local/include -O2
LDFLAGS += -L/usr/local/lib -lcurl

SRCS    != find src -maxdepth 1 -name '*.c' 2>/dev/null | sort
SRCS    += main.c

OBJS    != echo $(SRCS) | tr ' ' '\n' | \
           sed 's|src/\(.*\)\.c|$(BUILDDIR)/src/\1.o|; s|^main\.c|$(BUILDDIR)/main.o|' | \
           tr '\n' ' '

.PHONY: all clean dirs install

all: dirs $(TARGET)

dirs:
	@mkdir -p $(BINDIR) $(BUILDDIR)/src

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS) $(LDFLAGS)

.for _src in $(SRCS)
$(BUILDDIR)/$(_src:S/.c$/.o/): $(_src)
	$(CC) $(CFLAGS) -c $(_src) -o $(BUILDDIR)/$(_src:S/.c$/.o/)
.endfor

clean:
	rm -rf $(BUILDDIR) $(TARGET)

install: all
	install -m 0755 $(TARGET) /usr/local/bin/jpack
