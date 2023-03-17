FILES += img.o

default: build

img.o:
	@echo GEN $< to $@
	@objcopy -O elf32-i386 -B i386 -I binary img.tga $@