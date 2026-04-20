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
            if(!strcmp(buf, ".pos")) {
                get_tok(buf, f); 
                if (buf[0] == '0' && buf[1] == 'x') num = strtol(buf + 2, NULL, 16);
                else num = strtol(buf, NULL, 10);
                m = num; // literally move pointer to where we write next bytes
            }
            if(!strcmp(buf, ".align")) {
                get_tok(buf, f); // should be number
                if (buf[0] == '0' && buf[1] == 'x') num = strtol(buf + 2, NULL, 16);
                else num = strtol(buf, NULL, 10);
                while(m % num != 0) m++; // increment until multiple of num
            }
            if(!strcmp(buf, ".byte")) m += 1;
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

    // print label table
    label_table *temp = head;
    printf("label table: \n");
    while (temp) {
        printf("%s : 0x%lx\n", temp->name, temp->location);
        temp = temp->next;
    }
    printf("\n");

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
                get_tok(buf, f); // should be memory location (chosing to omit labels for now...)
                if (buf[0] == '0' && buf[1] == 'x') num = strtol(buf + 2, NULL, 16);
                else num = strtol(buf, NULL, 10);
                m = num; // literally move pointer to where we write next bytes
                printf(".pos %s # %lx\n", buf, num);
            }
            if(!strcmp(buf, ".align")) {
                get_tok(buf, f); // should be number
                if (buf[0] == '0' && buf[1] == 'x') num = strtol(buf + 2, NULL, 16);
                else num = strtol(buf, NULL, 10);
                while(m % num != 0) m++; // increment until multiple of num
                printf(".align %lx\n", num);
            }
            if(!strcmp(buf, ".byte")) { // write 1 byte
                get_tok(buf, f);
                if (buf[0] == '0' && buf[1] == 'x') num = strtol(buf + 2, NULL, 16);
                else num = strtol(buf, NULL, 10);
                memcpy(&c.mem[m], (unsigned char *)&num, 1);
                m ++;
                printf(".byte %lx\n", num);
            }
            if(!strcmp(buf, ".word")) { // write 2
                get_tok(buf, f);
                if (buf[0] == '0' && buf[1] == 'x') num = strtol(buf + 2, NULL, 16);
                else num = strtol(buf, NULL, 10);
                memcpy(&c.mem[m], (unsigned char *)&num, 2);
                m += 2;
                printf(".word %lx\n", num);
            }
            if(!strcmp(buf, ".long")) { // write 4
                get_tok(buf, f);
                if (buf[0] == '0' && buf[1] == 'x') num = strtol(buf + 2, NULL, 16);
                else num = strtol(buf, NULL, 10);
                memcpy(&c.mem[m], (unsigned char *)&num, 4);
                m += 4;
                printf(".long %lx\n", num);
            }
            if(!strcmp(buf, ".quad")) { // write 8
                get_tok(buf, f);
                if (buf[0] == '0' && buf[1] == 'x') num = strtol(buf + 2, NULL, 16);
                else num = strtol(buf, NULL, 10);
                memcpy(&c.mem[m], (unsigned char *)&num, 8);
                m += 8;
                printf(".quad %lx\n", num);
            }


            // instructions
            if (!strcmp(buf, "halt")) { 
                c.mem[m++] = (char) (0 << 4) + 0;
                printf("halt\n");
            }
            else if (!strcmp(buf, "nop")) {
                c.mem[m++] = (char) (1 << 4) + 0;
                printf("nop\n");
            }
            else if (!strcmp(buf, "rrmovq")) { 
                c.mem[m++] = (2 << 4) + 0;
                get_tok(buf, f); // should be a "ra,"
                buf[strlen(buf) - 1] = '\0'; // remove trailing','
                a = get_reg(buf);
                get_tok(buf, f); // should be a "rb"
                b = get_reg(buf);
                c.mem[m++] = (a << 4) + b; // write two nibbles
                printf("rrmovq %s, %s\n", get_reg_name(a), get_reg_name(b));
            }
            else if (!strcmp(buf, "irmovq")) {
                c.mem[m++] = (char) (3 << 4) + 0;
                get_tok(buf, f); // should be "<immediate>,"
                buf[strlen(buf) - 1] = '\0'; // remove trailing','
                if(buf[2] == 'x') num = strtol(buf+3, NULL, 16); // remove leading '$0x'
                else num = strtol(buf+1, NULL, 10); // remove leading '$'
                get_tok(buf, f); // should be a "rb"
                b = get_reg(buf);
                c.mem[m++] = (15 << 4) + b; // write two nibbles, first is 'f'
                memcpy(&c.mem[m], (unsigned char *)&num, 8);
                m += 8;
                printf("irmovq 0x%lx, %s\n", num, get_reg_name(b));
            }
            else if (!strcmp(buf, "rmmovq")) { 
                c.mem[m++] = (char) (4 << 4) + 0;
                get_tok(buf, f); // should be a "ra,"
                buf[strlen(buf) - 1] = '\0'; // remove trailing','
                a = get_reg(buf);
                get_tok(buf, f); // should be a "D(rb)"
                while (buf[j] != '(') { buf2[j] = buf[j]; j++; }
                if (j > 0) {
                    buf2[j] = '\0';
                    if (buf2[0] == '0' && buf2[1] == 'x') off = strtol(buf2 + 2, NULL, 16);
                    else off = strtol(buf2, NULL, 10);
                }
                buf[strlen(buf) - 1] = '\0'; // remove trailing')'
                b = get_reg(buf+j+1);
                c.mem[m++] = (a << 4) + b; // write two nibbles
                memcpy(&c.mem[m], (unsigned char *)&off, 8);
                m += 8;
                printf("rmmovq %s, 0x%lx(%s)\n", get_reg_name(a), off, get_reg_name(b));
            }
            else if (!strcmp(buf, "mrmovq")) {
                c.mem[m++] = (char) (5 << 4) + 0;
                get_tok(buf, f); // should be a "D(ra)"
                while (buf[j] != '(') { buf2[j] = buf[j]; j++; }
                if (j > 0) {
                    buf2[j] = '\0';
                    if (buf2[0] == '0' && buf2[1] == 'x') off = strtol(buf2 + 2, NULL, 16);
                    else off = strtol(buf2, NULL, 10);
                }
                buf[strlen(buf) - 2] = '\0'; // remove trailing'),'
                a = get_reg(buf+j+1);
                get_tok(buf, f); // should be a "rb,"
                b = get_reg(buf);
                c.mem[m++] = (a << 4) + b; // write two nibbles
                memcpy(&c.mem[m], (unsigned char *)&off, 8);
                m += 8;
                printf("mrmovq 0x%lx(%s), %s\n", off, get_reg_name(a), get_reg_name(b));
            }
            else if (!strcmp(buf, "addq")) {
                c.mem[m++] = (char) (6 << 4) + 0;
                get_tok(buf, f); // should be a "ra,"
                buf[strlen(buf) - 1] = '\0'; // remove trailing','
                a = get_reg(buf);
                get_tok(buf, f); // should be a "rb"
                b = get_reg(buf);
                c.mem[m++] = (a << 4) + b; // write two nibbles
                printf("addq %s, %s\n", get_reg_name(a), get_reg_name(b));
            }
            else if (!strcmp(buf, "subq")) { 
                c.mem[m++] = (char) (6 << 4) + 1;
                get_tok(buf, f); // should be a "ra,"
                buf[strlen(buf) - 1] = '\0'; // remove trailing','
                a = get_reg(buf);
                get_tok(buf, f); // should be a "rb"
                b = get_reg(buf);
                c.mem[m++] = (a << 4) + b; // write two nibbles
                printf("subq %s, %s\n", get_reg_name(a), get_reg_name(b));
            }
            else if (!strcmp(buf, "andq")) {
                c.mem[m++] = (char) (6 << 4) + 2;
                get_tok(buf, f); // should be a "ra,"
                buf[strlen(buf) - 1] = '\0'; // remove trailing','
                a = get_reg(buf);
                get_tok(buf, f); // should be a "rb"
                b = get_reg(buf);
                c.mem[m++] = (a << 4) + b; // write two nibbles
                printf("andq %s, %s\n", get_reg_name(a), get_reg_name(b));
            }
            else if (!strcmp(buf, "xorq")) { 
                c.mem[m++] = (char) (6 << 4) + 3;
                get_tok(buf, f); // should be a "ra,"
                buf[strlen(buf) - 1] = '\0'; // remove trailing','
                a = get_reg(buf);
                get_tok(buf, f); // should be a "rb"
                b = get_reg(buf);
                c.mem[m++] = (a << 4) + b; // write two nibbles
                printf("xorq %s, %s\n", get_reg_name(a), get_reg_name(b));
            }
            else if (!strcmp(buf, "jmp")) {
                c.mem[m++] = (char) (7 << 4) + 0;
                get_tok(buf, f);
                if (resolve_label(buf) >= 0) num = resolve_label(buf);
                else if (buf[0] == '0' && buf[1] == 'x') num = strtol(buf + 2, NULL, 16);
                else num = strtol(buf, NULL, 10);
                memcpy(&c.mem[m], (unsigned char *)&num, 8);
                m += 8;
                printf("jmp %s # 0x%lx\n", buf, num);
            }
            else if (!strcmp(buf, "jle")) {
                c.mem[m++] = (char) (7 << 4) + 1;
                get_tok(buf, f);
                if (resolve_label(buf) >= 0) num = resolve_label(buf);
                else if (buf[0] == '0' && buf[1] == 'x') num = strtol(buf + 2, NULL, 16);
                else num = strtol(buf, NULL, 10);
                memcpy(&c.mem[m], (unsigned char *)&num, 8);
                m += 8;
                printf("jle %s # 0x%lx\n", buf, num);
            }
            else if (!strcmp(buf, "jl")) {
                c.mem[m++] = (char) (7 << 4) + 2;
                get_tok(buf, f);
                if (resolve_label(buf) >= 0) num = resolve_label(buf);
                else if (buf[0] == '0' && buf[1] == 'x') num = strtol(buf + 2, NULL, 16);
                else num = strtol(buf, NULL, 10);
                memcpy(&c.mem[m], (unsigned char *)&num, 8);
                m += 8;
                printf("jl %s # 0x%lx\n", buf, num);
            }
            else if (!strcmp(buf, "je")) {
                c.mem[m++] = (char) (7 << 4) + 3;
                get_tok(buf, f);
                if (resolve_label(buf) >= 0) num = resolve_label(buf);
                else if (buf[0] == '0' && buf[1] == 'x') num = strtol(buf + 2, NULL, 16);
                else num = strtol(buf, NULL, 10);
                memcpy(&c.mem[m], (unsigned char *)&num, 8);
                m += 8;
                printf("je %s # 0x%lx\n", buf, num);
            }
            else if (!strcmp(buf, "jne")) {
                c.mem[m++] = (char) (7 << 4) + 4;
                get_tok(buf, f);
                if (resolve_label(buf) >= 0) num = resolve_label(buf);
                else if (buf[0] == '0' && buf[1] == 'x') num = strtol(buf + 2, NULL, 16);
                else num = strtol(buf, NULL, 10);
                memcpy(&c.mem[m], (unsigned char *)&num, 8);
                m += 8;
                printf("jne %s # 0x%lx\n", buf, num);
            }
            else if (!strcmp(buf, "jge")) {
                c.mem[m++] = (char) (7 << 4) + 5;
                get_tok(buf, f);
                if (resolve_label(buf) >= 0) num = resolve_label(buf);
                else if (buf[0] == '0' && buf[1] == 'x') num = strtol(buf + 2, NULL, 16);
                else num = strtol(buf, NULL, 10);
                memcpy(&c.mem[m], (unsigned char *)&num, 8);
                m += 8;
                printf("jge %s # 0x%lx\n", buf, num);
            }
            else if (!strcmp(buf, "jg")) {
                c.mem[m++] = (char) (7 << 4) + 6;
                get_tok(buf, f);
                if (resolve_label(buf) >= 0) num = resolve_label(buf);
                else if (buf[0] == '0' && buf[1] == 'x') num = strtol(buf + 2, NULL, 16);
                else num = strtol(buf, NULL, 10);
                memcpy(&c.mem[m], (unsigned char *)&num, 8);
                m += 8;
                printf("jg %s # 0x%lx\n", buf, num);
            }
            else if (!strcmp(buf, "cmovle")) {
                c.mem[m++] = (char) (2 << 4) + 1;
                get_tok(buf, f); // should be a "ra,"
                buf[strlen(buf) - 1] = '\0'; // remove trailing','
                a = get_reg(buf);
                get_tok(buf, f); // should be a "rb"
                b = get_reg(buf);
                c.mem[m++] = (a << 4) + b; // write two nibbles
                printf("cmovle %s, %s\n", get_reg_name(a), get_reg_name(b));
            }
            else if (!strcmp(buf, "cmovl")) {
                c.mem[m++] = (char) (2 << 4) + 2;
                get_tok(buf, f); // should be a "ra,"
                buf[strlen(buf) - 1] = '\0'; // remove trailing','
                a = get_reg(buf);
                get_tok(buf, f); // should be a "rb"
                b = get_reg(buf);
                c.mem[m++] = (a << 4) + b; // write two nibbles
                printf("cmovl %s, %s\n", get_reg_name(a), get_reg_name(b));
            }
            else if (!strcmp(buf, "cmove")) {
                c.mem[m++] = (char) (2 << 4) + 3;
                buf[strlen(buf) - 1] = '\0'; // remove trailing','
                a = get_reg(buf);
                get_tok(buf, f); // should be a "rb"
                b = get_reg(buf);
                c.mem[m++] = (a << 4) + b; // write two nibbles
                printf("cmove %s, %s\n", get_reg_name(a), get_reg_name(b));
            }
            else if (!strcmp(buf, "cmovne")) {
                c.mem[m++] = (char) (2 << 4) + 4;
                buf[strlen(buf) - 1] = '\0'; // remove trailing','
                a = get_reg(buf);
                get_tok(buf, f); // should be a "rb"
                b = get_reg(buf);
                c.mem[m++] = (a << 4) + b; // write two nibbles
                printf("cmovne %s, %s\n", get_reg_name(a), get_reg_name(b));
            }
            else if (!strcmp(buf, "cmovge")) {
                c.mem[m++] = (char) (2 << 4) + 5;
                buf[strlen(buf) - 1] = '\0'; // remove trailing','
                a = get_reg(buf);
                get_tok(buf, f); // should be a "rb"
                b = get_reg(buf);
                c.mem[m++] = (a << 4) + b; // write two nibbles
                printf("cmovge %s, %s\n", get_reg_name(a), get_reg_name(b));
            }
            else if (!strcmp(buf, "cmovg")) {
                c.mem[m++] = (char) (2 << 4) + 6;
                buf[strlen(buf) - 1] = '\0'; // remove trailing','
                a = get_reg(buf);
                get_tok(buf, f); // should be a "rb"
                b = get_reg(buf);
                c.mem[m++] = (a << 4) + b; // write two nibbles
                printf("cmovg %s, %s\n", get_reg_name(a), get_reg_name(b));
            }
            else if (!strcmp(buf, "call")) {
                c.mem[m++] = (char) (8 << 4) + 0;
                get_tok(buf, f);
                if (resolve_label(buf) >= 0) num = resolve_label(buf);
                else if (buf[0] == '0' && buf[1] == 'x') num = strtol(buf + 2, NULL, 16);
                else num = strtol(buf, NULL, 10);
                memcpy(&c.mem[m], (unsigned char *)&num, 8);
                m += 8;
                printf("call %s # 0x%lx\n", buf, num);
            }
            else if (!strcmp(buf, "ret")) {
                c.mem[m++] = (char) (9 << 4) + 0;
                printf("ret\n");
            }
            else if (!strcmp(buf, "pushq")) {
                c.mem[m++] = (char) (10 << 4) + 0;
                get_tok(buf, f);
                a = get_reg(buf);
                c.mem[m++] = (a << 4) + 15; // write two nibbles
                printf("pushq %s\n", get_reg_name(a));
            }
            else if (!strcmp(buf, "popq")) {
                c.mem[m++] = (char) (11 << 4) + 0;
                get_tok(buf, f);
                a = get_reg(buf);
                c.mem[m++] = (a << 4) + 15; // write two nibbles
                printf("popq %s\n", get_reg_name(a));
            }
            
            i = 0, j = 0, num = 0, off = 0;
        }
    }

    fclose(f);

    // step 2 - execute instructions

    for (i = 0; i < MEMSIZE; i++) {
        
    }

    print_cpu(&c);

    return 0;
}
