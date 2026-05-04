.pos 0x0
    irmovq stack, %rsp
    call main
    halt

.align 8
silly:
.quad -1
input:
    .quad 1
    .quad 3
    .quad 2
    .quad 5
    .quad 2
    .quad 3
    .quad 2
    .quad 4
    .quad 3
    .quad 2
length:
    .quad 10
silly2:
.quad -1

main:
    irmovq input, %rdx      # first param, arr start
    irmovq length, %rdi     # second param, arr len
    call search
    ret

search:
    mrmovq (%rdx), %r9
    irmovq $8, %r8
    addq %r8, %rdx
    mrmovq (%rdx), %r10
    subq %r9, %r10
    cmovg 
    inner1:
        inner2:

.pos 0x300
stack:
