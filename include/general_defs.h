#pragma once

#define halt asm volatile ("hlt");
#define sti asm volatile ("sti");
#define cli asm volatile ("cli");
