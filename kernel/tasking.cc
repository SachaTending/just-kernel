#include "printf.h"
#include "string.h"
#include "malloc.h"
#include "tasking.h"


PROCESS* c = 0;
uint32_t lpid = 0;
uint8_t __enabled = 0;

void schedule_noirq()
{
	if(!__enabled) return;
	asm volatile("int $0x2e");
	return;
}

void _kill() {
    printf("Hello, im _kill() function\n");
    if(c->pid == 1) { printf("Idle can't be killed!"); return; }
	printf("Killing process %s (%d)\n", c->name, c->pid);
	__enabled = 0;
    printf("Freeing stack...\n");
	free((void *)c->stacktop);
    printf("Freeing task...\n");
	free(c);
    printf("Freeing page table...\n");
	pfree((void *)c->cr3);
    printf("Switching task...\n");
	c->prev->next = c->next;
	c->next->prev = c->prev;
    printf("Enabling tasking...\n");
	__enabled = 1;
    printf("Im done.\n");
    schedule_noirq();
}

void __notified(int sig)
{

	switch(sig)
	{
		case SIG_ILL:
			printf("Received SIGILL, terminating!\n");
			_kill();
			break;
		case SIG_TERM:
			printf("Received SIGTERM, terminating!\n");
			_kill();
            break;
		case SIG_SEGV:
			printf("Received SIGSEGV, terminating!\n");
			_kill();
            break;
		default:
			printf("Received unknown SIG!\n");
			return;
	}
}

void send_sig(int sig)
{
	c->notify(sig);
}

int is_tasking()
{
	return __enabled;
}

PROCESS *p_proc()
{
	return c;
}

char* p_name()
{
	return c->name;
}

int p_pid()
{
	return c->pid;
}

void schedule() {
    if (is_tasking()) {
            //printf("schedule() called!\n");
        	asm volatile("mov %%esp, %%eax":"=a"(c->esp));
            c = c->next;
            asm volatile("mov %%eax, %%cr3": :"a"(c->cr3));
            asm volatile("mov %%eax, %%esp": :"a"(c->esp));
            asm volatile("out %%al, %%dx": :"d"(0x20), "a"(0x20)); // send EoI to master PIC
            asm volatile("iret");
    }
}

PROCESS* createProcess(char* name, uint32_t addr)
{
	PROCESS* p = (PROCESS *)malloc(sizeof(PROCESS));
	memset(p, 0, sizeof(PROCESS));
	p->name = name;
	p->pid = ++lpid;
	p->eip = addr;
	p->state = PROCESS_STATE_ALIVE;
	p->notify = __notified;
	p->esp = (uint32_t)malloc(4096);
	asm volatile("mov %%cr3, %%eax":"=a"(p->cr3));
	uint32_t* stack = (uint32_t *)(p->esp + 4096);
	p->stacktop = p->esp;
	*--stack = 0x00000202; // eflags
	*--stack = 0x8; // cs
	*--stack = (uint32_t)addr; // eip
	*--stack = 0; // eax
	*--stack = 0; // ebx
	*--stack = 0; // ecx;
	*--stack = 0; //edx
	*--stack = 0; //esi
	*--stack = 0; //edi
	*--stack = p->esp + 4096; //ebp
	*--stack = 0x10; // ds
	*--stack = 0x10; // fs
	*--stack = 0x10; // es
	*--stack = 0x10; // gs
	p->esp = (uint32_t)stack;
	printf("Created task %s with esp=0x%x eip=0x%x\n", p->name, p->esp, p->eip);
	return p;
}

void idle() {
    printf("Im idle task.\n");
    __enabled = 1;
    while (1) asm volatile ("hlt");
}

void __exec()
{
	asm volatile("mov %%eax, %%esp": :"a"(c->esp));
	asm volatile("pop %gs");
	asm volatile("pop %fs");
	asm volatile("pop %es");
	asm volatile("pop %ds");
	asm volatile("pop %ebp");
	asm volatile("pop %edi");
	asm volatile("pop %esi");
	asm volatile("pop %edx");
	asm volatile("pop %ecx");
	asm volatile("pop %ebx");
	asm volatile("pop %eax");
	asm volatile("iret");
}

void task3() {
    while (1) printf("task3()\n");
}

void task4() {
    while (1) printf("task4()\n");
}

void sus() {
    printf("amogus");
    while (1) asm volatile ("hlt");
}

void __addProcess(PROCESS* p)
{
	p->next = c->next;
	p->next->prev = p;
	p->prev = c;
	c->next = p;
}

int addProcess(PROCESS* p)
{
	__enabled = 0;
	__addProcess(p);
	__enabled = 1;
	return p->pid;
}



void setup_tasking() {
    printf("Setupping tasking...\n");
    c = createProcess("idle", (uint32_t)idle);
    c->next = c;
	c->prev = c;
    __addProcess(createProcess("sus", (uint32_t)sus));
    __addProcess(createProcess("task3", (uint32_t)task3));
    __addProcess(createProcess("task4", (uint32_t)task4));
    __exec();
}