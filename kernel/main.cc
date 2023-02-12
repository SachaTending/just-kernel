#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <tty.h>
#include <idt.h>
#include <port_io.h>
#include "printf.h"
#include "string.h"
#include <multiboot.h>
#include <vga.h>
#include "pci.h"

void log(const char *logd);

void ata_is_sus();

typedef void (*constructor)();
extern constructor start_ctors;
extern constructor end_ctors;
extern uint32_t _kernel_start;
extern uint32_t _kernel_end;
extern void callConstructors(void)
{
    for(constructor* i = &start_ctors;i != &end_ctors; i++)
        (*i)();
}

void print_russia() // made in russia xd
{
	terminal_setcolor(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
	printf("Ru");
	terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_BLUE, VGA_COLOR_BLACK));
	printf("ss");
	terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK));
	printf("ia");
	terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK)); // Restore colors
}



int i = 0;

bool keyb_received;
bool keyb_reading;
int keyb_out;

uint8_t led_stat = 0;

int keyb_read()
{
	keyb_reading = true;
	while (!keyb_received);
	keyb_reading = false;
	keyb_received = false;
	return keyb_out;
}


void kbd_led_handling(uint8_t ledstatus);

void keyb(struct regs *r)
{
	log("event.keypress ");
	int input = inb(0x60);
	printf("data: 0x%x or %d char %s\n", input, input, input);
	if (keyb_reading)
	{
		keyb_received = true;
		keyb_out = input;
	}
}

uint8_t prev_button_state[3];
uint8_t curr_button_state[3];

void print_button_state() {
    //qemu_printf("prev state: %d %d %d\n", prev_button_state[0], prev_button_state[1], prev_button_state[2]);
    //qemu_printf("curr state: %d %d %d\n", curr_button_state[0], curr_button_state[1], curr_button_state[2]);
}
int left_button_down() {
    return !prev_button_state[0] && curr_button_state[0];
}

int right_button_down() {
    return !prev_button_state[2] && curr_button_state[2];
}

int left_button_up() {
    return prev_button_state[0] && !curr_button_state[0];
}

int right_button_up() {
    return prev_button_state[2] && !curr_button_state[2];
}

/*
 * Every time mouse event fires, mouse_handler will be called
 * The argument regs is not used in here
 * */

uint8_t mouse_read();

#define MOUSE_LEFT_BUTTON(flag) (flag & 0x1)
#define MOUSE_RIGHT_BUTTON(flag) (flag & 0x2)
#define MOUSE_MIDDLE_BUTTON(flag) (flag & 0x4)

int mouse_x;
int mouse_y;

int old_mouse_x;
int old_mouse_y;

int old_old_mouse_x;
int old_old_mouse_y;

void mouse_print(size_t x, size_t y, uint8_t color, const char* data);

void mouse(struct regs * r)
{
    //printf("Mouse interrupt fired\n");
    static uint8_t mouse_cycle = 0;
    static char mouse_byte[3];
	switch(mouse_cycle) {
        case 0:
            mouse_byte[0] = mouse_read();
            // Inform the window message handler about mouse operation
            if(MOUSE_LEFT_BUTTON(mouse_byte[0])) {
                curr_button_state[0] = 1;
            }
            else {
                curr_button_state[0] = 0;
            }

            if(MOUSE_RIGHT_BUTTON(mouse_byte[0])) {
                curr_button_state[2] = 1;
            }
            else {
                curr_button_state[2] = 0;
            }
            mouse_cycle++;
            break;
		case 1:
            mouse_byte[1] = mouse_read();
            mouse_cycle++;
            break;
		case 2:
            mouse_byte[2]= mouse_read();
            // Position is not changed
            //if(mouse_byte[1] == 0 && mouse_byte[2] == 0)
            //    break;

            // Update mouse position
            // Transform delta values using some sort of log function
			old_old_mouse_x = old_mouse_x;
			old_old_mouse_y = old_mouse_y;

			old_mouse_x = mouse_x;
			old_mouse_y = mouse_y;
            mouse_x = mouse_x + (mouse_byte[1]);
            mouse_y = mouse_y - (mouse_byte[2]);
			if(mouse_x < 0)
                mouse_x = 0;
            if(mouse_y < 0)
                mouse_y = 0;
			if(mouse_x > 80 - 1)
                mouse_x = 80 - 1;
            if(mouse_y > 25 - 1)
                mouse_y = 25 - 1;
			mouse_cycle = 0;
            char *cursor = "h";
			const char *cursor_lol = "m";
            void save_buf();
            void reprint_buf();
            reprint_buf();
            save_buf();
			//terminal_putentryat2(0, vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK), old_old_mouse_x, old_old_mouse_y);
			//mouse_print(old_mouse_x, old_mouse_y, vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK), " ");
			mouse_print(mouse_x, mouse_y, vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK), "THIS IS MOUSE");
			break;
		
	}
}

void timer_call(struct regs *r)
{
	//i++;
}

static inline uint64_t rdtsc()
{
    uint64_t ret;
    asm volatile ( "rdtsc" : "=A"(ret) );
    return ret;
}

void log(const char *logd)
{
	printf("[%d]%s", i, logd);
}

void rtc_call(struct regs *r)
{
    i++;
    outportb(0x70, 0x0C);	// select register C
    inb(0x71);		// just throw away contents
}

void ide_primary_irq(struct regs *r)
{
	//printf("ATA: ide primary irq triggered\n");
}

void ide_secondary_irq(struct regs *r)
{
	//printf("ATA: ide second irq triggered\n");
}


