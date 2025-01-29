#include <stdio.h>
#include <stdlib.h> // contains exit() function
#include <unistd.h> // contains read(), close()
#include <fcntl.h>

// Defining Constants
enum {
    NMEM = 32768,   // Number of memory locations in a s20 machine.
    BPERW = 3   // Number of bytes in a word (In the s20 this value is 3)
};

int loadfile(char*);   // This is a prototype
void disassm(int);

// Global variable declarations:
unsigned long mem[NMEM];    // Remember: This time we are reserving NMEM LONGS!, not characters.

// argc is the number of command line arguments we will use.
// argv is an array of pointers to characters AKA array of strings (which represent the command line arguments)
int main(int argc, char* argv[]) { 
    int nwords;
    if (argc != 2) {
        fprintf(stderr, "Usage: s20dis file\n"); // printing and error
        // printf() prints to stdoutput. in this case we want to print to stderr
        // variable stderr is a file pointer. a pointer to an open file.
        exit(1);
    }
    nwords = loadfile(argv[1]);
    disassm(nwords);

    exit(0);  
}

int loadfile(char* fn) {
    int fd; // This is the file descriptor
    int n;  // number of bytes read() read
    int nword;
    unsigned char buf[NMEM * BPERW];
    unsigned char* p;
    unsigned long word; // this is the word assembler
    // Opening the file:
    fd = open(fn, O_RDONLY);    // We will be opening the file for read purposes only
    if (fd < 0) {   // Checking if there was an error
        // This prints to the stderror like the fprintf we used before
        // But it also prints some extra error information
        perror("loadfile");
        exit(2);
    }

    // Reading the file:
    n = read(fd, buf, NMEM * BPERW); // Parameters: fd, place in memory for the stuff we are reading, how many bytes you are reading
    close(fd);
    nword = n / BPERW;
    p = buf;    // We set p to point to the first thing in buf
    for (int i = 0; i < nword; i++) {
        word = *p++ << 16;
        word |= *p++ << 8;  // |= -> word = word | (something). It is a bitwise OR
        word |= *p++;       // We use |= because if we just use = we would not include *p++ << 16 or *p++ << 8
        mem[i] = word;
    }
}

void dosubop(unsigned long inst) {
    int rA, rB, rC, subop;

    rA = (inst & 0x0f8000) >> 15;
    rB = (inst & 0x007c00) >> 10;
    rC = (inst & 0x0003e0) >> 5;
    subop = inst & 0x00001f;
    switch (subop) {
    case 0x00:
        printf("nop\n");
        break;
    
    case 0x01:
        printf("ldi\tr%d, r%d, r%d\n", rA, rB, rC);
        break;
    
    case 0x02:
        printf("sti\tr%d, r%d, r%d\n", rC, rA, rB);
        break;
    
    case 0x03:
        printf("add\tr%d, r%d, r%d\n", rA, rB, rC);
        break;
    
    case 0x04:
        printf("sub\tr%d, r%d, r%d\n", rA, rB, rC);
        break;
    
    case 0x05:
        printf("and\tr%d, r%d, r%d\n", rA, rB, rC);
        break;
    
    case 0x06:
        printf("or\tr%d, r%d, r%d\n", rA, rB, rC);
        break;

    case 0x07:
        printf("xor\tr%d, r%d, r%d\n", rA, rB, rC);
        break;
    
    case 0x08:
        printf("shl\tr%d, %d, r%d\n", rA, rB, rC);  // Here, rB is treated as a constant
        break;

    case 0x09:
        printf("sal\tr%d, %d, r%d\n", rA, rB, rC);  // This is an Arithmetical shift (To the left, so it is useless)
        break;

    case 0x0a:
        printf("shr\tr%d, %d, r%d\n", rA, rB, rC);
        break;

    case 0x0b:
        printf("sar\tr%d, %d, r%d\n", rA, rB, rC);
        break;

    case 0x10:
        printf("rst\n");    // Return from subroutine
        break;
    
    case 0x1f:
        printf("halt\n");
        break;

    default:
        printf("Undefined\n");
        break;
    }
}

void disassm(int n) {
    int opcode, reg, addr;
    
    for (int i = 0; i < n; i++) {
        printf("%04x: %06lx    ", i, mem[i]);  // x is for hexadecimal
        opcode = (mem[i] & 0xf00000) >> 20; 
        reg = (mem[i] & 0x0f8000) >> 15;
        addr = mem[i] & 0x007ff;
        switch (opcode) {
        case 0x0:
            dosubop(mem[i]);    // See how we are giving the whole instruction to dosubop()
            break;
        
        case 0x1:
            printf("ld\t%04x, r%d\n", addr, reg);
            break;

        case 0x2:
            printf("st\tr%d, %04x\n", addr, reg);
            break;
        
        case 0x3:
            printf("br\t%04x\n", addr);
            break;

        case 0x4:
            printf("bsr\t%04x\n", addr);
            break;
        
        case 0x5:
            printf("brz\tr%d, %04x\n", reg, addr);
            break;

        case 0x6:
            printf("bnz\tr%d, %04x\n", reg, addr);
            break;

        case 0x7:
            printf("brn\tr%d, %04x\n", reg, addr);
            break;

        case 0x8:
            printf("bnn\tr%d, %04x\n", reg, addr);
            break;

        default:    // If neither of the conditions are met
            printf("Undefined\n");
            break;
        }
    }
}
