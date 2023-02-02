#include <module.h>
#include <idt.h>
#include <tty.h>
#include <port_io.h>
#include <string.h>
#include <printf.h>

struct idt_entry idt[256];
struct idt_ptr idtp;

const char *exception_messages[] =
{
    "Division By Zero",
    "Debug",
    "Non Maskable Interrupt",
    "Breakpoint",
    "Into Detected Overflow",
    "Out of Bounds",
    "Invalid Opcode",
    "No Coprocessor",

    "Double Fault",
    "Coprocessor Segment Overrun",
    "Bad TSS",
    "Segment Not Present",
    "Stack Fault",
    "General Protection Fault",
    "Page Fault",
    "Unknown Interrupt",

    "Coprocessor Fault",
    "Alignment Check",
    "Machine Check",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",

    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved"
};

extern "C" {
extern void isr0();
extern void isr1();
extern void isr2();
extern void isr3();
extern void isr4();
extern void isr5();
extern void isr6();
extern void isr7();
extern void isr8();
extern void isr9();
extern void isr10();
extern void isr11();
extern void isr12();
extern void isr13();
extern void isr14();
extern void isr15();
extern void isr16();
extern void isr17();
extern void isr18();
extern void isr19();
extern void isr20();
extern void isr21();
extern void isr22();
extern void isr23();
extern void isr24();
extern void isr25();
extern void isr26();
extern void isr27();
extern void isr28();
extern void isr29();
extern void isr30();
extern void isr31();

extern void irq0();
extern void irq1();
extern void irq2();
extern void irq3();
extern void irq4();
extern void irq5();
extern void irq6();
extern void irq7();
extern void irq8();
extern void irq9();
extern void irq10();
extern void irq11();
extern void irq12();
extern void irq13();
extern void irq14();
extern void irq15();
}

void *irq_routines[16] =
{
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0
};

void irq_install_handler(int irq, void (*handler)(struct regs *r))
{
    irq_routines[irq] = handler;
}

void irq_uninstall_handler(int irq)
{
    irq_routines[irq] = 0;
}

extern "C" void idt_load();

void idt_set_gate(unsigned char num, unsigned long base, unsigned short sel, unsigned char flags)
{
    /* The interrupt routine's base address */
    idt[num].base_lo = (base & 0xFFFF);
    idt[num].base_hi = (base >> 16) & 0xFFFF;

    /* The segment or 'selector' that this IDT entry will use
    *  is set here, along with any access flags */
    idt[num].sel = sel;
    idt[num].always0 = 0;
    idt[num].flags = flags;
}

void isrs_install()
{
    idt_set_gate(0, (unsigned long)isr0, 0x08, 0x8E);
    idt_set_gate(1, (unsigned long)isr1, 0x08, 0x8E);
    idt_set_gate(2, (unsigned long)isr2, 0x08, 0x8E);
    idt_set_gate(3, (unsigned long)isr3, 0x08, 0x8E);
    idt_set_gate(4, (unsigned long)isr4, 0x08, 0x8E);
    idt_set_gate(5, (unsigned long)isr5, 0x08, 0x8E);
    idt_set_gate(6, (unsigned long)isr6, 0x08, 0x8E);
    idt_set_gate(7, (unsigned long)isr7, 0x08, 0x8E);

    idt_set_gate(8, (unsigned long)isr8, 0x08, 0x8E);
    idt_set_gate(9, (unsigned long)isr9, 0x08, 0x8E);
    idt_set_gate(10, (unsigned long)isr10, 0x08, 0x8E);
    idt_set_gate(11, (unsigned long)isr11, 0x08, 0x8E);
    idt_set_gate(12, (unsigned long)isr12, 0x08, 0x8E);
    idt_set_gate(13, (unsigned long)isr13, 0x08, 0x8E);
    idt_set_gate(14, (unsigned long)isr14, 0x08, 0x8E);
    idt_set_gate(15, (unsigned long)isr15, 0x08, 0x8E);

    idt_set_gate(16, (unsigned long)isr16, 0x08, 0x8E);
    idt_set_gate(17, (unsigned long)isr17, 0x08, 0x8E);
    idt_set_gate(18, (unsigned long)isr18, 0x08, 0x8E);
    idt_set_gate(19, (unsigned long)isr19, 0x08, 0x8E);
    idt_set_gate(20, (unsigned long)isr20, 0x08, 0x8E);
    idt_set_gate(21, (unsigned long)isr21, 0x08, 0x8E);
    idt_set_gate(22, (unsigned long)isr22, 0x08, 0x8E);
    idt_set_gate(23, (unsigned long)isr23, 0x08, 0x8E);

    idt_set_gate(24, (unsigned long)isr24, 0x08, 0x8E);
    idt_set_gate(25, (unsigned long)isr25, 0x08, 0x8E);
    idt_set_gate(26, (unsigned long)isr26, 0x08, 0x8E);
    idt_set_gate(27, (unsigned long)isr27, 0x08, 0x8E);
    idt_set_gate(28, (unsigned long)isr28, 0x08, 0x8E);
    idt_set_gate(29, (unsigned long)isr29, 0x08, 0x8E);
    idt_set_gate(30, (unsigned long)isr30, 0x08, 0x8E);
    idt_set_gate(31, (unsigned long)isr31, 0x08, 0x8E);
}

