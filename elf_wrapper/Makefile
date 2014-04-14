#
# $Id: Makefile,v 1.18 2004/11/01 16:22:03 mkirchner Exp $
#

# CCFLAGS=-MD
CC=gcc

all: elf_wrapper

elf_wrapper: elf_wrapper.o
	$(CC) $(LDFLAGS) elf_wrapper.o -o elf_wrapper

elf_wrapper.o:
	$(CC) $(CCFLAGS) -c elf_wrapper.c



clean:
	rm -f elf_wrapper *.o *.d

