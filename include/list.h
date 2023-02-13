#ifndef _LIST_H
#define _LIST_H

struct list_elem 
{
    struct list_elem *prev;     /* Previous list element. */
    struct list_elem *next;     /* Next list element. */
};

/* List. */
struct list 
{
    struct list_elem head;      /* List head. */
    struct list_elem tail;      /* List tail. */
};

#endif