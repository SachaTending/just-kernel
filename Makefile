CFLAGS = -I include -m32 -march=i386 -c
LDFLAGS = -A i386 -melf_i386 -T link.ld
AS=NASM
ASFLAGS=-felf32

FILES = kernel/boot.o kernel/main.o


build: $(FILES) link

.cc.o:
	@echo C++ $@
	@g++ $(CFLAGS) -o $@ $<

.s.o:
	@echo NASM $@
	@nasm $(ASFLAGS) -o $@ $<


link: $(FILES)
	@echo LD kernel.bin
	@$(LD) $(LDFLAGS) -o kernel.bin $(FILES)

clean:
	@rm $(FILES)