#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "dshlib.h"

/*
 *  build_cmd_list
 *    cmd_line:     the command line from the user
 *    clist *:      pointer to clist structure to be populated
 *
 *  This function builds the command_list_t structure passed by the caller
 *  It does this by first splitting the cmd_line into commands by spltting
 *  the string based on any pipe characters '|'.  It then traverses each
 *  command.  For each command (a substring of cmd_line), it then parses
 *  that command by taking the first token as the executable name, and
 *  then the remaining tokens as the arguments.
 *
 *  NOTE your implementation should be able to handle properly removing
 *  leading and trailing spaces!
 *
 *  errors returned:
 *
 *    OK:                      No Error
 *    ERR_TOO_MANY_COMMANDS:   There is a limit of CMD_MAX (see dshlib.h)
 *                             commands.
 *    ERR_CMD_OR_ARGS_TOO_BIG: One of the commands provided by the user
 *                             was larger than allowed, either the
 *                             executable name, or the arg string.
 *
 *  Standard Library Functions You Might Want To Consider Using
 *      memset(), strcmp(), strcpy(), strtok(), strlen(), strchr()
 */
int build_cmd_list(char *cmd_line, command_list_t *clist)
{
    // check if the command is empty or if it is just a bunch of space chars
    if (strcmp(cmd_line, "\0") == 0 || strspn(cmd_line, " ") == strlen(cmd_line))
        return WARN_NO_CMDS;
    
    char *command_part;
    char *execute; // pointer to execute command
    char *arguments;
    int command_length;
    int bytes_read;
    int command_count;

    command_count = 0;
    bytes_read = 0;
    command_length = strlen(cmd_line);
    // printf("command_length: %d\n", command_length); // Delete
    command_part = strtok(cmd_line, PIPE_STRING);
    while (command_part != NULL) {
        if (command_count >= CMD_MAX)
            return ERR_TOO_MANY_COMMANDS;

        for (int i = strlen(command_part) - 1; command_part[i] == SPACE_CHAR; i--) {
            command_part[i + 1] = SPACE_CHAR;
            command_part[i] = '\0'; 
        }

        bytes_read += strlen(command_part) + 1;
        // printf("bytes_read: %d\n", bytes_read); // Delete
        execute = strtok(command_part, " ");
        if (strlen(execute) > EXE_MAX)
            return ERR_CMD_OR_ARGS_TOO_BIG;
        
        strcpy(clist->commands[command_count].exe, execute);
        arguments = strtok(NULL, "");
        if (arguments != NULL) {
            if (strlen(arguments) > ARG_MAX)
                return ERR_CMD_OR_ARGS_TOO_BIG;
    
            strcpy(clist->commands[command_count].args, arguments);
        } else
            strcpy(clist->commands[command_count].args, "\0");
         
        command_count++;
        if (bytes_read < command_length) {
            // printf("if executed: %s\n", cmd_line + bytes_read); // Delete
            command_part = strtok(cmd_line + bytes_read, PIPE_STRING);
        } else {
            // printf("else executed:\n"); // Delete
            command_part = NULL;
        }

        // printf("command_part: %s\n", command_part); // Delete
    } 

    clist->num = command_count;
    return OK;
}