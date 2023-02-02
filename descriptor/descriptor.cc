#include <module.h>
#include <idt.h>
#include <gdt.h>
#include <tty.h>

MODULE_START_CALL void descriptor_init()
{
    terminal_writestring("Descriptor: Initializating...\n");
    init_idt();
    init_gdt();
    //init_idt();
}