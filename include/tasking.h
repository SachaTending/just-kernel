#pragma once

void kernel_idle_task(void);
typedef struct _process {
	struct _process* prev;
	char* name;
	uint32_t pid;
	uint32_t esp;
	uint32_t stacktop;
	uint32_t eip;
	uint32_t cr3;
	uint32_t state;
	/* open() */
	uint16_t num_open_files;
	char **open_files;
	void (*notify)(int);
	struct _process* next;
} PROCESS;
#define PROCESS_STATE_ALIVE 0
#define PROCESS_STATE_ZOMBIE 1
#define PROCESS_STATE_DEAD 2

#define SIG_ILL 1
#define SIG_TERM 2
#define SIG_SEGV 3

void send_sig(int sig);
char* p_name();
int p_pid();