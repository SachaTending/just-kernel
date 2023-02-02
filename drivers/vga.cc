#include <printf.h>
#include <port_io.h>
#include <string.h>
#include <vga.h>


#define	VGA_AC_INDEX		0x3C0
#define	VGA_AC_WRITE		0x3C0
#define	VGA_AC_READ		0x3C1
#define	VGA_MISC_WRITE		0x3C2
#define VGA_SEQ_INDEX		0x3C4
#define VGA_SEQ_DATA		0x3C5
#define	VGA_DAC_READ_INDEX	0x3C7
#define	VGA_DAC_WRITE_INDEX	0x3C8
#define	VGA_DAC_DATA		0x3C9
#define	VGA_MISC_READ		0x3CC
#define VGA_GC_INDEX 		0x3CE
#define VGA_GC_DATA 		0x3CF
/*			COLOR emulation		MONO emulation */
#define VGA_CRTC_INDEX		0x3D4		/* 0x3B4 */
#define VGA_CRTC_DATA		0x3D5		/* 0x3B5 */
#define	VGA_INSTAT_READ		0x3DA

#define	VGA_NUM_SEQ_REGS	5
#define	VGA_NUM_CRTC_REGS	25
#define	VGA_NUM_GC_REGS		9
#define	VGA_NUM_AC_REGS		21

#define	peekb(S,O)		*(unsigned char *)(16uL * (S) + (O))
#define	pokeb(S,O,V)		*(unsigned char *)(16uL * (S) + (O)) = (V)
#define	pokew(S,O,V)		*(unsigned short *)(16uL * (S) + (O)) = (V)
#define	_vmemwr(DS,DO,S,N)	memcpy((char *)((DS) * 16 + (DO)), S, N)

void write_regs(unsigned char *regs)
{
	unsigned i;

/* write MISCELLANEOUS reg */
	outportb(VGA_MISC_WRITE, *regs);
	regs++;
/* write SEQUENCER regs */
	for(i = 0; i < VGA_NUM_SEQ_REGS; i++)
	{
		outportb(VGA_SEQ_INDEX, i);
		outportb(VGA_SEQ_DATA, *regs);
		regs++;
	}
/* unlock CRTC registers */
	outportb(VGA_CRTC_INDEX, 0x03);
	outportb(VGA_CRTC_DATA, inb(VGA_CRTC_DATA) | 0x80);
	outportb(VGA_CRTC_INDEX, 0x11);
	outportb(VGA_CRTC_DATA, inb(VGA_CRTC_DATA) & ~0x80);
/* make sure they remain unlocked */
	regs[0x03] |= 0x80;
	regs[0x11] &= ~0x80;
/* write CRTC regs */
	for(i = 0; i < VGA_NUM_CRTC_REGS; i++)
	{
		outportb(VGA_CRTC_INDEX, i);
		outportb(VGA_CRTC_DATA, *regs);
		regs++;
	}
/* write GRAPHICS CONTROLLER regs */
	for(i = 0; i < VGA_NUM_GC_REGS; i++)
	{
		outportb(VGA_GC_INDEX, i);
		outportb(VGA_GC_DATA, *regs);
		regs++;
	}
/* write ATTRIBUTE CONTROLLER regs */
	for(i = 0; i < VGA_NUM_AC_REGS; i++)
	{
		(void)inb(VGA_INSTAT_READ);
		outportb(VGA_AC_INDEX, i);
		outportb(VGA_AC_WRITE, *regs);
		regs++;
	}
/* lock 16-color palette and unblank display */
	(void)inb(VGA_INSTAT_READ);
	outportb(VGA_AC_INDEX, 0x20);
}

unsigned get_fb_seg(void)
{
	unsigned seg;

	outportb(VGA_GC_INDEX, 6);
	seg = inb(VGA_GC_DATA);
	seg >>= 2;
	seg &= 3;
	switch(seg)
	{
	case 0:
	case 1:
		seg = 0xA000;
		break;
	case 2:
		seg = 0xB000;
		break;
	case 3:
		seg = 0xB800;
		break;
	}
	return seg;
}

void vmemwr(unsigned dst_off, unsigned char *src, unsigned count)
{
	_vmemwr(get_fb_seg(), dst_off, src, count);
}

