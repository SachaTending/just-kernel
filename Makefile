CFLAGS = -I include -m32 -march=i386 -c -I include -fpermissive -g
LDFLAGS = -A i386 -melf_i386 -T link.ld -Map=kernmap.map
AS=NASM
ASFLAGS=-felf32
KERNEL=kernel.bin
ARCH=i386

FILES = kernel/boot.o kernel/main.o kernel/constructor_test.o \
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


link: $(FILES)
	@echo LD $(KERNEL)
	@$(LD) $(LDFLAGS) -o $(KERNEL) $(FILES)
	@cat kernmap.map | egrep '*0x00000000.*' | egrep -v '\.' | tr -s ' ' | sed -e 's/0x00000000//' > kernmap.small

clean:
	@-rm $(FILES)

run:
	@qemu-system-$(ARCH) -hda ext2.img -kernel $(KERNEL) -rtc base=localtime -device sb16,audiodev=a -audiodev sdl,id=a -display sdl -initrd phonk"("DO_NOT_TOUCH_FOR_TESTING")".wav