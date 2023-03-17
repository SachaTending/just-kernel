#include "printf.h"
#include "stdint.h"
#include "string.h"
#include "malloc.h"

uint32_t page_directory[1024] __attribute__((aligned(4096)));

uint32_t first_page_table[1024] __attribute__((aligned(4096)));

extern "C" void loadPageDirectory(unsigned int*);
extern "C" void enablePaging();

void setup_paging() {
    printf("Paging: Hello, im paging module, currentl, im called via setup_paging()\n");
    //page_directory = (uint32_t *)malloc(sizeof(page_directory));
    memset((void *)&page_directory, 0, sizeof(page_directory));
    //set each entry to not present
    int i;
    for(i = 0; i < 1024; i++)
    {
        // This sets the following flags to the pages:
        //   Supervisor: Only kernel-mode can access them
        //   Write Enabled: It can be both read from and written to
        //   Not Present: The page table is not present
        page_directory[i] = 0x00000002;
    }
    unsigned int a;
    
    //we will fill all 1024 entries in the table, mapping 4 megabytes
    for(a = 0; a < 1024; a++)
    {
        // As the address is page aligned, it will always leave 12 bits zeroed.
        // Those bits are used by the attributes ;)
        first_page_table[a] = (i * 0x1000) | 3; // attributes: supervisor level, read/write, present.
    }
    // attributes: supervisor level, read/write, present
    //page_directory[0] = ((unsigned int)first_page_table) | 3;
    asm volatile("mov %0, %%cr3":: "r"(&page_directory));
    printf("Paging: Enabling paging...\n");
    uint32_t cr0;
    asm volatile("mov %%cr0, %0": "=r"(cr0));
    cr0 |= 0x80000000; // Enable paging!
    asm volatile("mov %0, %%cr0":: "r"(cr0));
    printf("Paging: Done.\n");
}