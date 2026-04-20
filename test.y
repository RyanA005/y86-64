# simple test program

irmovq $10, %rax
irmovq $20, %rbx

addq %rax, %rbx     # rbx = 30
subq %rax, %rbx     # rbx = 20
andq %rax, %rbx
xorq %rax, %rbx

# memory test

irmovq $0x100, %rcx
rmmovq %rax, 0(%rcx)
mrmovq 0(%rcx), %rdx

# stack test

pushq %rax
pushq %rbx
popq %rsi
popq %rdi

halt