void set_plane(unsigned p)
{
	unsigned char pmask;

	p &= 3;
	pmask = 1 << p;
/* set read plane */
	outportb(VGA_GC_INDEX, 4);
	outportb(VGA_GC_DATA, p);
/* set write plane */
	outportb(VGA_SEQ_INDEX, 2);
	outportb(VGA_SEQ_DATA, pmask);
}

static void write_font(unsigned char *buf, unsigned font_height)
{
	unsigned char seq2, seq4, gc4, gc5, gc6;
	unsigned i;

/* save registers
set_plane() modifies GC 4 and SEQ 2, so save them as well */
	outportb(VGA_SEQ_INDEX, 2);
	seq2 = inb(VGA_SEQ_DATA);

	outportb(VGA_SEQ_INDEX, 4);
	seq4 = inb(VGA_SEQ_DATA);
/* turn off even-odd addressing (set flat addressing)
assume: chain-4 addressing already off */
	outportb(VGA_SEQ_DATA, seq4 | 0x04);

	outportb(VGA_GC_INDEX, 4);
	gc4 = inb(VGA_GC_DATA);

	outportb(VGA_GC_INDEX, 5);
	gc5 = inb(VGA_GC_DATA);
/* turn off even-odd addressing */
	outportb(VGA_GC_DATA, gc5 & ~0x10);

	outportb(VGA_GC_INDEX, 6);
	gc6 = inb(VGA_GC_DATA);
/* turn off even-odd addressing */
	outportb(VGA_GC_DATA, gc6 & ~0x02);
/* write font to plane P4 */
	set_plane(2);
/* write font 0 */
	for(i = 0; i < 256; i++)
	{
		vmemwr(16384u * 0 + i * 32, buf, font_height);
		buf += font_height;
	}
#if 0
/* write font 1 */
	for(i = 0; i < 256; i++)
	{
		vmemwr(16384u * 1 + i * 32, buf, font_height);
		buf += font_height;
	}
#endif
/* restore registers */
	outportb(VGA_SEQ_INDEX, 2);
	outportb(VGA_SEQ_DATA, seq2);
	outportb(VGA_SEQ_INDEX, 4);
	outportb(VGA_SEQ_DATA, seq4);
	outportb(VGA_GC_INDEX, 4);
	outportb(VGA_GC_DATA, gc4);
	outportb(VGA_GC_INDEX, 5);
	outportb(VGA_GC_DATA, gc5);
	outportb(VGA_GC_INDEX, 6);
	outportb(VGA_GC_DATA, gc6);
}



void set_text_mode(int hi_res);

void vpokeb(unsigned off, unsigned val)
{
	pokeb(get_fb_seg(), off, val);
}

unsigned vpeekb(unsigned off)
{
	return peekb(get_fb_seg(), off);
}

unsigned char reverse_bits(unsigned char arg)
{
	unsigned char ret_val = 0;

	if(arg & 0x01)
		ret_val |= 0x80;
	if(arg & 0x02)
		ret_val |= 0x40;
	if(arg & 0x04)
		ret_val |= 0x20;
	if(arg & 0x08)
		ret_val |= 0x10;
	if(arg & 0x10)
		ret_val |= 0x08;
	if(arg & 0x20)
		ret_val |= 0x04;
	if(arg & 0x40)
		ret_val |= 0x02;
	if(arg & 0x80)
		ret_val |= 0x01;
	return ret_val;
}

void font512(void)
{
/* Turbo C++ 1.0 seems to 'lose' any data declared 'static const' */
	/*static*/ const char msg1[] = "!txet sdrawkcaB";
	/*static*/ const char msg2[] = "?rorrim a toG";
/**/
	unsigned char seq2, seq4, gc4, gc5, gc6;
	unsigned font_height, i, j;

/* start in 80x25 text mode */
	set_text_mode(0);
/* code pasted in from write_font():
save registers
set_plane() modifies GC 4 and SEQ 2, so save them as well */
	outportb(VGA_SEQ_INDEX, 2);
	seq2 = inb(VGA_SEQ_DATA);

	outportb(VGA_SEQ_INDEX, 4);
	seq4 = inb(VGA_SEQ_DATA);
/* turn off even-odd addressing (set flat addressing)
assume: chain-4 addressing already off */
	outportb(VGA_SEQ_DATA, seq4 | 0x04);

	outportb(VGA_GC_INDEX, 4);
	gc4 = inb(VGA_GC_DATA);

	outportb(VGA_GC_INDEX, 5);
	gc5 = inb(VGA_GC_DATA);
/* turn off even-odd addressing */
	outportb(VGA_GC_DATA, gc5 & ~0x10);

	outportb(VGA_GC_INDEX, 6);
	gc6 = inb(VGA_GC_DATA);
/* turn off even-odd addressing */
	outportb(VGA_GC_DATA, gc6 & ~0x02);
/* write font to plane P4 */
	set_plane(2);
/* this is different from write_font():
use font 1 instead of font 0, and use it for BACKWARD text */
	font_height = 16;
	for(i = 0; i < 256; i++)
	{
		for(j = 0; j < font_height; j++)
		{
			vpokeb(16384u * 1 + 32 * i + j,
				reverse_bits(
					g_8x16_font[font_height * i + j]));
		}
	}
/* restore registers */
	outportb(VGA_SEQ_INDEX, 2);
	outportb(VGA_SEQ_DATA, seq2);
	outportb(VGA_SEQ_INDEX, 4);
	outportb(VGA_SEQ_DATA, seq4);
	outportb(VGA_GC_INDEX, 4);
	outportb(VGA_GC_DATA, gc4);
	outportb(VGA_GC_INDEX, 5);
	outportb(VGA_GC_DATA, gc5);
	outportb(VGA_GC_INDEX, 6);
	outportb(VGA_GC_DATA, gc6);
/* now: sacrifice attribute bit b3 (foreground intense color)
use it to select characters 256-511 in the second font */
	outportb(VGA_SEQ_INDEX, 3);
	outportb(VGA_SEQ_DATA, 4);
/* xxx - maybe re-program 16-color palette here
so attribute bit b3 is no longer used for 'intense' */
	for(i = 0; i < sizeof(msg1); i++)
	{
		vpokeb((80 * 8  + 40 + i) * 2 + 0, msg1[i]);
/* set attribute bit b3 for backward font */
		vpokeb((80 * 8  + 40 + i) * 2 + 1, 0x0F);
	}
	for(i = 0; i < sizeof(msg2); i++)
	{
		vpokeb((80 * 16 + 40 + i) * 2 + 0, msg2[i]);
		vpokeb((80 * 16 + 40 + i) * 2 + 1, 0x0F);
	}
}

void set_text_mode(int hi_res)
{
	unsigned rows, cols, ht, i;

	if(hi_res)
	{
		write_regs(g_80x50_text);
		cols = 80;
		rows = 50;
		ht = 8;
	}
	else
	{
		write_regs(g_80x25_text);
		cols = 80;
		rows = 25;
		ht = 16;
	}
/* set font */
	if(ht >= 16)
		write_font(g_8x16_font, 16);
	else
		write_font(g_8x8_font, 8);
/* tell the BIOS what we've done, so BIOS text output works OK */
	pokew(0x40, 0x4A, cols);	/* columns on screen */
	pokew(0x40, 0x4C, cols * rows * 2); /* framebuffer size */
	pokew(0x40, 0x50, 0);		/* cursor pos'n */
	pokeb(0x40, 0x60, ht - 1);	/* cursor shape */
	pokeb(0x40, 0x61, ht - 2);
	pokeb(0x40, 0x84, rows - 1);	/* rows on screen - 1 */
	pokeb(0x40, 0x85, ht);		/* char height */
/* set white-on-black attributes for all text */
	for(i = 0; i < cols * rows; i++)
		pokeb(0xB800, i * 2 + 1, 7);
}


//static void (*g_write_pixel)(unsigned x, unsigned y, unsigned c);

static unsigned g_wd, g_ht;



void write_pixel8x(unsigned x, unsigned y, unsigned c)
{
    /*
	unsigned wd_in_bytes;
	unsigned off;

	wd_in_bytes = g_wd / 4;
	off = wd_in_bytes * y + x / 4;
    //off =get_fb_seg();
	set_plane(x & 3);
	vpokeb(off, c);
    */

    unsigned char* location = (unsigned char*)0xA0000 + 320 * x + y;
    *location = c;

}

//g_write_pixel = write_pixel8;

void draw_x(void)
{
	unsigned x, y;

/* clear screen 
	for(y = 0; y < g_ht; y++)
		for(x = 0; x < g_wd; x++)
			g_write_pixel(x, y, 0);
    */
/* draw 2-color X */
	for(y = 0; y < g_ht; y++)
	{
		g_write_pixel((g_wd - g_ht) / 2 + y, y, 1);
		g_write_pixel((g_ht + g_wd) / 2 - y, y, 2);
	}
}