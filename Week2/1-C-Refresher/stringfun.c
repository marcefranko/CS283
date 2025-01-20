#include <stdio.h>
#include <string.h>
#include <stdlib.h>


#define BUFFER_SZ 50

//prototypes
void usage(char *);
void print_buff(char *, int);
int  setup_buff(char *, char *, int);

//prototypes for functions to handle required functionality
int  count_words(char *, int, int);
//add additional prototypes here


int setup_buff(char *buff, char *user_str, int len){
    //TODO: #4:  Implement the setup buff as per the directions
    char *ptr;  // This will be the pointer to the first character of the user_str
    char *ptr_to_buff;  // This will be a pointer to the buff
    int whitespace_counted; // This will check for double whitespaces in the user_str
    int end_of_string;  // This checks if we already reached the '\0' in the user string
    int user_str_len;   // This will be the length of the user_str after being copied in the buffer

    ptr = user_str;
    ptr_to_buff = buff;
    whitespace_counted = 0;
    end_of_string = 0;
    user_str_len = 0;

    // Copying the user string to the buffer without extra whitespace:
    for (int i = 0; i < len; i++) {
        if (*ptr == '\0' || end_of_string) {
            end_of_string = 1;
            *ptr_to_buff = '.';
        } else if (*ptr == ' ' || *ptr == '\t') {
            if (whitespace_counted) {
                ptr++;
                continue;
            } else {
                *ptr_to_buff = ' ';
                whitespace_counted = 1;
                user_str_len++;
            }
        } else {
            *ptr_to_buff = *ptr;
            whitespace_counted = 0;
            user_str_len++;
        }
        ptr_to_buff++;
        ptr++;
    }

    // Checking if user string is more than 50 chars:
    // To do that, we check if we have reached the end of the string with !end_of_string
    // However, end_of_string will be true (or 1) if the user_str is <= than 49 characters, and we
    // are allowing up to 50 characters per user str (not including the '\0' character)
    // That is why we also check the pointer to the user string: it is currently pointing to the 51th value
    // of the user_string
    // If the value of this pointer is not the null character, then the user_str is more than 50 characters
    if (!end_of_string && *ptr != '\0') {
        return -1;  // User input overflows
    }

    return user_str_len; // Return the length of the user_str. 
}

void print_buff(char *buff, int len){
    printf("Buffer:  ");
    for (int i=0; i<len; i++){
        putchar(*(buff+i));
    }
    putchar('\n');
}

void usage(char *exename){
    printf("usage: %s [-h|c|r|w|x] \"string\" [other args]\n", exename);
}

int count_words(char *buff, int len, int str_len) {
    //YOU MUST IMPLEMENT
    char *buff_pointer; // Pointer to the buffer (I tried just using buff but it didn't work)
    int word_count;
    int word_start; // This variable keeps track if we are inside a word or not

    buff_pointer = buff;
    word_count = 0;
    word_start = 0;

    for (int i = 0; i < str_len; i++) {
        if (!word_start) {
            if (*buff_pointer == ' ') 
                continue;
            else {
                word_count++;
                word_start = 1;
            }
        } else {
            if (*buff_pointer == ' ')
                word_start = 0;
        }
        buff_pointer++;
    }

    return word_count;
}

//ADD OTHER HELPER FUNCTIONS HERE FOR OTHER REQUIRED PROGRAM OPTIONS

int main(int argc, char *argv[]){

    char *buff;             //placehoder for the internal buffer
    char *input_string;     //holds the string provided by the user on cmd line
    char opt;               //used to capture user option from cmd line
    int  rc;                //used for return codes
    int  user_str_len;      //length of user supplied string

    //TODO:  #1. WHY IS THIS SAFE, aka what if arv[1] does not exist?
    //      This block of code is checking if the user input meets the correct format
    //      In this case, we are checking if the number of arguments is at least 3, and if the first character
    //      of the second argument is a '-'.
    //      So, if argv[1] does not exist, the program will print the correct input format, and exit
    //      This is safe because it prevents the program from crashing, or accessing memory that may not belong 
    //      to the program.
    if ((argc < 2) || (*argv[1] != '-')){
        usage(argv[0]);
        exit(1);
    }

    opt = (char)*(argv[1]+1);   //get the option flag

    //handle the help flag and then exit normally
    if (opt == 'h'){
        usage(argv[0]);
        exit(0);
    }

    //WE NOW WILL HANDLE THE REQUIRED OPERATIONS

    //TODO:  #2 Document the purpose of the if statement below
    //      PLACE A COMMENT BLOCK HERE EXPLAINING
    if (argc < 3){
        usage(argv[0]);
        exit(1);
    }

    input_string = argv[2]; //capture the user input string

    //TODO:  #3 Allocate space for the buffer using malloc and
    //          handle error if malloc fails by exiting with a 
    //          return code of 99
    // CODE GOES HERE FOR #3
    buff = malloc(BUFFER_SZ);
    // Checking if the memory allocation was successful:
    if (buff == NULL) {
        printf("Memory allocation failure.\n");
        return -2;  // Returning a Memory allocation failure: -2
    }
    user_str_len = setup_buff(buff, input_string, BUFFER_SZ);     //see todos

    if (user_str_len < 0){
        printf("Error setting up buffer, error = %d\n", user_str_len);
        exit(2);
    }

    switch (opt){
        case 'c':
            rc = count_words(buff, BUFFER_SZ, user_str_len);  //you need to implement
            if (rc < 0){
                printf("Error counting words, rc = %d", rc);
                exit(2);
            }
            printf("Word Count: %d\n", rc);
            break;

        //TODO:  #5 Implement the other cases for 'r' and 'w' by extending
        //       the case statement options
        default:
            usage(argv[0]);
            exit(1);
    }

    //TODO:  #6 Dont forget to free your buffer before exiting
    print_buff(buff,BUFFER_SZ);
    exit(0);
}

//TODO:  #7  Notice all of the helper functions provided in the 
//          starter take both the buffer as well as the length.  Why
//          do you think providing both the pointer and the length
//          is a good practice, after all we know from main() that 
//          the buff variable will have exactly 50 bytes?
//  
//          PLACE YOUR ANSWER HERE