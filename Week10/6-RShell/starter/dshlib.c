#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>

#include "dshlib.h"



#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>

#include "dshlib.h"

int alloc_cmd_buff(cmd_buff_t *cmd_buff) {
	cmd_buff->_cmd_buffer = malloc(SH_CMD_MAX);
	if (!cmd_buff->_cmd_buffer) {
		return ERR_MEMORY;
	}

	cmd_buff->argc = 0;
	for (int i = 0; i < CMD_ARGV_MAX; i++) {
		cmd_buff->argv[i] = NULL;
	}

    cmd_buff->input_file = NULL;
    cmd_buff->output_file = NULL;
    cmd_buff->append = false;

	return OK;
}

int free_cmd_buff(cmd_buff_t *cmd_buff) {
	if (cmd_buff == NULL) {
		fprintf(stderr, "free_cmd_buff error: cmd_buff is NULL\n");
		return ERR_MEMORY;
	}

	if (cmd_buff->_cmd_buffer) {
		free(cmd_buff->_cmd_buffer);
		cmd_buff->_cmd_buffer = NULL;
	}

	for (int i = 0; i < cmd_buff->argc; i++) {
		if (cmd_buff->argv[i]) {
			free(cmd_buff->argv[i]);
			cmd_buff->argv[i] = NULL;
		}
	}

    if (cmd_buff->input_file) {
        free(cmd_buff->input_file);
        cmd_buff->input_file = NULL;
    }

    if (cmd_buff->output_file) {
        free(cmd_buff->output_file);
        cmd_buff->output_file = NULL;
    }
    
	cmd_buff->argc = 0;
	return OK;
}

int clear_cmd_buff(cmd_buff_t *cmd_buff) {
	for (int i = 0; i < cmd_buff->argc; i++) {
		if (cmd_buff->argv[i]) {
			free(cmd_buff->argv[i]);
			cmd_buff->argv[i] = NULL;
		}
	}

	cmd_buff->argc = 0;
	if (cmd_buff->_cmd_buffer) {
		cmd_buff->_cmd_buffer[0] = '\0';
	}

	return OK;
}

int build_cmd_buff(char *cmd_line, cmd_buff_t *cmd_buff) {
	int argc = 0;
	char token[ARG_MAX];
	int token_index = 0;
	char *p = cmd_line;
	bool in_quote = false;
	bool in_token = false;

	while (*p != '\0') {
		if (in_quote) {
			if (*p == '"') {
				in_quote = false;
				token[token_index] = '\0';
                if (token_index >= ARG_MAX) {
                    fprintf(stderr, "error: argument too long\n");
                    return ERR_CMD_OR_ARGS_TOO_BIG;
                }

                if (argc == 0 && strlen(token) >= EXE_MAX) {
                    fprintf(stderr, "error: command name too long\n");
                    return ERR_CMD_OR_ARGS_TOO_BIG;
                }

				if (token_index > 0) {
					if (argc >= CMD_ARGV_MAX - 1) {
						return ERR_CMD_ARGS_BAD;
					}

					cmd_buff->argv[argc] = strdup(token);
					argc++;
				}

				token_index = 0;
				in_token = false;
			} else {
                if (token_index >= ARG_MAX - 1) {
                    fprintf(stderr, "error: argument too long\n");
                    return ERR_CMD_OR_ARGS_TOO_BIG;
                }

				token[token_index++] = *p;
			}
		} else {
			if (*p == '"') {
				in_quote = true;
				in_token = true;
			} else if (isspace((unsigned char)*p)) {
				if (in_token) {
					token[token_index] = '\0';
                    if (token_index >= ARG_MAX) {
                        fprintf(stderr, "error: argument too long\n");
                        return ERR_CMD_OR_ARGS_TOO_BIG;
                    }
    
                    if (argc == 0 && strlen(token) >= EXE_MAX) {
                        fprintf(stderr, "error: command name too long\n");
                        return ERR_CMD_OR_ARGS_TOO_BIG;
                    }

					if (argc >= CMD_ARGV_MAX - 1) {
						return  ERR_CMD_ARGS_BAD;
					}

					cmd_buff->argv[argc] = strdup(token);
					argc++;
					token_index = 0;
					in_token = false;
				}
			} else {
				in_token = true;
                if (token_index >= ARG_MAX - 1) {
                    fprintf(stderr, "error: argument too long\n");
                    return ERR_CMD_OR_ARGS_TOO_BIG;
                }

				token[token_index++] = *p;
			}
		}

		p++;
	}

	if (in_token || in_quote) {
		token[token_index] = '\0';
        if (argc == 0 && strlen(token) >= EXE_MAX) {
            fprintf(stderr, "error: command name too long\n");
            return ERR_CMD_OR_ARGS_TOO_BIG;
        }

		if (argc < CMD_ARGV_MAX - 1) {
			cmd_buff->argv[argc] = strdup(token);
			argc++;
		}
	}

	cmd_buff->argv[argc] = NULL;
	cmd_buff->argc = argc;

	return OK;
}

