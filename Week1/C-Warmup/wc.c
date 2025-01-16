#include <stdio.h>

int main() {
    int nc = 0, nl = 0, nw = 0;
    int c;
    int inword = 0;
    while ((c = getchar()) != EOF) {
        nc++;
        if (c == '\n')
            nl++;
        if (c == ' ' || c == '\t' || c == '\n') {
            if (inword == 1) {  // You can just say if (inword)
                nw++;
                inword = 0;
            }
        } else {
            inword = 1;
        }
    }
          
    printf("%d\t%d\t%d\n", nl, nw, nc);
}