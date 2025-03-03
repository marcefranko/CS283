#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/wait.h>
#include "dshlib.h"

/*
 * Implement your exec_local_cmd_loop function by building a loop that prompts the 
 * user for input.  Use the SH_PROMPT constant from dshlib.h and then
 * use fgets to accept user input.
 * 
 *      while(1){
 *        printf("%s", SH_PROMPT);
 *        if (fgets(cmd_buff, ARG_MAX, stdin) == NULL){
 *           printf("\n");
 *           break;
 *        }
 *        //remove the trailing \n from cmd_buff
 *        cmd_buff[strcspn(cmd_buff,"\n")] = '\0';
 * 
 *        //IMPLEMENT THE REST OF THE REQUIREMENTS
 *      }
 * 
 *   Also, use the constants in the dshlib.h in this code.  
 *      SH_CMD_MAX              maximum buffer size for user input
 *      EXIT_CMD                constant that terminates the dsh program
 *      SH_PROMPT               the shell prompt
 *      OK                      the command was parsed properly
 *      WARN_NO_CMDS            the user command was empty
 *      ERR_TOO_MANY_COMMANDS   too many pipes used
 *      ERR_MEMORY              dynamic memory management failure
 * 
 *   errors returned
 *      OK                     No error
 *      ERR_MEMORY             Dynamic memory management failure
 *      WARN_NO_CMDS           No commands parsed
 *      ERR_TOO_MANY_COMMANDS  too many pipes used
 *   
 *   console messages
 *      CMD_WARN_NO_CMD        print on WARN_NO_CMDS (C)
 *      CMD_ERR_PIPE_LIMIT     print on ERR_TOO_MANY_COMMANDS
 *      CMD_ERR_EXECUTE        print on execution failure of external command
 * 
 *  Standard Library Functions You Might Want To Consider Using (assignment 1+)
 *      malloc(), free(), strlen(), fgets(), strcspn(), printf()
 * 
 *  Standard Library Functions You Might Want To Consider Using (assignment 2+)
 *      fork(), execvp(), exit(), chdir()
 */
int dollar_questionmark = 0; // dollar_questionmark -> $?  LOL

int exec_local_cmd_loop() {
    char cmd_buff[SH_CMD_MAX];
    int rc = 0, cmd_return = 0;
    cmd_buff_t cmd;

    if (alloc_cmd_buff(&cmd) != OK)
        return ERR_MEMORY;

    while (1) {
        printf("%s", SH_PROMPT);
        if (fgets(cmd_buff, SH_CMD_MAX, stdin) == NULL) { // this is like input() in python. stores what is read in cmd_buff, reads maximum ARG_MAX chars, it's reading from stdin
            printf("\n");
            break;
        }
        //remove the trailing \n from cmd_buff
        cmd_buff[strcspn(cmd_buff, "\n")] = '\0';
        cmd_return = build_cmd_buff(cmd_buff, &cmd);
		if (cmd_return == WARN_NO_CMDS) {
			printf(CMD_WARN_NO_CMD);
			continue;
		} 
		else if (cmd_return == ERR_CMD_ARGS_BAD) {
			fprintf(stderr, "command error: Too many arguments provided\n");
			continue;
		}
		
		if (match_command(cmd.argv[0]) == BI_NOT_BI)
			exec_cmd(&cmd); // Delete
		else 
			exec_built_in_cmd(&cmd);
    }

    return OK;
}

int alloc_cmd_buff(cmd_buff_t *cmd_buff) {
    cmd_buff->_cmd_buffer = malloc(SH_CMD_MAX);
    if (!cmd_buff->_cmd_buffer)
        return ERR_MEMORY;
    
    cmd_buff->argc = 0;
    for (int i = 0; i < CMD_ARGV_MAX; i++)
		cmd_buff->argv[i] = NULL;
	
	return OK;
}