void irq_remap(void)
{
    outportb(0x20, 0x11);
    outportb(0xA0, 0x11);
    outportb(0x21, 0x20);
    outportb(0xA1, 0x28);
    outportb(0x21, 0x04);
    outportb(0xA1, 0x02);
    outportb(0x21, 0x01);
    outportb(0xA1, 0x01);
    outportb(0x21, 0x0);
    outportb(0xA1, 0x0);
}

void irq_install()
{
    irq_remap();

    idt_set_gate(32, (unsigned long)irq0, 0x08, 0x8E);
    idt_set_gate(33, (unsigned long)irq1, 0x08, 0x8E);
    idt_set_gate(34, (unsigned long)irq2, 0x08, 0x8E);
    idt_set_gate(35, (unsigned long)irq3, 0x08, 0x8E);
    idt_set_gate(36, (unsigned long)irq4, 0x08, 0x8E);
    idt_set_gate(37, (unsigned long)irq5, 0x08, 0x8E);
    idt_set_gate(38, (unsigned long)irq6, 0x08, 0x8E);
    idt_set_gate(39, (unsigned long)irq7, 0x08, 0x8E);

    idt_set_gate(40, (unsigned long)irq8, 0x08, 0x8E);
    idt_set_gate(41, (unsigned long)irq9, 0x08, 0x8E);
    idt_set_gate(42, (unsigned long)irq10, 0x08, 0x8E);
    idt_set_gate(43, (unsigned long)irq11, 0x08, 0x8E);
    idt_set_gate(44, (unsigned long)irq12, 0x08, 0x8E);
    idt_set_gate(45, (unsigned long)irq13, 0x08, 0x8E);
    idt_set_gate(46, (unsigned long)irq14, 0x08, 0x8E);
    idt_set_gate(47, (unsigned long)irq15, 0x08, 0x8E);
}

extern "C" void irq_handler(struct regs *r)
{
    /* This is a blank function pointer */
    void (*handler)(struct regs *r);
    /* Find out if we have a custom handler to run for this
    *  IRQ, and then finally, run it */
    handler = irq_routines[r->int_no - 32];
    if (handler)
    {
        handler(r);
    }
    /* If the IDT entry that was invoked was greater than 40
    *  (meaning IRQ8 - 15), then we need to send an EOI to
    *  the slave controller */
    if (r->int_no >= 40)
    {
        outportb(0xA0, 0x20);
    }

    /* In either case, we need to send an EOI to the master
    *  interrupt controller too */
    outportb(0x20, 0x20);
}

extern "C" void fault_handler(struct regs *r)
{
    if (r->int_no < 32)
    {
        terminal_writestring(exception_messages[r->int_no]);
        terminal_writestring(" Exception.\n");
        if (r->int_no==3)
        {
            //printf("амогус\n");
            return;
        }
        if (r->int_no==13)
        {
            printf("Caused by instrcuction at 0x%x\n", r->eip);
            //r->eip = 0;
            printf("0x%x\n", (char *)r->eip);
            return;
        }
        printf("System Halted!\n");
        __asm__ volatile("cli");
        for (;;);
    }
}

void init_idt()
{
    terminal_writestring("IDT: Initializating...\n");
    idtp.limit = (sizeof (struct idt_entry) * 256) - 1;
    idtp.base = (uint32_t) &idt;
    memset(&idt, 0, sizeof(struct idt_entry) * 256);
    isrs_install();
    irq_install();
    idt_load();
}