#ifndef _SPINLOCK_H
#define _SPINLOCK_H
#include "types.h"

struct task;

struct __spinlock_t {
    volatile int value;
    struct task *holder;
} __packed;

typedef struct __spinlock_t spinlock_t;

#endif