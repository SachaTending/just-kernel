#include "malloc.h"
#include "printf.h"
#include "stdint.h"

static uint32_t page_directory[1024] __attribute__((aligned(4096)));
static uint32_t page_dir_loc = 0;
static uint32_t* last_page = 0;
static uint32_t first_page_table[1024] __attribute__((aligned(4096)));

extern "C" void loadPageDirectory(unsigned int*);
extern "C" void enablePaging();

void paging_map_virtual_to_phys(uint32_t virt, uint32_t phys)
{
	uint16_t id = virt >> 22;
	for(int i = 0; i < 1024; i++)
	{
		last_page[i] = phys | 3;
		phys += 4096;
	}
	page_directory[id] = ((uint32_t)last_page) | 3;
	last_page = (uint32_t *)(((uint32_t)last_page) + 4096);
	printf("Mapping 0x%x (%d) to 0x%x\n", virt, id, phys);
}

void paging_enable()
{
    printf("DEBUG: Writing page directory location...\n");
	asm volatile("mov %%eax, %%cr3": :"a"(page_dir_loc));	
	asm volatile("mov %cr0, %eax");
    printf("DEBUG: Setting PAGING_ENABLE bit...\n");
	//asm volatile("orl $0x80000000, %eax");
    asm volatile("orl $0x80000001, %eax");
    printf("(pls work)DEBUG: Enabling paging...\n");
	asm volatile("mov %eax, %cr0");
}

extern uint32_t _kernel_start;

void paging_init()
{
	printf("Setting up paging\n");
	//page_directory = (uint32_t*)0x400000; // In original code(https://github.com/levex/osdev/blob/bba025f8cfced6ad1addc625aaf9dab8fa7aef80/memory/paging.c#L43), it uses fixed address, thats unsafe :skull:
	page_dir_loc = (uint32_t)&page_directory;
	last_page = (uint32_t *)page_dir_loc+1;
	for(int i = 0; i < 1024; i++)
	{
		page_directory[i] = 0 | 2;
	}
	//paging_map_virtual_to_phys(0, 0);
    //paging_map_virtual_to_phys(0, &_kernel_start);
	//paging_map_virtual_to_phys(0, 0x100000);
    //we will fill all 1024 entries in the table, mapping 4 megabytes
    int i;
    for(i = 0; i < 1024; i++)
    {
        // As the address is page aligned, it will always leave 12 bits zeroed.
        // Those bits are used by the attributes ;)
        first_page_table[i] = (i * 0x1000) | 3; // attributes: supervisor level, read/write, present.
    }
    page_directory[0] = ((unsigned int)first_page_table) | 3;
    //paging_map_virtual_to_phys((uint32_t)&_kernel_start, (uint32_t)&_kernel_start);
	//paging_enable();
	loadPageDirectory(page_directory);
	enablePaging();
	printf("Paging was successfully enabled!\n");
}