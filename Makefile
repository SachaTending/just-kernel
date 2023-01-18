# Makefile for JamesM's kernel tutorials.
# The C and C++ rules are already setup by default.
# The only one that needs changing is the assembler 
# rule, as we use nasm instead of GNU as.

SOURCES=boot.o main.o monitor.o common.o descriptor_tables.o isr.o interrupt.o gdt.o timer.o \
        kheap.o paging.o ordered_array.o fs.o initrd.o task.o process.o syscall.o

CC=cc
CFLAGS= -I include/ -m32 -g -O2 -ffreestanding -march=i386 -nodefaultlibs 
LDFLAGS=-Tlink.ld -melf_i386 -A i386 /usr/lib/gcc/x86_64-pc-linux-gnu/12.2.0/32/libgcc.a
ASFLAGS=-felf32

all: $(SOURCES) link

clean:
	-rm $(SOURCES)

link:
	@ld $(LDFLAGS) -o kernel $(SOURCES)

.s.o:
	nasm $(ASFLAGS) $<
