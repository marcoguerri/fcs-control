SOURCES := $(shell find . -maxdepth 1 -name '*.c')
OBJECTS := $(SOURCES:.c=.o)

LIBCRC_DIR = libcrc
OBJECTS += $(LIBCRC_DIR)/libcrc.a

CFLAGS = -Wall -O0 -fPIC -D_GNU_SOURCE
LDLIBS = -ldl -lcrc
LDFLAGS = -Llibcrc

all: corrupt

corrupt: $(OBJECTS) 
	$(CC) $(OBJECTS) -o $@ $(LDIBS) $(FLAGS)

$(LIBCRC_DIR)/libcrc.a: 
	make -C $(LIBCRC_DIR) libcrc.a 

%.o: %.cpp
	$(CC) $(CFLAGS) -o $@

clean:
	make -C $(LIBCRC_DIR) $@
	@rm -f $(OBJECTS)
	rm corrupt

.PHONY: clean