int close_cmd_buff(cmd_buff_t *cmd_buff) {
	return free_cmd_buff(cmd_buff);
}

char *expand_path(const char *path) {
    if (path[0] == '~') {
        const char *home = getenv("HOME");
        if (!home) {
            fprintf(stderr, "Error: HOME environment variable not set.\n");
            return strdup(path);
        }

        size_t len = strlen(home) + strlen(path); 
        char *expanded = malloc(len);
        if (!expanded) {
            perror("malloc");
            exit(EXIT_FAILURE);
        }

        sprintf(expanded, "%s%s", home, path + 1);
        return expanded;
    }

    return strdup(path);
}

int process_redirection(cmd_buff_t *cmd) {
	int i = 0, j = 0;
	while (cmd->argv[i] != NULL) {
		if (strcmp(cmd->argv[i], INPUT_REDIRECT) == 0) {
			if (cmd->argv[i+1] == NULL) {
				fprintf(stderr, "redirect: missing input file for redirection\n");
				return ERR_CMD_ARGS_BAD;
			}

			if (cmd->input_file != NULL) {
				fprintf(stderr, "redirect: multiple input redirection operators\n");
				return ERR_CMD_ARGS_BAD;
			}

			cmd->input_file = cmd->argv[i+1];
            free(cmd->argv[i]);
			i += 2;
		} else if (strcmp(cmd->argv[i], OUTPUT_REDIRECT) == 0) {
			if (cmd->argv[i+1] == NULL) {
				fprintf(stderr, "redirect: missing output file for redirection '>'\n");
				return ERR_CMD_ARGS_BAD;
			}

			if (cmd->output_file != NULL) {
				fprintf(stderr, "redirect: multiple output redirection operators\n");
				return ERR_CMD_ARGS_BAD;
			}
			
			cmd->output_file = strdup(cmd->argv[i+1]);
			cmd->append = false;
			i += 2;
		} else if (strcmp(cmd->argv[i], APPEND_REDIRECT) == 0) {
			if (cmd->argv[i+1] == NULL) {
				fprintf(stderr, "redirect: missing output file for redirection '>>'\n");
				return ERR_CMD_ARGS_BAD;
			}

			if (cmd->output_file != NULL) {
				fprintf(stderr, "redirect: multiple output redirection operators\n");
				return ERR_CMD_ARGS_BAD;
			}

			cmd->output_file = strdup(cmd->argv[i+1]);
			cmd->append = true;
			i += 2;
		} else {
			cmd->argv[j++] = cmd->argv[i++];
		}
	}

	cmd->argv[j] = NULL;
	cmd->argc = j;

	return OK;
}

