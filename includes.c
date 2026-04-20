#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#define MEMSIZE 2048

// data stuctures

enum reg { rax, rcx, rdx, rbx, rsp, rbp, rsi, rdi, r8, r9, r10, r11, r12, r13, r14, garb };

typedef struct {
    char mem[MEMSIZE];
    long regs[16];
    long pc;
    char cc[4];
    char stat;
} cpu;

// helpers

void init_cpu(cpu *c) { // init entire CPU to 0
    for(int i = 0; i < MEMSIZE; i++ ) c->mem[i] = 0;
    for(int i = 0; i < 16; i++ ) c->regs[i] = 0;
    for(int i = 0; i < 4; i++ ) c->cc[i] = 0;
    c->stat = 0;
    c->pc = 0;
    c->regs[rsp] = 100;
}

enum reg get_reg(char *s) {
    if (!strcmp(s, "%rax")) return rax; // 0
    if (!strcmp(s, "%rcx")) return rcx; // 1
    if (!strcmp(s, "%rdx")) return rdx; // 2
    if (!strcmp(s, "%rbx")) return rbx; // 3
    if (!strcmp(s, "%rsp")) return rsp; // 4
    if (!strcmp(s, "%rbp")) return rbp; // 5
    if (!strcmp(s, "%rsi")) return rsi; // 6
    if (!strcmp(s, "%rdi")) return rdi; // 7
    if (!strcmp(s, "%r8")) return r8;   // 8
    if (!strcmp(s, "%r9")) return r9;   // 9 
    if (!strcmp(s, "%r10")) return r10; // 10 a
    if (!strcmp(s, "%r11")) return r11; // 11 b
    if (!strcmp(s, "%r12")) return r12; // 12 c 
    if (!strcmp(s, "%r13")) return r13; // 13 d
    if (!strcmp(s, "%r14")) return r14; // 14 e 
    else return garb;                // 15 f
}

char *get_reg_name(enum reg r) {
    if (r == 0) return "%rax"; 
    if (r == 1) return "%rcx"; 
    if (r == 2) return "%rdx"; 
    if (r == 3) return "%rbx"; 
    if (r == 4) return "%rsp"; 
    if (r == 5) return "%rbp"; 
    if (r == 6) return "%rsi"; 
    if (r == 7) return "%rdi"; 
    if (r == 8) return "%r8";   
    if (r == 9) return "%r9";   
    if (r == 10) return "%r10"; 
    if (r == 11) return "%r11"; 
    if (r == 12) return "%r12"; 
    if (r == 13) return "%r13"; 
    if (r == 14) return "%r14"; 
    else return "garb";
}

void print_cpu(cpu *c) {
    printf("pc : %lx\n\n", c->pc);
    printf("registers:\n");
    for (int i = 0; i < 4; i++) {
        printf("%4s : %4lx", get_reg_name(i), c->regs[i]);
        printf(" | %4s : %4lx", get_reg_name(i+4), c->regs[i+4]);
        printf(" | %4s : %4lx", get_reg_name(i+8), c->regs[i+8]);
        printf(" | %4s : %4lx\n", get_reg_name(i+12), c->regs[i+12]);
    }
    printf("\n[ZF : %x] [SF : %x] [OF : %x] [STAT : %x]\n", c->cc[0], c->cc[1], c->cc[2], c->stat);

    printf("\nmemory up to rsp (%03lx):\n", c->regs[rsp]);
    printf("000 :");
    for (int i = 0; i < c->regs[rsp]; i++) {
        printf(" %02x", (unsigned char) c->mem[i]);
        if (!((i + 1) % 8)) printf("\n%03x :", i);
    }
    printf("\n\n");
}

void get_tok(char *buf, FILE *f) {
    int i = 0; char ch;
    while ((ch = fgetc(f)) != ' ' && ch != '\n' && i < 32) buf[i++] = ch;
    buf[i] = '\0';
}

