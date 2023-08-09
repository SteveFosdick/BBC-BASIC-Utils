CC	= gcc
CFLAGS	= -O2 -Wall

PROGS = bas2txt comal2txt txt2bas

all: $(PROGS) libbasdata.a

MODULES = basdata_fpr.o basdata_fpw.o basdata_oth.o basdata_var.o

$(MODULES): basdata.h

libbasdata.a: $(MODULES)
	ar rc libbasdata.a $(MODULES)

clean:
	rm -f $(PROGS)

install: $(PROGS) libbasdata.a
	sudo install -b -m 0555 -s $(PROGS) /usr/local/bin
	sudo install -b -m 0444 libbasdata.a /usr/local/lib