int build_cmd_buff(char *cmd_line, cmd_buff_t *cmd_buff) {
	if (strcmp(cmd_line, "\0") == 0 || strspn(cmd_line, " ") == strlen(cmd_line))
		return WARN_NO_CMDS;

	int argc, char_ind;
	char token[ARG_MAX], *character;
	bool in_quote, in_char;

    argc = 0;
    char_ind = 0;
    character = cmd_line;
    in_quote = false;
    in_char = false;
	while (*character != '\0') {
		if (in_quote) {
			if (*character == '"') {
				in_quote = false;
				token[char_ind] = '\0';
				if (char_ind > 0) {
					if (argc >= CMD_ARGV_MAX - 1) 
                        return ERR_CMD_ARGS_BAD;

					cmd_buff->argv[argc++] = strdup(token);
				}
				char_ind = 0;
				in_char = false;
			} else {
				token[char_ind++] = *character;
			}
		} else {
			if (*character == '"') {
				in_quote = true;
				in_char = true;
			} else if (*character == SPACE_CHAR) {
				if (in_char) {
					token[char_ind] = '\0';
					if (argc >= CMD_ARGV_MAX - 1) return ERR_CMD_ARGS_BAD;
					cmd_buff->argv[argc++] = strdup(token);
					char_ind = 0;
					in_char = false;
				}
			} else {
				in_char = true;
				token[char_ind++] = *character;
			}
		}
		character++;
	}
	if (in_quote || in_char) {
		token[char_ind] = '\0';
		if (argc < CMD_ARGV_MAX - 1) cmd_buff->argv[argc++] = strdup(token);
	}
	cmd_buff->argv[argc] = NULL;
	cmd_buff->argc = argc;
	cmd_buff->_cmd_buffer = cmd_line;
	return OK;
}

Built_In_Cmds match_command(const char *input) {
	if (strcmp(input, EXIT_CMD) == 0) 
        return BI_CMD_EXIT;
	else if (strcmp(input, "dragon") == 0) 
        return BI_CMD_DRAGON;
	else if (strcmp(input, "cd") == 0) 
        return BI_CMD_CD;
	else if (strcmp(input, "rc") == 0) 
        return BI_RC; 
	else
        return BI_NOT_BI;
}

Built_In_Cmds exec_built_in_cmd(cmd_buff_t *cmd) {
	Built_In_Cmds command = match_command(cmd->argv[0]);
	if (command == BI_CMD_EXIT) 
		exit(0);
	else if (command == BI_CMD_DRAGON) {
		if (cmd->argc > 1) {
			fprintf(stderr, "dragon: too many arguments\n");
			dollar_questionmark = ERR_CMD_ARGS_BAD;
			return ERR_CMD_ARGS_BAD;
		}
		print_dragon();
		dollar_questionmark = OK;
		return BI_EXECUTED;
	}
	else if (command == BI_CMD_CD) {
		if (cmd->argc == 2) {
			if (chdir(cmd->argv[1]) != 0) {
				fprintf(stderr, "cd: '%s' does not exists\n", cmd->argv[1]);
				dollar_questionmark = ERR_EXEC_CMD;
				return ERR_EXEC_CMD;
			}
		} else if (cmd->argc > 2) {
			fprintf(stderr, "cd: too many arguments\n");
			dollar_questionmark = ERR_CMD_ARGS_BAD;
			return ERR_CMD_ARGS_BAD;
		} 
		dollar_questionmark = OK;
		return BI_EXECUTED;
	}
	else if (command == BI_RC) {
		printf("%d\n", dollar_questionmark);
		return BI_EXECUTED;
	}
	else
		return BI_NOT_BI;
}

int exec_cmd(cmd_buff_t *cmd) {
	int c_result;
	pid_t f_result = fork();
	if (f_result < 0) {
		perror("fork error");
		return ERR_EXEC_CMD;
	}
	else if (f_result == 0) {
		execvp(cmd->argv[0], cmd->argv);
		int err = errno;
		if (err == ENOENT) 
			fprintf(stderr, "%s: command not found\n", cmd->argv[0]);
		else if (err == EACCES)
			fprintf(stderr, "%s: permission denied\n", cmd->argv[0]);
		else 
			perror("execvp failed");

		exit(err);
	}
	else {
		waitpid(f_result, &c_result, 0);
		dollar_questionmark = WEXITSTATUS(c_result);
		return c_result;
	}
}