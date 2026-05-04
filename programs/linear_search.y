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
length:
    .quad 5

output:
    .quad 0

main:
    irmovq input, %rdx      # first param, arr start
    irmovq length, %rdi     # second param, arr len
    irmovq $6, %rsi         # third param, value
    call search
    ret

search:
    xorq %rax, %rax
    irmovq $1, %r8
    mrmovq (%rdi), %r13
continue:
    subq %r8, %r13
    jl done
    irmovq $8, %r9
    rrmovq %rdx, %r10
    mrmovq (%rdx), %r12
    subq %rsi, %r12
    je found
    rrmovq %r10, %rdx
    addq %r9, %rdx
    jmp continue

found:
    mrmovq (%r10), %rax
    ret

done:
    irmovq $-1, %rax
    ret


.pos 0x300
stack:
