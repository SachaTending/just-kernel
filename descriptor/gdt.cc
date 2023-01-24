#include <module.h>
#include <tty.h>

MODULE_START_CALL void init_gdt()
{
    terminal_writestring("gdt xd\n");
}