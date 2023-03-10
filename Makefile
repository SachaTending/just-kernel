CFLAGS = -I include -m32 -march=i386 -c -I include -fpermissive -g
LDFLAGS = -A i386 -melf_i386 -T link.ld -Map=kernmap.map
AS=NASM
ASFLAGS=-felf32 -g
KERNEL=kernel.bin
ARCH=i386

FILES = kernel/boot.o kernel/main.o kernel/constructor_test.o \
	kernel/ap_trampoline.o \
	common/tty.o common/string.o common/port_io.o common/printf.o \
	common/math.o common/test_stuff.o

-include targets/*.mk

build: $(FILES) link

.cc.o:
	@echo C++ $@
	@g++ $(CFLAGS) -o $@ $<

.s.o:
	@echo NASM $@
	@nasm $(ASFLAGS) -o $@ $<

.S.o:
	@echo GAS $@
	@gcc $(CFLAGS) -o $@ $<

link: $(FILES)
	@echo LD $(KERNEL)
	@$(LD) $(LDFLAGS) -o $(KERNEL) $(FILES)

clean:
	@-rm $(FILES)

run: build
	@qemu-system-$(ARCH) -hda ext2.img -kernel $(KERNEL) -rtc base=localtime -device sb16,audiodev=a -audiodev sdl,id=a -serial stdio -smp cpus=2,cores=2 -device piix3-usb-uhci,id=usb-bus0 -device usb-audio,bus=usb-bus0