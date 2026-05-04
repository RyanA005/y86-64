.pos 0x0
    irmovq stack, %rsp
    call main
    halt

.align 8
input:
    .quad 1
    .quad 2
    .quad 3
    .quad 4
    .quad 5
    .quad 11
    .quad 12
    .quad 13
    .quad 14
    .quad 15
length:
    .quad 10

main:
    irmovq input, %rdx      # first param, arr start
    irmovq length, %rdi     # second param, arr len
    irmovq $11, %rsi         # third param, value
    call search
    ret

search:
    


.pos 0x300
stack:
