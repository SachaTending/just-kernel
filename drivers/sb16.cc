#include <stdint.h>
#include <printf2.h>
#include <port_io.h>
#include <stddef.h>
#include <module.h>
#include <idt.h>
#include <multiboot.h>
#include <string.h>

typedef double f64;

f64 fmod(f64 x, f64 m);
f64 fabs(f64 x);
f64 sin(f64 x);
f64 cos(f64 x);

#define int16_t int

#define NUM_NOTES 8

#define NUM_OCTAVES 7
#define OCTAVE_SIZE 12

#define OCTAVE_1 0
#define OCTAVE_2 1
#define OCTAVE_3 2
#define OCTAVE_4 3
#define OCTAVE_5 4
#define OCTAVE_6 5
#define OCTAVE_7 6

#define NOTE_C      0
#define NOTE_CS     1
#define NOTE_DF     NOTE_CS
#define NOTE_D      2
#define NOTE_DS     3
#define NOTE_EF     NOTE_DS
#define NOTE_E      4
#define NOTE_F      5
#define NOTE_FS     6
#define NOTE_GF     NOTE_FS
#define NOTE_G      7
#define NOTE_GS     8
#define NOTE_AF     NOTE_GS
#define NOTE_A      9
#define NOTE_AS     10
#define NOTE_BF     NOTE_AS
#define NOTE_B      11

#define NOTE_NONE   12

#define WAVE_SIN        0
#define WAVE_SQUARE     1
#define WAVE_NOISE      2
#define WAVE_TRIANGLE   3

#define MIXER_IRQ       0x5
#define MIXER_IRQ_DATA  0x2

#define DSP_MIXER       0x224
#define DSP_MIXER_DATA  0x225
#define DSP_RESET       0x226
#define DSP_READ        0x22A
#define DSP_WRITE       0x22C
#define DSP_READ_STATUS 0x22E
#define DSP_ACK_8       DSP_READ_STATUS
#define DSP_ACK_16      0x22F

#define DSP_PROG_16     0xB0
#define DSP_PROG_8      0xC0
#define DSP_AUTO_INIT   0x06
#define DSP_PLAY        0x00
#define DSP_RECORD      0x08
#define DSP_MONO        0x00
#define DSP_STEREO      0x20
#define DSP_UNSIGNED    0x00
#define DSP_SIGNED      0x10

#define DMA_CHANNEL_16  5
#define DMA_FLIP_FLOP   0xD8
#define DMA_BASE_ADDR   0xC4
#define DMA_COUNT       0xC6

#define DSP_SET_TIME    0x40
#define DSP_SET_RATE    0x41
#define DSP_ON          0xD1
#define DSP_OFF         0xD3
#define DSP_OFF_8       0xD0
#define DSP_ON_8        0xD4
#define DSP_OFF_16      0xD5
#define DSP_ON_16       0xD6
#define DSP_VERSION     0xE1

#define DSP_VOLUME  0x22
#define DSP_IRQ     0x80

#define SAMPLE_RATE     44100
#define BUFFER_MS       40

#define DSP_HALT_SINGLE_CYCLE_DMA 0x00D0

#define BUFFER_SIZE ((size_t) (SAMPLE_RATE * (BUFFER_MS / 1000.0)))

#define E 2.71828
#define PI 3.14159265358979323846264338327950

static bool buffer_flip = false;

static int16_t buffer[BUFFER_SIZE];

static void reset_sb16();

static void transfer(void *buf, uint32_t len) {
    uint8_t mode = 0x48;

    // disable DMA channel
    outportb(DSP_ON_8, 4 + (DMA_CHANNEL_16 % 4));

    // clear byte-poiner flip-flop
    outportb(DMA_FLIP_FLOP, 1);

    // write DMA mode for transfer
    outportb(DSP_ON_16, (DMA_CHANNEL_16 % 4) | mode | (1 << 4));

    // write buffer offset (div 2 for 16-bit)
    uint16_t offset = (((unsigned long) buf) / 2) % 65536;
    outportb(DMA_BASE_ADDR, (uint8_t) ((offset >> 0) & 0xFF));
    outportb(DMA_BASE_ADDR, (uint8_t) ((offset >> 8) & 0xFF));

    // write transfer length
    outportb(DMA_COUNT, (uint8_t) (((len - 1) >> 0) & 0xFF));
    outportb(DMA_COUNT, (uint8_t) (((len - 1) >> 8) & 0xFF));

    // write buffer
    outportb(0x8B, ((unsigned long) buf) >> 16);

    // enable DMA channel
    outportb(0xD4, DMA_CHANNEL_16 % 4);
}

static void dsp_write(uint8_t b) {
    while (inb(DSP_WRITE) & 0x80);
    outportb(DSP_WRITE, b);
}

typedef struct {
    char             format[4];      // RIFF
    unsigned long     f_len;          // filelength
    char            wave_fmt[8];    // WAVEfmt_
    unsigned long    fmt_len;    // format lenght
    unsigned short  fmt_tag;    // format Tag
    unsigned short  channel;    // Mono/Stereo
    unsigned long   samples_per_sec;
    unsigned long   avg_bytes_per_sec;
    unsigned short  blk_align;
    unsigned short  bits_per_sample;
    char            data[4];        // data
    unsigned long   data_len;    // data size
} wavehdr_t;

static void set_sample_rate(uint16_t hz) {
    dsp_write(DSP_SET_RATE);
    dsp_write((uint8_t) ((hz >> 8) & 0xFF));
    dsp_write((uint8_t) (hz & 0xFF));
}
char buf[50] = {};

