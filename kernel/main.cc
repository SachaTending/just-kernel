#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <tty.h>
#include <idt.h>
#include <port_io.h>
#include <printf2.h>

void log(const char *logd);

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

int strCmp(const char* s1, const char* s2)
{
    while(*s1 && (*s1 == *s2))
    {
        s1++;
        s2++;
    }
    return *(const unsigned char*)s1 - *(const unsigned char*)s2;
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

int keyb_read()
{
	keyb_reading = true;
	while (!keyb_received);
	keyb_reading = false;
	keyb_received = false;
	return keyb_out;
}

void keyb(struct regs *r)
{
	log("event.keypress ");
	int input = inb(0x60);
	printf("data: 0x%x or %d\n", input, input);
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

void terminal_putentryat(char c, uint8_t color, size_t x, size_t y);

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
			const char cursor_lol = "@";
			terminal_putentryat(0, vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK), old_old_mouse_x, old_old_mouse_y);
			terminal_putentryat(cursor_lol, vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK), old_mouse_x, old_mouse_y);
			terminal_putentryat(cursor_lol, vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK), mouse_x, mouse_y);
			break;
		
	}
}

void timer_call(struct regs *r)
{
	i++;
}

void log(const char *logd)
{
	printf("[%d]%s", i, logd);
}

void install_ints()
{
	irq_install_handler(0, timer_call);
	irq_install_handler(1, keyb);
	irq_install_handler(12, mouse);
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
	uint8_t _status;  //unsigned char
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

extern "C" void kernel_main(void) 
{
	/* Initialize terminal interface */
	terminal_initialize();
	/* Newline support is left as an exercise. */
	log("Just kernel ver 1.0 beta\n");
	log("kernel location:");printf("0x%x kernel end: 0x%x kernel size: 0x%x (or %d)\n", _kernel_start, _kernel_end, _kernel_end - _kernel_start, _kernel_end - _kernel_start);
	log("Hello from ");print_russia();terminal_writestring("!\n");
	log("Note: Sometimes, system triggers excepton, dont worry, and reboot\nbug catched on qemu\n");
	// terminal_writestring("Builded on host: ");terminal_writestring(_HOST_USER);terminal_writestring("@");terminal_writestring(_HOST_NAME);terminal_writestring("\n");
	log("Calling constructros...\n");
	callConstructors();
	log("Installing interrupts...\n");
	install_mouse();
	install_ints();
	asm volatile("hlt");
	log("Testing printf...\n");
	printf("%d decimal\n", 123);
	printf("%x hex\n", 0xFFFU);
	log("Ok, done.\nSystem halted becuase idk what to do\n");
	for (;;) {asm volatile("hlt");}
}