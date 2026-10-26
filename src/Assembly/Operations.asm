bits 64
default rel

section .text
    global ANDOP
    
ANDOP:
    mov eax, ecx
    and eax, edx
    ret