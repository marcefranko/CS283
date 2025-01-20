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

unsigned long mem[NMEM];

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
    printf("nwords: %d\n", nwords);
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
    }
}
