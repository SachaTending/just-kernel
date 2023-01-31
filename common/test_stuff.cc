#include <port_io.h>
#include <printf.h>
#include <stddef.h>
#include <idt.h>

#define DSP_RESET 0x226
#define DSP_READ 0x22A
#define DSP_WRITE 0x22C
#define DSP_READ_STATUS 0x22E
#define DSP_INT_ACK 0x22F

#define REG_ADDR 0x224
#define REG_DATA 0x225

#define DIRECT_DAC 0x10
#define ENABLE_SPEAKER 0xD1

#define MIXER_WRITE 0x224
#define MIXER_DATA 0x225



#define outb(data, port) outportb(port, data);

void dsp_reset2(){
    int i;
    uint8_t status = 0;
    uint32_t buf[4];
    *buf = 128;
      //rtc_write(0, buf, 4);

    outb(1, DSP_RESET);
    for (size_t i = 0; i < 1000000; i++);
    outb(0, DSP_RESET);
    status = inb(DSP_READ_STATUS);
    if (~status & 128) {
        printf("sb16 init fail\n");
    }

    if(inb(DSP_READ) != 0xAA){
        printf("Could not init sb16\n");
    }

    return;
}

void interrupt_sb16(struct regs *r)
{
    printf("sb16: interrupt fired\n");
}

void reset_sb16();

void play_simple_sound2(){
    return;
    dsp_reset2();
    while(inb(DSP_WRITE));
    printf("Enabling speaker\n");
    outb(0xD1, DSP_WRITE);

    while(inb(DSP_WRITE));
    outportb(MIXER_WRITE, 0x80);
    outportb(MIXER_DATA, 0x02);
    irq_install_handler(5, interrupt_sb16);
    outportb(0x0A, 5);
    outportb(0x0C, 1);
    /*
    printf("Playing sound\n");
    outb(0xF0, DSP_WRITE);

      while(1){
            while(inb(DSP_WRITE));
            outb(0x10, DSP_WRITE);
            outb(0x00, DSP_WRITE);
            //rtc_read(0, 0, NULL, 0);
            while(inb(DSP_WRITE));
            outb(0x10, DSP_WRITE);
            outb(0xFF, DSP_WRITE);
            //rtc_read(0, 0, NULL, 0);
      }

      return;
    */
}
