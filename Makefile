CC	= gcc
CFLAGS	= -O2 -Wall

PROGS = bas2txt comal2txt txt2bas

all: $(PROGS)

clean:
	rm -f $(PROGS)

install: $(PROGS)
	sudo install -b -m 0555 -s $(PROGS) /usr/local/bin
