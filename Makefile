CFLAGS = -I include -m32 -march=i386 -c -I include
LDFLAGS = -A i386 -melf_i386 -T link.ld
AS=NASM
ASFLAGS=-felf32
KERNEL=kernel.bin
ARCH=i386

FILES = kernel/boot.o kernel/main.o kernel/constructor_test.o \
	common/tty.o common/string.o common/port_io.o

-include targets/*.mk

build: $(FILES) link

.cc.o:
	@echo C++ $@
	@g++ $(CFLAGS) -o $@ $<

.s.o:
	@echo NASM $@
	@nasm $(ASFLAGS) -o $@ $<


link: $(FILES)
	@echo LD $(KERNEL)
	@$(LD) $(LDFLAGS) -o $(KERNEL) $(FILES)

clean:
	@-rm $(FILES)

run:
	@qemu-system-$(ARCH) -kernel $(KERNEL)