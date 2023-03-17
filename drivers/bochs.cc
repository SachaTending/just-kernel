#include "printf.h"
#include "stdint.h"
#include "module.h"
#include "port_io.h"
#include "pit.h"

#define PREFERRED_VY 4096
#define PREFERRED_B 32

uint16_t lfb_resolution_x = 0;
uint16_t lfb_resolution_y = 0;
uint16_t lfb_resolution_b = 0;



static void finalize_graphics(uint16_t x, uint16_t y, uint16_t b) {
	lfb_resolution_x = x;
	lfb_resolution_y = y;
	lfb_resolution_b = b;
}

uint8_t * lfb_vid_memory = (uint8_t *)0xE0000000;

void graphics_install_bochs(uint16_t resolution_x, uint16_t resolution_y);

uintptr_t lfb_get_address() {
	return (uintptr_t)lfb_vid_memory;
}


#pragma GCC diagnostic ignored "-Wconversion"
#pragma GCC diagnostic ignored "-Wunused-function"

#define VBE_DISPI_IOPORT_INDEX 0x01CE
#define VBE_DISPI_IOPORT_DATA 0x01CF
#define VBE_DISPI_INDEX_ID 0x0
#define VBE_DISPI_INDEX_XRES 0x1
#define VBE_DISPI_INDEX_YRES 0x2
#define VBE_DISPI_INDEX_BPP 0x3
#define VBE_DISPI_INDEX_ENABLE 0x4
#define VBE_DISPI_INDEX_BANK 0x5
#define VBE_DISPI_INDEX_VIRT_WIDTH 0x6
#define VBE_DISPI_INDEX_VIRT_HEIGHT 0x7
#define VBE_DISPI_INDEX_X_OFFSET 0x8
#define VBE_DISPI_INDEX_Y_OFFSET 0x9

#define VBE_DISPI_DISABLED 0x00
#define VBE_DISPI_ENABLED 0x01
#define VBE_DISPI_GETCAPS 0x02
#define VBE_DISPI_8BIT_DAC 0x20
#define VBE_DISPI_LFB_ENABLED 0x40
#define VBE_DISPI_NOCLEARMEM 0x80

void vbe_write(u16 index, u16 value) {
  outportw(VBE_DISPI_IOPORT_INDEX, index);
  outportw(VBE_DISPI_IOPORT_DATA, value);
}

u32 vbe_read(u16 index) {
  outportw(VBE_DISPI_IOPORT_INDEX, index);
  return inw(VBE_DISPI_IOPORT_DATA);
}

void vbe_set(u16 xres, u16 yres, u16 bpp) {
  vbe_write(VBE_DISPI_INDEX_ENABLE, VBE_DISPI_DISABLED);
  vbe_write(VBE_DISPI_INDEX_XRES, xres);
  vbe_write(VBE_DISPI_INDEX_YRES, yres);
  vbe_write(VBE_DISPI_INDEX_BPP, bpp);
  vbe_write(VBE_DISPI_INDEX_ENABLE, VBE_DISPI_ENABLED | VBE_DISPI_LFB_ENABLED);
}

void video_set_resolution(u16 xres, u16 yres, u8 depth) {
  vbe_set(xres, yres, depth);
}

/* only valid for 800x600x32bpp */
void putpixel(unsigned char *screen, u8 x, u8 y, int color) {
  u8 where = x * 800 + y * 1;
  screen[where] = color & 255;             // BLUE
  screen[where + 1] = (color >> 8) & 255;  // GREEN
  screen[where + 2] = (color >> 16) & 255; // RED
}

void bochs_setup() {
    //graphics_install_bochs(640, 480);
    vbe_set(800, 600, 32);
    for (uintptr_t fb_offset = 0xE0000000; fb_offset < 0xFF000000; fb_offset += 0x01000000) {

		/* Go find it */
		for (uintptr_t x = fb_offset; x < fb_offset + 0xFF0000; x += 0x1000) {
			if (((uintptr_t *)x)[0] == 0xA5ADFACE) {
				lfb_vid_memory = (uint8_t *)x;
                printf("founded vid mem.\n");
				goto mem_found;
			}
		}

	}

    mem_found:
        finalize_graphics(800, 600, 32);
}

void graphics_install_bochs(uint16_t resolution_x, uint16_t resolution_y) 
{
	printf("Trying to setup up bochs graphics controller...");
	outports(0x1CE, 0x00);
	uint16_t i = inports(0x1CF);
	if (i < 0xB0C0 || i > 0xB0C6) {
        printf("Error.\n");
		return;
	}
	outports(0x1CF, 0xB0C4);
	i = inports(0x1CF);
	/* Disable VBE */
	outports(0x1CE, 0x04);
	outports(0x1CF, 0x00);
	/* Set X resolution to 1024 */
	outports(0x1CE, 0x01);
	outports(0x1CF, resolution_x);
	/* Set Y resolution to 768 */
	outports(0x1CE, 0x02);
	outports(0x1CF, resolution_y);
	/* Set bpp to 32 */
	outports(0x1CE, 0x03);
	outports(0x1CF, PREFERRED_B);
	/* Set Virtual Height to stuff */
	outports(0x1CE, 0x07);
	outports(0x1CF, PREFERRED_VY);
	/* Re-enable VBE */
    outports(0x1CE, 0x04);
	outports(0x1CF, 0x41);

	/* XXX: Massive hack */
	uint32_t * text_vid_mem = (uint32_t *)0xA0000;
	text_vid_mem[0] = 0xA5ADFACE;

	for (uintptr_t fb_offset = 0xE0000000; fb_offset < 0xFF000000; fb_offset += 0x01000000) {

		/* Go find it */
		for (uintptr_t x = fb_offset; x < fb_offset + 0xFF0000; x += 0x1000) {
			if (((uintptr_t *)x)[0] == 0xA5ADFACE) {
				lfb_vid_memory = (uint8_t *)x;
				goto mem_found;
			}
		}

	}

mem_found:
	finalize_graphics(resolution_x, resolution_y, PREFERRED_B);
}