int build_cmd_list(char *cmd_line, command_list_t *clist) {
	char *saveptr;
	int count = 0;
	char *token = strtok_r(cmd_line, PIPE_STRING, &saveptr);

	while (token != NULL) {
		while (isspace((unsigned char)*token)) {
			token++;
		}

		if (*token == '\0') {
            for (int i = 0; i < count; i++) {
                free_cmd_buff(&clist->commands[i]);
            }

            return WARN_NO_CMDS;
		}

		if (count >= CMD_MAX) {
            for (int i = 0; i < count; i++) {
                free_cmd_buff(&clist->commands[i]);
            }
            
			return ERR_TOO_MANY_COMMANDS;
		}

		int rc = alloc_cmd_buff(&clist->commands[count]);
		if (rc != OK) {
			return rc;
		}

		strncpy(clist->commands[count]._cmd_buffer, token, SH_CMD_MAX);
		clist->commands[count]._cmd_buffer[SH_CMD_MAX - 1] = '\0';
		rc = build_cmd_buff(clist->commands[count]._cmd_buffer, &clist->commands[count]);
		if (rc != OK) {
			return rc;
		}

		rc = process_redirection(&clist->commands[count]);
        if (rc != OK) {
            return rc;
        }

        if (clist->commands[count].argc == 0) {
            free_cmd_buff(&clist->commands[count]);
            for (int i = 0; i < count; i++) {
                free_cmd_buff(&clist->commands[i]);
            }

            return WARN_NO_CMDS;
        }
        
		count++;
		token = strtok_r(NULL, PIPE_STRING, &saveptr);
	}

	clist->num = count;
	if (count == 0) {
		return WARN_NO_CMDS;
	}

    if (clist->num >= 3) {
        for (int i = 1; i < clist->num - 1; i++) {
            if (clist->commands[i].input_file != NULL || clist->commands[i].output_file != NULL) {
                for (int j = 0; j < clist->num; j++) {
                    free_cmd_buff(&clist->commands[j]);
                }

                fprintf(stderr, "error:redirection not allowed in intermediate commands\n");
                return ERR_CMD_ARGS_BAD;
            }
        }
    }

	return OK;
}

int free_cmd_list(command_list_t *cmd_list) {
	for (int i = 0; i < cmd_list->num; i++) {
		free_cmd_buff(&cmd_list->commands[i]);
	}

	cmd_list->num = 0;
	return OK;
}

Built_In_Cmds match_command(const char *input) {
	if (strcmp(input, EXIT_CMD) == 0)
		return BI_CMD_EXIT;

	if (strcmp(input, "cd") == 0)
		return BI_CMD_CD;

	if (strcmp(input, "dragon") == 0)
		return BI_CMD_DRAGON;

	return BI_NOT_BI;
}

Built_In_Cmds exec_built_in_cmd(cmd_buff_t *cmd) {
	Built_In_Cmds type = match_command(cmd->argv[0]);
	if (type == BI_CMD_EXIT) {
		exit(0);
	}

	if (type == BI_CMD_CD) {
		if (cmd->argc == 1) {
		} else if (cmd->argc == 2) {
			if (chdir(cmd->argv[1]) != 0) {
				perror("cd failed");
				return ERR_EXEC_CMD;
			}
		} else {
			fprintf(stderr, "cd: too many arguments\n");
			return ERR_CMD_ARGS_BAD;
		}

		return BI_EXECUTED;
	}

	if (type == BI_CMD_DRAGON) {
		if (cmd->argc > 1) {
			fprintf(stderr, "dragon: too many arguments\n");
			return ERR_CMD_ARGS_BAD;
		}

		print_dragon();
		return BI_EXECUTED;
	}

	return BI_NOT_BI;
}

