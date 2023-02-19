
.globl ap_trampoline
    .code16
ap_trampoline:
    cli
    cld
    ljmp    $0, $0x8040
    .align 16
_L8010_GDT_table:
    .long 0, 0
    .long 0x0000FFFF, 0x00CF9A00    
    .long 0x0000FFFF, 0x008F9200    
    .long 0x00000068, 0x00CF8900    
_L8030_GDT_value:
    .word _L8030_GDT_value - _L8010_GDT_table - 1
    .long 0x8010
    .long 0, 0
    .align 64
_L8040:
    xorw    %ax, %ax
    movw    %ax, %ds
    lgdtl   0x8030
    movl    %cr0, %eax
    orl     $1, %eax
    movl    %eax, %cr0
    ljmp    $8, $0x8060
    .align 32
    .code32
_L8060:
    movw    $16, %ax
    movw    %ax, %ds
    movw    %ax, %ss
    mov     $1, %eax
    cpuid
    shrl    $24, %ebx
    movl    %ebx, %edi
    shll    $15, %ebx
    movl    stack_top2, %esp
    subl    %ebx, %esp
    pushl   %edi
1:  pause
    cmpb    $0, bspdone
    jz      1b
    ljmp    $8, $ap_startup