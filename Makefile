NAME    := xine-server-x
VERSION := 0.1
CC      :=  gcc 
LIBS    := -ldl -lpthread -lmicrohttpd ${EXTRA_LIBS} 
TARGET	:= $(NAME)
SOURCES := $(shell find src/ -type f -name *.c)
OBJECTS := $(patsubst src/%,build/%,$(SOURCES:.c=.o))
DEPS	:= $(OBJECTS:.o=.deps)
DOCS    := $(shell find docroot/ -type f)
COMMA   := ,
BINS    := $(patsubst %,-Wl$(COMMA)%,$(DOCS))
DESTDIR := /
PREFIX  := /usr
MANDIR  := $(DESTDIR)/$(PREFIX)/share/man
BINDIR  := $(DESTDIR)/$(PREFIX)/bin
SHAREBASE := $(DESTDIR)/$(PREFIX)/share
GXSRADIO  := $(SHAREBASE)/gxsradio/
SHARE   := $(SHAREBASE)/$(TARGET)
CFLAGS  := -g -fpie -fpic -Wall -DNAME=\"$(NAME)\" -DVERSION=\"$(VERSION)\" -DSHARE=\"$(SHARE)\" -DGXSRADIO=\"$(GXSRADIO)\" -DPREFIX=\"$(PREFIX)\" -DBUILD_DATETIME=\"$(shell date +%s)\" -I include ${EXTRA_CFLAGS}
LDFLAGS := -pie ${EXTRA_LDFLAGS}

all: $(TARGET)
debug: CFLAGS += -g
debug: $(TARGET) 

$(TARGET): $(OBJECTS) 
	$(CC) $(LDFLAGS) -o $(TARGET) \
	-Wl,--format=binary $(BINS) \
	-Wl,--format=default $(OBJECTS) $(LIBS) 

build/%.o: src/%.c
	@mkdir -p build/
	$(CC) $(CFLAGS) -MD -MF $(@:.o=.deps) -c -o $@ $<

clean:
	@echo "  Cleaning..."; $(RM) -r build/ $(TARGET) 

install: $(TARGET)
	mkdir -p $(DESTDIR)/$(PREFIX) $(DESTDIR)/$(BINDIR) $(DESTDIR)/$(MANDIR)
	strip $(TARGET)
	install -m 755 $(TARGET) $(DESTDIR)/${BINDIR}
	mkdir -p $(DESTDIR)/$(MANDIR)/man1
	cp -p man1/* $(DESTDIR)/${MANDIR}/man1/

-include $(DEPS)

.PHONY: clean