void install_ints()
{
	irq_install_handler(0, timer_call);
	irq_install_handler(1, keyb);
    irq_install_handler(8, rtc_call);
	irq_install_handler(12, mouse);
    irq_install_handler(15, ide_secondary_irq);
    irq_install_handler(14, ide_primary_irq);
}

void mouse_wait(uint8_t a_type) {
    uint32_t _time_out=100000; //unsigned int
    if(a_type==0) {
        while(_time_out--) {
            if((inb(0x64) & 1)==1)
            {
                return;
            }
        }
        return;
    }
    else {
        while(_time_out--) {
            if((inb(0x64) & 2)==0) {
                return;
            }
        }
        return;
    }
}

void mouse_write(uint8_t a_write) //unsigned char
{
    //Tell the mouse we are sending a command
    mouse_wait(1);
    outportb(0x64, 0xD4);
    mouse_wait(1);
    //Finally write
    outportb(0x60, a_write);
}

uint8_t mouse_read()
{
    mouse_wait(0);
    return inb(0x60);
}

void install_mouse()
{
	uint8_t _status; 
    //Enable the auxiliary mouse device
    mouse_wait(1);
    outportb(0x64, 0xA8);

    //Enable the interrupts
    mouse_wait(1);
    outportb(0x64, 0x20);
    mouse_wait(0);
    _status=(inb(0x60) | 2);
    mouse_wait(1);
    outportb(0x64, 0x60);
    mouse_wait(1);
    outportb(0x60, _status);

    // Tell the mouse to use default settings
    mouse_write(0xF6);
    mouse_read();  //Acknowledge

    // Enable the mouse
    mouse_write(0xF4);
    mouse_read();  //Acknowledge
}

void kbd_ack(void){
  while(!(inb(0x60)==0xfa));
}


void kbd_led_handling(uint8_t ledstatus){;
    outportb(0x60,0xed);
    kbd_ack();
    outportb(0x60,ledstatus);
}

void play_simple_sound(multiboot_info_t *mbi);

#define CHECK_FLAG(flags,bit)   ((flags) & (1 << (bit)))

extern "C" void trigger_gp(void);

static void fillrect(unsigned char *vram, unsigned char r, unsigned char g, unsigned   char b, unsigned char w, unsigned char h) {
    unsigned char *where = vram;
    int i, j;
 
    for (i = 0; i < w; i++) {
        for (j = 0; j < h; j++) {
            g_write_pixel(j, i, r);
        }
    }
}

void mm_init(uint32_t kernel_end);
char* malloc(size_t size);
void free(void *mem);
void mm_print_out();

class abc {
    public:
        void sus();
};

void abc::sus()
{
    log("abc::sus called!\n");
}

extern "C" void kernel_main(multiboot_info_t *mbi) 
{
    //write_regs(g_80x50_text);
    PCI pci;
    write_font(g_8x16_font, 16);
    font512();
    //sset_text_mode(1);
    write_font(g_8x8_font, 8);
	terminal_initialize();
	log("Just kernel ver 1.5 beta by TendingStream73#5806\n");
    log("Booted by: ");printf("%s\n", mbi->boot_loader_name);
	log("kernel location:");printf("0x%x kernel end: 0x%x kernel size: %d\n", (unsigned)&_kernel_start, (unsigned)&_kernel_end, (unsigned)&_kernel_end - (unsigned)&_kernel_start);
	log("Hello from ");print_russia();terminal_writestring("!\n");
	log("Note: Sometimes, system triggers excepton, dont worry, and reboot\nbug catched on qemu\n");
    mm_init((unsigned)&_kernel_end);
    pci.pci_init();
    pci.pci_proc_dump();
	// terminal_writestring("Builded on host: ");terminal_writestring(_HOST_USER);terminal_writestring("@");terminal_writestring(_HOST_NAME);terminal_writestring("\n");
    log("Calling constructros...\n");
	callConstructors();
	log("Installing interrupts...\n");
	install_mouse();
	install_ints();
    outportb(0x70, 0x8B);		// select register B, and disable NMI
    char prev=inb(0x71);	// read the current value of register B
    outportb(0x70, 0x8B);		// set the index again (a read will reset the index to register D)
    outportb(0x71, prev | 0x40);	// write the previous value ORed with 0x40. This turns on bit 6 of register B
	asm volatile("hlt");
	log("Testing printf...\n");
	printf("%d decimal\n", 123);
	printf("%x hex\n", 0xABC);
    write_regs(g_320x200x256_modex);
    uint32_t * frmabebuffer=(uint32_t *)get_fb_seg();
    write_regs(g_80x50_text);
    printf("Frmabebuffer at 0x%x\n", frmabebuffer);
    //printf("We going to graphics!\n");
    int i2;
    while (i2<10000000) {i2++;}
    //write_regs(g_320x200x256_modex);
    memset((void *)frmabebuffer, 0, 320*200);
    i2 = 0;
    int offest2;
    /*
    while (i2<256)
    {
        i2++;
        offest2 = offest2 + 100;
        memset(0xA0000+offest2, i2, 100);
    }
    */
    
    //fillrect(0xA0000, 5, 4, 4, 4 ,4);
    //memset(0xA0000, 0, 320*200);
    //write_regs(g_320x200x256_modex);
	//g_write_pixel = write_pixel8x;
	//draw_x();
    //play_simple_sound(mbi);
    log("Testing C++ features...\n");
    abc Abc;
    Abc.sus();
    log("System halted becuase idk what to do\n");
    ata_is_sus();
    //trigger_gp();
	// for (;;) {asm volatile("hlt");}
    for (;;)
    {
        asm volatile("hlt");
    }
}