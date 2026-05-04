#include "includes.c"

int main(int argc, char **argv) {
    
    // basic args check

    if (argc != 2) {
        printf("usage: %s <filename>\n", argv[0]);
        exit(1);
    }
    
    // open file and verify pointer

    FILE *f = fopen(argv[1], "r");

    if (!f) {
        printf("inavlid filename: %s\n", argv[1]);
        exit(1);
    }

    // init cpu stuct 

    cpu c;
    init_cpu(&c);

    // step 1 : turn text into raw opcodes
    
    long num = 0, off = 0, m = 0;
    int i = 0, j = 0;
    enum reg a, b;
    char buf[32], buf2[32];
    char ch;

    // first pass to resolve labels

    while ((ch = fgetc(f)) != EOF) { // read char by char

        if (ch != ' ' && ch != '\n') buf[i++] = ch; // fill buffer with single tokens
        else {
            buf[i] = '\0';
            if (*buf == '#') while ((ch = fgetc(f)) != '\n' && ch != EOF) { // skip comments
                i = 0; 
                continue;
            }
            if (i == 0) continue; // comment or empty line

            if (buf[strlen(buf)-1] == ':') { buf[strlen(buf)-1] = '\0'; push_label(buf, m); } // label (ends in :)
            else if(!strcmp(buf, ".pos")) {
                get_value(buf, f, &num);
                m = num; // literally move pointer to where we write next bytes
            }
            else if(!strcmp(buf, ".align")) {
                get_value(buf, f, &num);
                while(m % num != 0) m++; // increment until multiple of num
            }
            else if(!strcmp(buf, ".byte")) m += 1;
            else if(!strcmp(buf, ".word")) m += 2;
            else if(!strcmp(buf, ".long")) m += 4;
            else if(!strcmp(buf, ".quad")) m += 8;
            else if (!strcmp(buf, "halt")) m += 1;
            else if (!strcmp(buf, "nop")) m += 1;
            else if (!strcmp(buf, "rrmovq")) m += 2;
            else if (!strcmp(buf, "irmovq")) m += 10;
            else if (!strcmp(buf, "rmmovq")) m += 10;
            else if (!strcmp(buf, "mrmovq")) m += 10;
            else if (!strcmp(buf, "addq")) m += 2;
            else if (!strcmp(buf, "subq")) m += 2;
            else if (!strcmp(buf, "andq")) m += 2;
            else if (!strcmp(buf, "xorq")) m += 2;
            else if (!strcmp(buf, "jmp")) m += 9;
            else if (!strcmp(buf, "jle")) m += 9;
            else if (!strcmp(buf, "jl")) m += 9;
            else if (!strcmp(buf, "je")) m += 9;
            else if (!strcmp(buf, "jne")) m += 9;
            else if (!strcmp(buf, "jge")) m += 9;
            else if (!strcmp(buf, "jg")) m += 9;
            else if (!strcmp(buf, "cmovle")) m += 2;
            else if (!strcmp(buf, "cmovl")) m += 2;
            else if (!strcmp(buf, "cmove")) m += 2;
            else if (!strcmp(buf, "cmovne")) m += 2;
            else if (!strcmp(buf, "cmovge")) m += 2;
            else if (!strcmp(buf, "cmovg")) m += 2;
            else if (!strcmp(buf, "call")) m += 9;
            else if (!strcmp(buf, "ret")) m += 1;
            else if (!strcmp(buf, "pushq")) m += 2;
            else if (!strcmp(buf, "popq")) m += 2;

            i = 0;
        }
    }
    
    rewind(f);
    m = 0;

    // second pass to write bytes

    while ((ch = fgetc(f)) != EOF) { // read char by char

        if (ch != ' ' && ch != '\n') buf[i++] = ch; // fill buffer with single tokens
        else {
            buf[i] = '\0';
            if (*buf == '#') while ((ch = fgetc(f)) != '\n' && ch != EOF) { // skip comments
                i = 0; 
                continue;
            }
            if (i == 0) continue; // comment or empty line

            // pseudo ops
            if(!strcmp(buf, ".pos")) {
                get_value(buf, f, &num);
                m = num; // literally move pointer to where we write next bytes
                // printf(".pos %s # %lx\n", buf, num);
            }
            if(!strcmp(buf, ".align")) {
                get_value(buf, f, &num);
                while(m % num != 0) m++; // increment until multiple of num
                // printf(".align %lx\n", num);
            }
            if(!strcmp(buf, ".byte")) { // write 1 byte
                get_value(buf, f, &num);
                memcpy(&c.mem[m], (unsigned char *)&num, 1);
                m += 1;
                // printf(".byte %lx\n", num);
            }
            if(!strcmp(buf, ".word")) { // write 2
                get_value(buf, f, &num);
                memcpy(&c.mem[m], (unsigned char *)&num, 2);
                m += 2;
                // printf(".word %lx\n", num);
            }
            if(!strcmp(buf, ".long")) { // write 4
                get_value(buf, f, &num);
                memcpy(&c.mem[m], (unsigned char *)&num, 4);
                m += 4;
                // printf(".long %lx\n", num);
            }
            if(!strcmp(buf, ".quad")) { // write 8
                get_value(buf, f, &num);
                memcpy(&c.mem[m], (unsigned char *)&num, 8);
                m += 8;
                // printf(".quad %lx\n", num);
            }


            // instructions

            if (!strcmp(buf, "halt")) { 
                c.mem[m++] = (char) (0 << 4) + 0;
                // printf("halt\n");
            }
            else if (!strcmp(buf, "nop")) {
                c.mem[m++] = (char) (1 << 4) + 0;
                // printf("nop\n");
            }
            else if (!strcmp(buf, "rrmovq")) { 
                c.mem[m++] = (2 << 4) + 0;
                get_two_registers(buf, f, &a, &b);
                c.mem[m++] = (a << 4) + b;
                // printf("rrmovq %s, %s\n", get_reg_name(a), get_reg_name(b));
            }
            else if (!strcmp(buf, "irmovq")) {
                c.mem[m++] = (char) (3 << 4) + 0;
                get_tok(buf, f); // should be "<immediate>,"
                buf[strlen(buf) - 1] = '\0'; // remove trailing','
                // special case of immediate written first, must remove comma
                if (resolve_label(buf) >= 0) num = resolve_label(buf);
                else if(buf[2] == 'x') num = strtol(buf+3, NULL, 16);
                else num = strtol(buf+1, NULL, 10); 
                get_second_register(buf, f, &b);
                c.mem[m++] = (15 << 4) + b; // write two nibbles, first is 'f'
                memcpy(&c.mem[m], (unsigned char *)&num, 8);
                m += 8;
                // printf("irmovq 0x%lx, %s\n", num, get_reg_name(b));
            }
            else if (!strcmp(buf, "rmmovq")) { 
                c.mem[m++] = (char) (4 << 4) + 0;
                get_first_register(buf, f, &a);
                get_tok(buf, f); // should be a "D(rb)"
                while (buf[j] != '(') { buf2[j] = buf[j]; j++; }
                if (j > 0) {
                    if (resolve_label(buf) >= 0) num = resolve_label(buf);
                    else if (buf[0] == '0' && buf[1] == 'x') num = strtol(buf + 2, NULL, 16);
                    else num = strtol(buf, NULL, 10);
                }
                buf[strlen(buf) - 1] = '\0'; // remove trailing')'
                b = get_reg(buf+j+1);
                c.mem[m++] = (a << 4) + b;
                memcpy(&c.mem[m], (unsigned char *)&num, 8);
                m += 8;
                // printf("rmmovq %s, 0x%lx(%s)\n", get_reg_name(a), off, get_reg_name(b));
            }
            else if (!strcmp(buf, "mrmovq")) {
                c.mem[m++] = (char) (5 << 4) + 0;
                get_tok(buf, f); // should be a "D(ra),"
                while (buf[j] != '(') { buf2[j] = buf[j]; j++; }
                if (j > 0) {
                    if (resolve_label(buf) >= 0) num = resolve_label(buf);
                    else if (buf[0] == '0' && buf[1] == 'x') num = strtol(buf + 2, NULL, 16);
                    else num = strtol(buf, NULL, 10);
                }
                buf[strlen(buf) - 2] = '\0'; // remove trailing'),'
                a = get_reg(buf+j+1);
                get_second_register(buf, f, &b);
                c.mem[m++] = (a << 4) + b;
                memcpy(&c.mem[m], (unsigned char *)&num, 8);
                m += 8;
                // printf("mrmovq 0x%lx(%s), %s\n", off, get_reg_name(a), get_reg_name(b));
            }
            else if (!strcmp(buf, "addq")) {
                c.mem[m++] = (char) (6 << 4) + 0;
                get_two_registers(buf, f, &a, &b);
                c.mem[m++] = (a << 4) + b;
                // printf("addq %s, %s\n", get_reg_name(a), get_reg_name(b));
            }
            else if (!strcmp(buf, "subq")) { 
                c.mem[m++] = (char) (6 << 4) + 1;
                get_two_registers(buf, f, &a, &b);
                c.mem[m++] = (a << 4) + b;
                // printf("subq %s, %s\n", get_reg_name(a), get_reg_name(b));
            }
            else if (!strcmp(buf, "andq")) {
                c.mem[m++] = (char) (6 << 4) + 2;
                get_two_registers(buf, f, &a, &b);
                c.mem[m++] = (a << 4) + b;
                // printf("andq %s, %s\n", get_reg_name(a), get_reg_name(b));
            }
            else if (!strcmp(buf, "xorq")) { 
                c.mem[m++] = (char) (6 << 4) + 3;
                get_two_registers(buf, f, &a, &b);
                c.mem[m++] = (a << 4) + b;
                // printf("xorq %s, %s\n", get_reg_name(a), get_reg_name(b));
            }
            else if (!strcmp(buf, "jmp")) {
                c.mem[m++] = (char) (7 << 4) + 0;
                get_value(buf, f, &num);
                memcpy(&c.mem[m], (unsigned char *)&num, 8);
                m += 8;
                // printf("jmp %s # 0x%lx\n", buf, num);
            }
            else if (!strcmp(buf, "jle")) {
                c.mem[m++] = (char) (7 << 4) + 1;
                get_value(buf, f, &num);
                memcpy(&c.mem[m], (unsigned char *)&num, 8);
                m += 8;
                // printf("jle %s # 0x%lx\n", buf, num);
            }
            else if (!strcmp(buf, "jl")) {
                c.mem[m++] = (char) (7 << 4) + 2;
                get_value(buf, f, &num);
                memcpy(&c.mem[m], (unsigned char *)&num, 8);
                m += 8;
                // printf("jl %s # 0x%lx\n", buf, num);
            }
            else if (!strcmp(buf, "je")) {
                c.mem[m++] = (char) (7 << 4) + 3;
                get_value(buf, f, &num);
                memcpy(&c.mem[m], (unsigned char *)&num, 8);
                m += 8;
                // printf("je %s # 0x%lx\n", buf, num);
            }
            else if (!strcmp(buf, "jne")) {
                c.mem[m++] = (char) (7 << 4) + 4;
                get_value(buf, f, &num);
                memcpy(&c.mem[m], (unsigned char *)&num, 8);
                m += 8;
                // printf("jne %s # 0x%lx\n", buf, num);
            }
            else if (!strcmp(buf, "jge")) {
                c.mem[m++] = (char) (7 << 4) + 5;
                get_value(buf, f, &num);
                memcpy(&c.mem[m], (unsigned char *)&num, 8);
                m += 8;
                // printf("jge %s # 0x%lx\n", buf, num);
            }
            else if (!strcmp(buf, "jg")) {
                c.mem[m++] = (char) (7 << 4) + 6;
                get_value(buf, f, &num);
                memcpy(&c.mem[m], (unsigned char *)&num, 8);
                m += 8;
                // printf("jg %s # 0x%lx\n", buf, num);
            }
            else if (!strcmp(buf, "cmovle")) {
                c.mem[m++] = (char) (2 << 4) + 1;
                get_two_registers(buf, f, &a, &b);
                c.mem[m++] = (a << 4) + b;
                // printf("cmovle %s, %s\n", get_reg_name(a), get_reg_name(b));
            }
            else if (!strcmp(buf, "cmovl")) {
                c.mem[m++] = (char) (2 << 4) + 2;
                get_two_registers(buf, f, &a, &b);
                c.mem[m++] = (a << 4) + b;
                // printf("cmovl %s, %s\n", get_reg_name(a), get_reg_name(b));
            }
            else if (!strcmp(buf, "cmove")) {
                c.mem[m++] = (char) (2 << 4) + 3;
                get_two_registers(buf, f, &a, &b);
                c.mem[m++] = (a << 4) + b;
                // printf("cmove %s, %s\n", get_reg_name(a), get_reg_name(b));
            }
            else if (!strcmp(buf, "cmovne")) {
                c.mem[m++] = (char) (2 << 4) + 4;
                get_two_registers(buf, f, &a, &b);
                c.mem[m++] = (a << 4) + b;
                // printf("cmovne %s, %s\n", get_reg_name(a), get_reg_name(b));
            }
            else if (!strcmp(buf, "cmovge")) {
                c.mem[m++] = (char) (2 << 4) + 5;
                get_two_registers(buf, f, &a, &b);
                c.mem[m++] = (a << 4) + b;
                // printf("cmovge %s, %s\n", get_reg_name(a), get_reg_name(b));
            }
            else if (!strcmp(buf, "cmovg")) {
                c.mem[m++] = (char) (2 << 4) + 6;
                get_two_registers(buf, f, &a, &b);
                c.mem[m++] = (a << 4) + b;
                // printf("cmovg %s, %s\n", get_reg_name(a), get_reg_name(b));
            }
            else if (!strcmp(buf, "call")) {
                c.mem[m++] = (char) (8 << 4) + 0;
                get_value(buf, f, &num);
                memcpy(&c.mem[m], (unsigned char *)&num, 8);
                m += 8;
                // printf("call %s # 0x%lx\n", buf, num);
            }
            else if (!strcmp(buf, "ret")) {
                c.mem[m++] = (char) (9 << 4) + 0;
                // printf("ret\n");
            }
            else if (!strcmp(buf, "pushq")) {
                c.mem[m++] = (char) (10 << 4) + 0;
                get_tok(buf, f);
                a = get_reg(buf);
                c.mem[m++] = (a << 4) + 15;
                // printf("pushq %s\n", get_reg_name(a));
            }
            else if (!strcmp(buf, "popq")) {
                c.mem[m++] = (char) (11 << 4) + 0;
                get_tok(buf, f);
                a = get_reg(buf);
                c.mem[m++] = (a << 4) + 15;
                // printf("popq %s\n", get_reg_name(a));
            }
            
            i = 0, j = 0, num = 0, off = 0;
        }
    }

    fclose(f);

    // step 2 - execute instructions

    char stepping = 1;

    while (!c.stat) {
        
        i++;

        long temp = 0, im = 0, dest = 0;
        unsigned char icode = ((c.mem[c.pc] >> 4) & 0xf);
        unsigned char ifun = (c.mem[c.pc] & 0xf);
        unsigned char ra = 0, rb = 0; 

        c.pc++;

        char command[32];
        while(stepping) {
            printf("command : ");
            scanf("%s", command);
            if (!strcmp(command, "help")) {
                printf("commands:\nstep\nrun\nregisters\nstack\nflags\nexit\n");
            }
            else if (!strcmp(command, "step")) {
                break;
            }
            else if (!strcmp(command, "run")) {
                stepping = 0;
                break;
            }
            else if (!strcmp(command, "registers")) {
                print_registers(&c);
            }
            else if (!strcmp(command, "stack")) {
                print_stack(&c);
            }
            else if (!strcmp(command, "flags")) {
                print_flags(&c);
            }
            else if (!strcmp(command, "exit")) {
                printf("exiting...\n");
                return 0;
            }
            else {
                printf("unknown command\n");
            }
         }
        
        printf("0x%-4lx : ", c.pc - 1);

        switch (icode) {
            case 0: // halt
                c.stat = HLT;
                printf("halt\n");
                break;
            case 1: // nop
                printf("nop\n");
                break;
            case 2: // rrmovq OR cmovxx
                ra = ((c.mem[c.pc] >> 4) & 0xf);
                rb = (c.mem[c.pc] & 0x0f);

                if (ifun == 0) { // rrmovq
                    printf("rrmovq %s, %s\n", get_reg_name(ra), get_reg_name(rb));
                    c.regs[rb] = c.regs[ra];
                }
                else if (ifun == 1) { // cmovle
                    printf("cmovle %s, %s\n", get_reg_name(ra), get_reg_name(rb));
                    if ((c.cc[SF] ^ c.cc[OF]) | c.cc[ZF]) {
                        c.regs[rb] = c.regs[ra];
                    }
                }
                else if (ifun == 2) { // cmovl
                    printf("cmovl %s, %s\n", get_reg_name(ra), get_reg_name(rb));
                    if (c.cc[SF] ^ c.cc[OF]) {
                        c.regs[rb] = c.regs[ra];
                    }
                }
                else if (ifun == 3) { // cmove
                    printf("cmove %s, %s\n", get_reg_name(ra), get_reg_name(rb));
                    if (c.cc[ZF]) {
                        c.regs[rb] = c.regs[ra];
                    }
                }
                else if (ifun == 4) { // cmovne
                    printf("cmovne %s, %s\n", get_reg_name(ra), get_reg_name(rb));
                    if (!c.cc[ZF]) {
                        c.regs[rb] = c.regs[ra];
                    }
                }
                else if (ifun == 5) { // cmovge
                    printf("cmovge %s, %s\n", get_reg_name(ra), get_reg_name(rb));
                    if (!(c.cc[SF] ^ c.cc[OF])) {
                        c.regs[rb] = c.regs[ra];
                    }
                }
                else if (ifun == 6) { // cmovg
                    printf("cmovg %s, %s\n", get_reg_name(ra), get_reg_name(rb));
                    if (!((c.cc[SF] ^ c.cc[OF]) | c.cc[ZF])) {
                        c.regs[rb] = c.regs[ra];
                    }
                }
                c.pc += 1;
                break;
            case 3: // irmovq
                ra = ((c.mem[c.pc] >> 4) & 0xf);
                rb = (c.mem[c.pc] & 0xf);
                c.pc += 1;
                memcpy(&temp, &c.mem[c.pc], 8);
                printf("irmovq 0x%lx, %s\n", temp, get_reg_name(rb));
                c.regs[rb] = temp;
                c.pc += 8;
                break;
            case 4: // rmmovq
                ra = ((c.mem[c.pc] >> 4) & 0xf);
                rb = (c.mem[c.pc] & 0xf);
                c.pc += 1;
                memcpy(&num, &c.mem[c.pc], 8); // copy displacement
                c.pc += 8;
                printf("rmmovq %s, 0x%lx(%s)\n", get_reg_name(ra), num, get_reg_name(rb));
                num += c.regs[rb]; // displacement + rb
                memcpy(&c.mem[num], &c.regs[ra], 8); // set mem at num
                break;
            case 5: // mrmovq
                ra = ((c.mem[c.pc] >> 4) & 0xf);
                rb = (c.mem[c.pc] & 0xf);
                c.pc += 1;
                memcpy(&num, &c.mem[c.pc], 8); // copy displacement
                c.pc += 8;
                printf("mrmovq 0x%lx(%s), %s\n", num, get_reg_name(ra), get_reg_name(rb));
                num += c.regs[ra]; // displacement + rb
                memcpy(&c.regs[rb], &c.mem[num], 8); // reg to *num
                break;
            case 6: // opq
                ra = ((c.mem[c.pc] >> 4) & 0xf);
                rb = (c.mem[c.pc] & 0xf);
                temp = c.regs[rb];
                
                for (i = 0; i < 3; i++) { c.cc[i] = 0; }

                if (ifun == 0) { // addq
                    printf("addq %s, %s\n", get_reg_name(ra), get_reg_name(rb));
                    c.regs[rb] += c.regs[ra];
                    if ((c.regs[ra] < 0 && temp < 0 && c.regs[rb] >= 0) || (c.regs[ra] > 0 && temp > 0 && c.regs[rb] <= 0)) c.cc[OF] = 1;
                }
                else if (ifun == 1) { // subq
                    printf("subq %s, %s\n", get_reg_name(ra), get_reg_name(rb));
                    c.regs[rb] -= c.regs[ra];
                    if ((c.regs[ra] > 0 && temp < 0 && c.regs[rb] > 0) || (c.regs[ra] < 0 && temp > 0 && c.regs[rb] < 0)) c.cc[OF] = 1;
                }
                else if (ifun == 2) { // andq
                    printf("andq %s, %s\n", get_reg_name(ra), get_reg_name(rb));
                    c.regs[rb] &= c.regs[ra];
                }
                else if (ifun == 3) { // xorq
                    printf("xorq %s, %s\n", get_reg_name(ra), get_reg_name(rb));
                    c.regs[rb] ^= c.regs[ra];
                }
                if (c.regs[rb] == 0) c.cc[ZF] = 1;
                if (c.regs[rb] < 0)  c.cc[SF] = 1;
                c.pc += 1;
                break;
            case 7: // jxx
                if (ifun == 0) { // jmp
                    printf("jmp %lx\n", c.pc);
                    memcpy(&c.pc, &c.mem[c.pc], 8);
                    break;
                }
                else if (ifun == 1) { // jle
                    printf("jle %lx\n", c.pc);
                    if ((c.cc[SF] ^ c.cc[OF]) | c.cc[ZF]) {
                        memcpy(&c.pc, &c.mem[c.pc], 8);
                        break;
                    }
                }
                else if (ifun == 2) { // jl
                    printf("jl %lx\n", c.pc);
                    if (c.cc[SF] ^ c.cc[OF]) {
                        memcpy(&c.pc, &c.mem[c.pc], 8);
                        break;
                    }
                }
                else if (ifun == 3) { // je
                    printf("je %lx\n", c.pc);
                    if (c.cc[ZF]) {
                        memcpy(&c.pc, &c.mem[c.pc], 8);
                        break;
                    }
                }
                else if (ifun == 4) { // jne
                    printf("jne %lx\n", c.pc);
                    if (!c.cc[ZF]) {
                        memcpy(&c.pc, &c.mem[c.pc], 8);
                        break;
                    }
                }
                else if (ifun == 5) { // jge
                    printf("jge %lx\n", c.pc);
                    if (!(c.cc[SF] ^ c.cc[OF])) {
                        memcpy(&c.pc, &c.mem[c.pc], 8);
                        break;
                    }
                }
                else if (ifun == 6) { // jg
                    printf("jg %lx\n", c.pc);
                    if (!((c.cc[SF] ^ c.cc[OF]) | c.cc[ZF])) {
                        memcpy(&c.pc, &c.mem[c.pc], 8);
                        break;
                    }
                }
                c.pc += 8;
                break;
            case 8: // call
                memcpy(&temp, &c.mem[c.pc], 8);
                printf("call 0x%lx\n", temp);
                c.pc += 8;
                c.regs[rsp] -= 8;
                memcpy(&c.mem[c.regs[rsp]], &c.pc, 8);
                c.pc = temp;
                break;
            case 9: // ret
                printf("ret\n");
                memcpy(&c.pc, &c.mem[c.regs[rsp]], 8);
                c.regs[rsp] += 8;
                break;
            case 10: // pushq
                ra = (c.mem[c.pc] >> 4) & 0xf;
                printf("pushq %s\n", get_reg_name(ra));
                c.pc += 1;
                c.regs[rsp] -= 8;
                memcpy(&c.mem[c.regs[rsp]], &c.regs[ra], 8);
                break;
            case 11: // popq
                ra = (c.mem[c.pc] >> 4) & 0xf;
                printf("popq %s\n", get_reg_name(ra));
                c.pc += 1;
                memcpy(&c.regs[ra], &c.mem[c.regs[rsp]], 8);
                c.regs[rsp] += 8;
                break;
            default:
                printf("error\n");
                break;
        }
    }
    /*

    */

    // print_cpu(&c);

    return 0;
}
