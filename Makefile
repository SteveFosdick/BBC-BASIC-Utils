CC	= gcc
CFLAGS	= -O2 -Wall

PROGS = bas2txt comal2txt txt2bas basdata_test

all: $(PROGS) basprt basread baswrit libbasdata.a

%: %.bbc
	txt2bas $< $@

MODULES = basdata_fpr.o basdata_fpw.o basdata_oth.o basdata_var.o

$(MODULES): basdata.h

libbasdata.a: $(MODULES)
	ar rc libbasdata.a $(MODULES)

basdata_test: basdata_test.c libbasdata.a
	$(CC) $(CFLAGS) -L . -o basdata_test basdata_test.c -lbasdata -lm

clean:
	rm -f $(PROGS)

install: $(PROGS) libbasdata.a
	sudo install -b -m 0555 -s $(PROGS) /usr/local/bin
	sudo install -b -m 0444 libbasdata.a /usr/local/lib