int position = 50;

wavehdr_t *current = {0};

multiboot_module_t *cur = {0};




int isprint(char c) {
    return ((c >= ' ' && c <= '~') ? 1 : 0);
}

void xxd(void * data, unsigned int len)
{
    unsigned int i, j;

    for(i = 0; i < len + ((len % 25) ? (80 - len % 25) : 0); i++) {
        /* print offset */
        if(i % 80 == 0) {
            printf("0x%06x: ", i);
        }

        /* print hex data */
        if(i < len) {
            printf("%02x ", 0xFF & ((char*)data)[i]);
        }
        else {
            /* end of block, just aligning for ASCII dump */
            printf("   ");
        }

        /* print ASCII dump */
        if(i % 25 == (25 - 1)) {
            for(j = i - (80 - 1); j <= i; j++) {
                if(j >= len) {
                    printf(' ');
                }
                else if(isprint(((char*)data)[j])) /* printable char */ {
                    printf(0xFF & ((char*)data)[j]);
                }
                else {
                    printf('.');
                }
            }
            printf('\n');
        }
    }
    printf("\n");
}

static void sb16_irq_handler(struct regs *regs) {
    buffer_flip = !buffer_flip;
    //printf("SB16: Interrupt triggered!\n");
    position++;
    //printf("%d %x %s\n", position, cur->mod_start + position, (char *)((char *) current->data)[position]);
    //memset(cur->mod_start+position, (unsigned)&buf, 50);
    int pos = cur->mod_start+position;
    memset(buf,pos,40);
    // dsp_write(DSP_HALT_SINGLE_CYCLE_DMA);
    //transfer(buf, 12);
    //printf("%x\n", buf);
    //printf("%d 0x%x\n", pos, pos);
    xxd(buf,1);
    // dsp_write(DSP_HALT_SINGLE_CYCLE_DMA);
    inb(DSP_READ_STATUS);
    inb(DSP_ACK_16);
}

#include <bruh.h>


MODULE_START_CALL void init_sb16()
{
    return;
    printf("SB16: initializating...\n");
    reset_sb16();
    wavehdr_t *bruh_info = (wavehdr_t *) bruh;
    printf("SB16: channels: %d\n", bruh_info->channel);
    printf("SB16: data len: %d\n", bruh_info->data_len);
    uint16_t sample_count = (42442 / 2) - 1;
    irq_install_handler(MIXER_IRQ, sb16_irq_handler);
    outportb(DSP_MIXER, DSP_IRQ);
    outportb(DSP_MIXER_DATA, MIXER_IRQ_DATA);
    set_sample_rate(44100);
    dsp_write(DSP_PLAY | DSP_PROG_16 | DSP_AUTO_INIT);
    dsp_write(DSP_SIGNED | DSP_MONO);
    dsp_write((uint8_t) ((sample_count >> 0) & 0xFF));
    dsp_write((uint8_t) ((sample_count >> 8) & 0xFF));
    dsp_write(DSP_ON);
    dsp_write(DSP_ON_16);
    //transfer(bruh_info->data, bruh_info->data_len - bruh_info->data_len%80);
}

void play_simple_sound(multiboot_info_t *mbi)
{
    multiboot_module_t *mod;
    int i;
      
    printf ("mods_count = %d, mods_addr = 0x%x\n",
            (int) mbi->mods_count, (int) mbi->mods_addr);
    for (i = 0, mod = (multiboot_module_t *) mbi->mods_addr;
       i < mbi->mods_count;
           i++, mod++) {
            current = (wavehdr_t *) mod->mod_start;
            printf("SB16: channels: %d\n", current->channel);
            printf("SB16: data len: %d\n", current->data_len);
            printf("0x%x 0x%x 0x%x %d\n", mod->mod_start, mod->mod_end, mod->mod_end - mod->mod_start, mod->mod_end - mod->mod_start);
            reset_sb16();
            irq_install_handler(MIXER_IRQ, sb16_irq_handler);
            outportb(DSP_MIXER, DSP_IRQ);
            outportb(DSP_MIXER_DATA, MIXER_IRQ_DATA);
            set_sample_rate(48000);
            uint16_t sample_count = (current->data_len / 2) - 1;
            dsp_write(DSP_PLAY | DSP_PROG_16 | DSP_AUTO_INIT);
            dsp_write(DSP_UNSIGNED | DSP_STEREO);
            dsp_write((uint8_t) ((sample_count >> 0) & 0xFF));
            dsp_write((uint8_t) ((sample_count >> 8) & 0xFF));
            dsp_write(DSP_ON);
            dsp_write(DSP_ON_16);
            position = 1;
            cur = mod;
            memset((unsigned)&buf,cur->mod_start+position,40);
            transfer(buf, 50);
        }
}

void reset_sb16() {
    uint8_t status;
    uint8_t major;
    uint8_t minor;

    outportb(DSP_RESET, 1);

    // TODO: maybe not necessary
    // ~3 microseconds?
    for (size_t i = 0; i < 1000000; i++);

    outportb(DSP_RESET, 0);

    status = inb(DSP_READ_STATUS);
    if (~status & 128) {
        goto fail;
    }

    status = inb(DSP_READ);
    if (status != 0xAA) {
        goto fail;
    }

    outportb(DSP_WRITE, DSP_VERSION);
    major = inb(DSP_READ),
    minor = inb(DSP_READ);
    printf("SB16: version: %d.%d\n", major, minor);
    if (major < 4) {
        status = (major << 4) | minor;
        goto fail;
    }

    return;
fail:
    printf("failed to reset sb16, status %d\n", status);
}