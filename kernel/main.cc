#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <tty.h>

typedef void (*constructor)();
extern constructor start_ctors;
extern constructor end_ctors;
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
	terminal_writestring("Ru");
	terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_BLUE, VGA_COLOR_BLACK));
	terminal_writestring("ss");
	terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK));
	terminal_writestring("ia");
	terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK)); // Restore colors
}

extern "C" void kernel_main(void) 
{
	/* Initialize terminal interface */
	terminal_initialize();
	/* Newline support is left as an exercise. */
	terminal_writestring("Hello, kernel World!\n");
	terminal_writestring("Also hello from ");print_russia();terminal_writestring("!\n");
	terminal_writestring("Calling constructros...\n");
	callConstructors();
}