int exec_cmd(cmd_buff_t *cmd) {
	pid_t pid = fork();
	if (pid < 0) {
		perror("fork");
		return ERR_EXEC_CMD;
	}

	if (pid == 0) {
		if (cmd->input_file != NULL) {
            char *input_path = expand_path(cmd->input_file);
			int fd_in = open(input_path, O_RDONLY);
			if (fd_in < 0) {
				perror("open input file");
				exit(ERR_EXEC_CMD);
			}

			dup2(fd_in, STDIN_FILENO);
			close(fd_in);
		}

		if (cmd->output_file != NULL) {
            char *output_path = expand_path(cmd->output_file);
			int fd_out;
			if (cmd->append) {
				fd_out = open(output_path, O_WRONLY | O_CREAT | O_APPEND, 0644);
			} else {
				fd_out = open(output_path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
			}

			if (fd_out < 0) {
				perror("open output file");
				exit(ERR_EXEC_CMD);
			}

			dup2(fd_out, STDOUT_FILENO);
			close(fd_out);
		}

		execvp(cmd->argv[0], cmd->argv);
		perror("execvp");
		exit(ERR_EXEC_CMD);
	}

	int status;
	waitpid(pid, &status, 0);

	return status;
}

int execute_pipeline(command_list_t *clist) {
	int num = clist->num;
	pid_t pids[CMD_MAX];
	int prev_pipe_fd[2] = {-1, -1};

	for (int i = 0; i < num; i++) {
		int curr_pipe_fd[2];
		if (i < num - 1) {
			if (pipe(curr_pipe_fd) < 0) {
				perror("pipe");
				return ERR_EXEC_CMD;
			}
		}

		pids[i] = fork();
		if (pids[i] < 0) {
			perror("fork");
			return  ERR_EXEC_CMD;
		}

		if (pids[i] == 0) {
			if (i > 0) {
				dup2(prev_pipe_fd[0], STDIN_FILENO);
			}

			if (i < num - 1) {
				dup2(curr_pipe_fd[1], STDOUT_FILENO);
			}

			if (i == 0 && clist->commands[i].input_file != NULL) {
                char *input_path = expand_path(clist->commands[i].input_file);
				int fd_in = open(input_path, O_RDONLY);
				if (fd_in < 0) {
					perror("open input file");
					exit(ERR_EXEC_CMD);
				}

				dup2(fd_in, STDIN_FILENO);
				close(fd_in);
			}

			if (i == num - 1 && clist->commands[i].output_file != NULL) {
                char *output_path = expand_path(clist->commands[i].output_file);
				int fd_out;
				if (clist->commands[i].append) {
					fd_out = open(output_path, O_WRONLY | O_CREAT | O_APPEND, 0644);
				} else {
					fd_out = open(output_path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
				}

				if (fd_out < 0) {
					perror("open output file");
					exit(ERR_EXEC_CMD);
				}

				dup2(fd_out, STDOUT_FILENO);
				close(fd_out);
			}

			if (i > 0) {
				close(prev_pipe_fd[0]);
				close(prev_pipe_fd[1]);
			}

			if (i < num - 1) {
				close(curr_pipe_fd[0]);
				close(curr_pipe_fd[1]);
			}

			execvp(clist->commands[i].argv[0], clist->commands[i].argv);
			perror("execvp");
			exit(ERR_EXEC_CMD);
		}

		if (i > 0) {
			close(prev_pipe_fd[0]);
			close(prev_pipe_fd[1]);
		}

		if (i < num - 1) {
			prev_pipe_fd[0] = curr_pipe_fd[0];
			prev_pipe_fd[1] = curr_pipe_fd[1];
		}
	}

	int status;
	for (int i = 0; i < num; i++) {
		waitpid(pids[i], &status, 0);
	}

	return OK;
}
int exec_local_cmd_loop()
{
	char cmd_buff[SH_CMD_MAX];
	while (1) {
		printf("%s", SH_PROMPT);
		if (fgets(cmd_buff, SH_CMD_MAX, stdin) == NULL) {
			printf("\n");
			break;
		}

        if (strchr(cmd_buff, '\n') == NULL) {
            fprintf(stderr, "error: maximum buffer size for user input is %d", SH_CMD_MAX);
            int c;
            while ((c = getchar()) != '\n' && c != EOF) { }
            
            continue;
        }

		cmd_buff[strcspn(cmd_buff, "\n")] = '\0';
		if (strlen(cmd_buff) == 0) {
			continue;
		}

		if (strcmp(cmd_buff, EXIT_CMD) == 0) {
			exit(0);
		}

		command_list_t clist;
		clist.num = 0;
		char _cmd_buff[SH_CMD_MAX];
		strncpy(_cmd_buff, cmd_buff, SH_CMD_MAX);

		int rc = build_cmd_list(_cmd_buff, &clist);
		if (rc != 0) {
			if (rc == WARN_NO_CMDS) {
				fprintf(stderr, CMD_WARN_NO_CMD);
			} else if (rc == ERR_TOO_MANY_COMMANDS) {
				fprintf(stderr, CMD_ERR_PIPE_LIMIT, CMD_MAX);
			} else {
				fprintf(stderr, "error parsing command line\n");
			}

			continue;
		}

		if (clist.num == 1) {
			Built_In_Cmds type = match_command(clist.commands[0].argv[0]);
			if (type != BI_NOT_BI) {
				exec_built_in_cmd(&clist.commands[0]);
			} else {
				exec_cmd(&clist.commands[0]);
			}
		} else {
			execute_pipeline(&clist);
		}

		free_cmd_list(&clist);
	}

	return OK;
}