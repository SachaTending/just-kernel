#include <tty.h>
#include <port_io.h>
#include <string.h>

size_t terminal_row;
size_t terminal_column;
uint8_t terminal_color;
uint16_t* terminal_buffer;

void terminal_initialize(void) 
{
	terminal_row = 0;
	terminal_column = 0;
	terminal_color = vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
	terminal_buffer = (uint16_t*) 0xB8000;
	for (size_t y = 0; y < VGA_HEIGHT; y++) {
		for (size_t x = 0; x < VGA_WIDTH; x++) {
			const size_t index = y * VGA_WIDTH + x;
			terminal_buffer[index] = vga_entry(' ', terminal_color);
		}
	}
}

void terminal_setcolor(uint8_t color) 
{
	terminal_color = color;
}

void terminal_putentryat(char c, uint8_t color, size_t x, size_t y) 
{
	const size_t index = y * VGA_WIDTH + x;
	terminal_buffer[index] = vga_entry(c, color);
}

void update_cursor();

void terminal_putchar(char c) 
{
	if(c == '\n') {
        terminal_column = 0;
        terminal_row++;
    } else {
		terminal_putentryat(c, terminal_color, terminal_column, terminal_row);
		if (++terminal_column == VGA_WIDTH) {
			terminal_column = 0;
			if (++terminal_row == VGA_HEIGHT)
				terminal_row = 0;
		}
	}
	update_cursor();
}

void update_cursor() {

    unsigned curr_pos = terminal_row * VGA_WIDTH + terminal_column;

    outportb(0x3D4, 14);
    outportb(0x3D5, curr_pos >> 8);
    outportb(0x3D4, 15);
    outportb(0x3D5, curr_pos);
}

void terminal_write(const char* data, size_t size) 
{
	char d;
	for (size_t i = 0; i < size; i++) {
		d = data[i];
		terminal_putchar(d);
	}
}

void terminal_writestring(const char* data) 
{
	terminal_write(data, strlen(data));
}