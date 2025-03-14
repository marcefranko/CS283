#ifndef __DSHLIB_H__
    #define __DSHLIB_H__


//Constants for command structure sizes
#define EXE_MAX 64
#define ARG_MAX 256
#define CMD_MAX 8
#define CMD_ARGV_MAX (CMD_MAX + 1)
// Longest command that can be read from the shell
#define SH_CMD_MAX EXE_MAX + ARG_MAX

typedef struct command
{
    char exe[EXE_MAX];
    char args[ARG_MAX];
} command_t;

typedef struct cmd_buff
{
    int  argc;
    char *argv[CMD_ARGV_MAX];
    char *_cmd_buffer;
} cmd_buff_t;

/* WIP - Move to next assignment 
#define N_ARG_MAX    15     //MAX number of args for a command
typedef struct command{
    char exe [EXE_MAX];
    char args[ARG_MAX];
    int  argc;
    char *argv[N_ARG_MAX + 1];  //last argv[LAST] must be \0
}command_t;
*/

typedef struct command_list{
    int num;
    cmd_buff_t commands[CMD_MAX];
}command_list_t;

//Special character #defines
#define SPACE_CHAR  ' '
#define PIPE_CHAR   '|'
#define PIPE_STRING "|"

#define SH_PROMPT "dsh3> "
#define EXIT_CMD "exit"
#define EXIT_SC     99

//Standard Return Codes
#define OK                       0
#define WARN_NO_CMDS            -1
#define ERR_TOO_MANY_COMMANDS   -2
#define ERR_CMD_OR_ARGS_TOO_BIG -3
#define ERR_CMD_ARGS_BAD        -4      //for extra credit
#define ERR_MEMORY              -5
#define ERR_EXEC_CMD            -6
#define OK_EXIT                 -7

//prototypes
int alloc_cmd_buff(cmd_buff_t *cmd_buff); // c
int free_cmd_buff(cmd_buff_t *cmd_buff); // c
int clear_cmd_buff(cmd_buff_t *cmd_buff); // nn
int build_cmd_buff(char *cmd_line, cmd_buff_t *cmd_buff); // c
int close_cmd_buff(cmd_buff_t *cmd_buff); // nn
int build_cmd_list(char *cmd_line, command_list_t *clist);
int free_cmd_list(command_list_t *cmd_lst); // c

//built in command stuff
typedef enum {
    BI_CMD_EXIT, // c
    BI_CMD_DRAGON, // c
    BI_CMD_CD, // c
    BI_NOT_BI, // c
    BI_EXECUTED, // c
    BI_RC, // c
} Built_In_Cmds;
Built_In_Cmds match_command(const char *input); 
Built_In_Cmds exec_built_in_cmd(cmd_buff_t *cmd);

//main execution context
int exec_local_cmd_loop(); // c
int exec_cmd(cmd_buff_t *cmd); // c
int execute_pipeline(command_list_t *clist);




//output constants
#define CMD_OK_HEADER       "PARSED COMMAND LINE - TOTAL COMMANDS %d\n"
#define CMD_WARN_NO_CMD     "warning: no commands provided\n"
#define CMD_ERR_PIPE_LIMIT  "error: piping limited to %d commands\n"

#endif