# entry point
.pos 0
    irmovq stack, %rsp     # initialize stack pointer fix to use label later
    call main
    halt

# main program
.pos 0x20
main:
    # register + ALU test
    irmovq $10, %rax
    irmovq $20, %rbx
    addq %rax, %rbx        # rbx = 30
    subq %rax, %rbx        # rbx = 20
    andq %rax, %rbx        # rbx = 0x10 & 0x14 = 0x10
    xorq %rax, %rbx        # rbx = 0x10 ^ 0x0a

    # memory test
    irmovq data, %rcx
    rmmovq %rax, 0(%rcx)   # store rax
    mrmovq 0(%rcx), %rdx   # load into rdx

    # stack test
    pushq %rax
    pushq %rbx
    popq %rsi
    popq %rdi

    # call test
    call func

    halt

# function: increments rax
func:
    irmovq $1, %r8
    addq %r8, %rax
    ret

# data section
.pos 0x200
data:
    .quad 0

# stack (grows downward)
.pos 0x300
stack:
