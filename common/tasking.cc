// that code from wiki.org
// pls work

#include "stdint.h" // maybe needed
#include "pit.h"
#include "general_defs.h" // halt, sti, cli
#include "tasking.h"
#include "printf.h"

void kernel_idle_task(void) {
    printf("\rHello, im idle task, this is fake taskikng, so pls wait for full multitasking(and maybe paging), ok?\n");
    for(;;) {
        halt
    }
}