FILES += img.o

img.o:
	@echo OBJCOPY img.o
	@objcopy -O elf32-i386 -B i386 -I binary img.tga img.o