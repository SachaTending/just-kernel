section .text

global vga_set_mode

vga_set_mode:
    mov ebp, esp               ; save stack pointer

    push dword  [ebp+4]        ; ss
    push dword  [ebp+8]        ; esp
    pushfd                     ; eflags
    or dword [esp], (1 << 17)  ; set VM flags
    push dword [ebp+12]        ; cs
    push dword  [ebp+16]       ; eip
    iret
    int 0x13
    ret