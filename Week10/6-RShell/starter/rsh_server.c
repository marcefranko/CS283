#include <sys/socket.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/un.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/time.h>

#include <signal.h>
#include <pthread.h>

#include "dshlib.h"
#include "rshlib.h"

typedef struct {
    int client_socket;
} client_thread_data_t;
pthread_mutex_t server_mutex = PTHREAD_MUTEX_INITIALIZER;
volatile int server_running = 1;

void* handle_client_connection(void* arg) {
    client_thread_data_t *client_data = (client_thread_data_t*)arg;
    int client_socket = client_data->client_socket;  
    int flags = fcntl(client_socket, F_GETFL, 0);
    fcntl(client_socket, F_SETFL, flags & ~O_NONBLOCK);
    
    int rc = exec_client_requests(client_socket);
    if (rc == OK_EXIT) {
        printf("Client requested server shutdown\n");
        pthread_mutex_lock(&server_mutex);
        server_running = 0;
        pthread_mutex_unlock(&server_mutex);

        int temp_socket = socket(AF_INET, SOCK_STREAM, 0);
        if (temp_socket >= 0) {
            struct sockaddr_in serv_addr;
            memset(&serv_addr, 0, sizeof(serv_addr));
            serv_addr.sin_family = AF_INET;
            serv_addr.sin_port = htons(RDSH_DEF_PORT);
            inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr);
            
            connect(temp_socket, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
            close(temp_socket);
        }
    }
    
    close(client_socket);
    free(client_data);
    
    return NULL;
}

int process_threaded_cli_requests(int svr_socket) {
    int client_socket;
    pthread_t thread_id;
    struct sockaddr_in client_addr;
    socklen_t addrlen = sizeof(client_addr);
    int rc = OK;

    server_running = 1;
    signal(SIGPIPE, SIG_IGN);
    
    struct timeval timeout;
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;
    
    if (setsockopt(svr_socket, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0) {
        perror("setsockopt timeout");
    }
    
    printf("Threaded server started. Waiting for connections...\n");
    
    while (1) {
        pthread_mutex_lock(&server_mutex);
        int should_continue = server_running;
        pthread_mutex_unlock(&server_mutex);
        
        if (!should_continue) {
            printf("Server shutting down...\n");
            break;
        }
        
        client_socket = accept(svr_socket, (struct sockaddr*)&client_addr, &addrlen);
        if (client_socket < 0) {
            if (errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK) {
                continue;
            }
            
            perror("accept");
            rc = ERR_RDSH_COMMUNICATION;
            break;
        }
        
        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(client_addr.sin_addr), client_ip, INET_ADDRSTRLEN);
        printf("New client connected from %s:%d\n", client_ip, ntohs(client_addr.sin_port));
        
        client_thread_data_t *client_data = malloc(sizeof(client_thread_data_t));
        if (!client_data) {
            close(client_socket);
            perror("malloc");
            continue;
        }
        
        client_data->client_socket = client_socket;
        
        if (pthread_create(&thread_id, NULL, handle_client_connection, client_data) != 0) {
            perror("pthread_create");
            free(client_data);
            close(client_socket);
            continue;
        }
        
        pthread_detach(thread_id);
    }
    
    return rc;
}

/*
 * start_server(ifaces, port, is_threaded)
 *      ifaces:  a string in ip address format, indicating the interface
 *              where the server will bind.  In almost all cases it will
 *              be the default "0.0.0.0" which binds to all interfaces.
 *              note the constant RDSH_DEF_SVR_INTFACE in rshlib.h
 * 
 *      port:   The port the server will use.  Note the constant 
 *              RDSH_DEF_PORT which is 1234 in rshlib.h.  If you are using
 *              tux you may need to change this to your own default, or even
 *              better use the command line override -s implemented in dsh_cli.c
 *              For example ./dsh -s 0.0.0.0:5678 where 5678 is the new port  
 * 
 *      is_threded:  Used for extra credit to indicate the server should implement
 *                   per thread connections for clients  
 * 
 *      This function basically runs the server by: 
 *          1. Booting up the server
 *          2. Processing client requests until the client requests the
 *             server to stop by running the `stop-server` command
 *          3. Stopping the server. 
 * 
 *      This function is fully implemented for you and should not require
 *      any changes for basic functionality.  
 * 
 *      IF YOU IMPLEMENT THE MULTI-THREADED SERVER FOR EXTRA CREDIT YOU NEED
 *      TO DO SOMETHING WITH THE is_threaded ARGUMENT HOWEVER.  
 */
int start_server(char *ifaces, int port, int is_threaded){
    int svr_socket;
    int rc;

    svr_socket = boot_server(ifaces, port);
    if (svr_socket < 0){
        int err_code = svr_socket;  //server socket will carry error code
        return err_code;
    }

    if (is_threaded) {
        printf("Starting multi-threaded server...\n");
        rc = process_threaded_cli_requests(svr_socket);
    } else {
        printf("Starting single-threaded server...\n");
        rc = process_cli_requests(svr_socket);
    }

    stop_server(svr_socket);
    return rc;
}

/*
 * stop_server(svr_socket)
 *      svr_socket: The socket that was created in the boot_server()
 *                  function. 
 * 
 *      This function simply returns the value of close() when closing
 *      the socket.  
 */
int stop_server(int svr_socket){
    return close(svr_socket);
}

/*
 * boot_server(ifaces, port)
 *      ifaces & port:  see start_server for description.  They are passed
 *                      as is to this function.   
 * 
 *      This function "boots" the rsh server.  It is responsible for all
 *      socket operations prior to accepting client connections.  Specifically: 
 * 
 *      1. Create the server socket using the socket() function. 
 *      2. Calling bind to "bind" the server to the interface and port
 *      3. Calling listen to get the server ready to listen for connections.
 * 
 *      after creating the socket and prior to calling bind you might want to 
 *      include the following code:
 * 
 *      int enable=1;
 *      setsockopt(svr_socket, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int));
 * 
 *      when doing development you often run into issues where you hold onto
 *      the port and then need to wait for linux to detect this issue and free
 *      the port up.  The code above tells linux to force allowing this process
 *      to use the specified port making your life a lot easier.
 * 
 *  Returns:
 * 
 *      server_socket:  Sockets are just file descriptors, if this function is
 *                      successful, it returns the server socket descriptor, 
 *                      which is just an integer.
 * 
 *      ERR_RDSH_COMMUNICATION:  This error code is returned if the socket(),
 *                               bind(), or listen() call fails. 
 * 
 */
int boot_server(char *ifaces, int port){
    int server_socket;
    struct sockaddr_in serv_addr;
    int enable = 1;

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("socket");
        return ERR_RDSH_COMMUNICATION;
    }

    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable)) < 0) {
        perror("setsockopt");
        close(server_socket);
        return ERR_RDSH_COMMUNICATION;
    }

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    if (inet_pton(AF_INET, ifaces, &serv_addr.sin_addr) <= 0) {
        perror("inet_pton");
        close(server_socket);
        return ERR_RDSH_COMMUNICATION;
    }

    if (bind(server_socket, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("bind");
        close(server_socket);
        return ERR_RDSH_COMMUNICATION;
    }

    if (listen(server_socket, 5) < 0) {
        perror("listen");
        close(server_socket);
        return ERR_RDSH_COMMUNICATION;
    }

    return server_socket;
}

/*
 * process_cli_requests(svr_socket)
 *      svr_socket:  The server socket that was obtained from boot_server()
 *   
 *  This function handles managing client connections.  It does this using
 *  the following logic
 * 
 *      1.  Starts a while(1) loop:
 *  
 *          a. Calls accept() to wait for a client connection. Recall that 
 *             the accept() function returns another socket specifically
 *             bound to a client connection. 
 *          b. Calls exec_client_requests() to handle executing commands
 *             sent by the client. It will use the socket returned from
 *             accept().
 *          c. Loops back to the top (step 2) to accept connecting another
 *             client.  
 * 
 *          note that the exec_client_requests() return code should be
 *          negative if the client requested the server to stop by sending
 *          the `stop-server` command.  If this is the case step 2b breaks
 *          out of the while(1) loop. 
 * 
 *      2.  After we exit the loop, we need to cleanup.  Dont forget to 
 *          free the buffer you allocated in step #1.  Then call stop_server()
 *          to close the server socket. 
 * 
 *  Returns:
 * 
 *      OK_EXIT:  When the client sends the `stop-server` command this function
 *                should return OK_EXIT. 
 * 
 *      ERR_RDSH_COMMUNICATION:  This error code terminates the loop and is
 *                returned from this function in the case of the accept() 
 *                function failing. 
 * 
 *      OTHERS:   See exec_client_requests() for return codes.  Note that positive
 *                values will keep the loop running to accept additional client
 *                connections, and negative values terminate the server. 
 * 
 */
int process_cli_requests(int svr_socket){
    int client_socket;
    int rc;
    struct sockaddr_in client_addr;
    socklen_t addrlen = sizeof(client_addr);

    while (1) {
        client_socket = accept(svr_socket, (struct sockaddr*)&client_addr, &addrlen);
        if (client_socket < 0) {
            perror("accept");
            return ERR_RDSH_COMMUNICATION;
        }
        
        rc = exec_client_requests(client_socket);
        close(client_socket);

        if (rc == OK_EXIT) {
            break;
        }
        
        if (rc < 0) {
            return rc;
        }
    }

    return OK_EXIT;
}

/*
 * exec_client_requests(cli_socket)
 *      cli_socket:  The server-side socket that is connected to the client
 *   
 *  This function handles accepting remote client commands. The function will
 *  loop and continue to accept and execute client commands.  There are 2 ways
 *  that this ongoing loop accepting client commands ends:
 * 
 *      1.  When the client executes the `exit` command, this function returns
 *          to process_cli_requests() so that we can accept another client
 *          connection. 
 *      2.  When the client executes the `stop-server` command this function
 *          returns to process_cli_requests() with a return code of OK_EXIT
 *          indicating that the server should stop. 
 * 
 *  Note that this function largely follows the implementation of the
 *  exec_local_cmd_loop() function that you implemented in the last 
 *  shell program deliverable. The main difference is that the command will
 *  arrive over the recv() socket call rather than reading a string from the
 *  keyboard. 
 * 
 *  This function also must send the EOF character after a command is
 *  successfully executed to let the client know that the output from the
 *  command it sent is finished.  Use the send_message_eof() to accomplish 
 *  this. 
 * 
 *  Of final note, this function must allocate a buffer for storage to 
 *  store the data received by the client. For example:
 *     io_buff = malloc(RDSH_COMM_BUFF_SZ);
 *  And since it is allocating storage, it must also properly clean it up
 *  prior to exiting.
 * 
 *  Returns:
 * 
 *      OK:       The client sent the `exit` command.  Get ready to connect
 *                another client. 
 *      OK_EXIT:  The client sent `stop-server` command to terminate the server
 * 
 *      ERR_RDSH_COMMUNICATION:  A catch all for any socket() related send
 *                or receive errors. 
 */
int exec_client_requests(int cli_socket) {
    char *io_buff = malloc(RDSH_COMM_BUFF_SZ);
    if (!io_buff) {
        return ERR_MEMORY;
    }

    while (1) {
        memset(io_buff, 0, RDSH_COMM_BUFF_SZ);
        int total_bytes = 0;
        int bytes_received;
        int found_null = 0;
        while (1) {
            bytes_received = recv(cli_socket, io_buff + total_bytes, RDSH_COMM_BUFF_SZ - total_bytes, 0);
            
            if (bytes_received < 0) {
                if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR) {
                    continue;
                }
                
                perror("recv");
                free(io_buff);
                return ERR_RDSH_COMMUNICATION;
            }

            if (bytes_received == 0) {
                printf("Client disconnected\n");
                free(io_buff);
                return OK;
            }

            total_bytes += bytes_received;
            for (int i = total_bytes - bytes_received; i < total_bytes; i++) {
                if (io_buff[i] == '\0') {
                    found_null = 1;
                    break;
                }
            }

            if (found_null || total_bytes >= RDSH_COMM_BUFF_SZ - 1) {
                break;
            }
        }

        io_buff[total_bytes] = '\0';        
        command_list_t clist;
        memset(&clist, 0, sizeof(clist));
        int rc = build_cmd_list(io_buff, &clist);
        if (rc != OK) {
            send_message_string(cli_socket, "Error parsing command\n");
            send_message_eof(cli_socket);
            continue;
        }

        if (clist.num == 1) {
            Built_In_Cmds result = rsh_built_in_cmd(&clist.commands[0]);
            if (result != BI_NOT_BI) {
                if (result == BI_CMD_EXIT) {
                    send_message_string(cli_socket, RCMD_MSG_CLIENT_EXITED);
                    send_message_eof(cli_socket);
                    free_cmd_buff(&clist.commands[0]);
                    free(io_buff);

                    return OK;
                } else if (result == BI_CMD_STOP_SVR) {
                    send_message_string(cli_socket, RCMD_MSG_SVR_STOP_REQ);
                    send_message_eof(cli_socket);
                    free_cmd_buff(&clist.commands[0]);
                    free(io_buff);

                    return OK_EXIT;
                }

                send_message_eof(cli_socket);
                free_cmd_buff(&clist.commands[0]);
                continue;
            }
        }

        rc = rsh_execute_pipeline(cli_socket, &clist);
        send_message_eof(cli_socket);
        for (int i = 0; i < clist.num; i++) {
            free_cmd_buff(&clist.commands[i]);
        }
    }

    free(io_buff);
    return OK;
}

/*
 * send_message_eof(cli_socket)
 *      cli_socket:  The server-side socket that is connected to the client

 *  Sends the EOF character to the client to indicate that the server is
 *  finished executing the command that it sent. 
 * 
 *  Returns:
 * 
 *      OK:  The EOF character was sent successfully. 
 * 
 *      ERR_RDSH_COMMUNICATION:  The send() socket call returned an error or if
 *           we were unable to send the EOF character. 
 */
int send_message_eof(int cli_socket){
    int bytes_sent = send(cli_socket, &RDSH_EOF_CHAR, 1, 0);
    if (bytes_sent == 1) {
        return OK;
    }

    return ERR_RDSH_COMMUNICATION;
}

/*
 * send_message_string(cli_socket, char *buff)
 *      cli_socket:  The server-side socket that is connected to the client
 *      buff:        A C string (aka null terminated) of a message we want
 *                   to send to the client. 
 *   
 *  Sends a message to the client.  Note this command executes both a send()
 *  to send the message and a send_message_eof() to send the EOF character to
 *  the client to indicate command execution terminated. 
 * 
 *  Returns:
 * 
 *      OK:  The message in buff followed by the EOF character was 
 *           sent successfully. 
 * 
 *      ERR_RDSH_COMMUNICATION:  The send() socket call returned an error or if
 *           we were unable to send the message followed by the EOF character. 
 */
int send_message_string(int cli_socket, char *buff){
    if (buff == NULL) {
        return ERR_RDSH_COMMUNICATION;
    }

    int len = strlen(buff);
    int total_sent = 0;
    int sent;
    while (total_sent < len) {
        sent = send(cli_socket, buff + total_sent, len - total_sent, 0);
        if (sent <= 0) {
            fprintf(stderr, CMD_ERR_RDSH_SEND, total_sent, len);
            return ERR_RDSH_COMMUNICATION;
        }
        total_sent += sent;
    }

    return send_message_eof(cli_socket);
}


/*
 * rsh_execute_pipeline(int cli_sock, command_list_t *clist)
 *      cli_sock:    The server-side socket that is connected to the client
 *      clist:       The command_list_t structure that we implemented in
 *                   the last shell. 
 *   
 *  This function executes the command pipeline.  It should basically be a
 *  replica of the execute_pipeline() function from the last deliverable. 
 *  The only thing different is that you will be using the cli_sock as the
 *  main file descriptor on the first executable in the pipeline for STDIN,
 *  and the cli_sock for the file descriptor for STDOUT, and STDERR for the
 *  last executable in the pipeline.  See picture below:  
 * 
 *      
 *┌───────────┐                                                    ┌───────────┐
 *│ cli_sock  │                                                    │ cli_sock  │
 *└─────┬─────┘                                                    └────▲──▲───┘
 *      │   ┌──────────────┐     ┌──────────────┐     ┌──────────────┐  │  │    
 *      │   │   Process 1  │     │   Process 2  │     │   Process N  │  │  │    
 *      │   │              │     │              │     │              │  │  │    
 *      └───▶stdin   stdout├─┬──▶│stdin   stdout├─┬──▶│stdin   stdout├──┘  │    
 *          │              │ │   │              │ │   │              │     │    
 *          │        stderr├─┘   │        stderr├─┘   │        stderr├─────┘    
 *          └──────────────┘     └──────────────┘     └──────────────┘   
 *                                                      WEXITSTATUS()
 *                                                      of this last
 *                                                      process to get
 *                                                      the return code
 *                                                      for this function       
 * 
 *  Returns:
 * 
 *      EXIT_CODE:  This function returns the exit code of the last command
 *                  executed in the pipeline.  If only one command is executed
 *                  that value is returned.  Remember, use the WEXITSTATUS()
 *                  macro that we discussed during our fork/exec lecture to
 *                  get this value. 
 */
int rsh_execute_pipeline(int cli_sock, command_list_t *clist) {
    if (!clist || clist->num <= 0) {
        return ERR_RDSH_CMD_EXEC;
    }

    int num_cmds = clist->num;
    int pipe_fds[2][2] = {{-1, -1}, {-1, -1}}; 
    int current = 0;                           
    pid_t pids[CMD_MAX];              
    int i, status;

    for (i = 0; i < CMD_MAX; i++) {
        pids[i] = -1;
    }

    for (i = 0; i < num_cmds; i++) {
        if (i < num_cmds - 1) {
            if (pipe(pipe_fds[current]) < 0) {
                perror("pipe");
                for (int j = 0; j < i; j++) {
                    if (pids[j] > 0) {
                        kill(pids[j], SIGTERM);
                    }
                }

                return ERR_RDSH_CMD_EXEC;
            }
        }

        pids[i] = fork();
        if (pids[i] < 0) {
            perror("fork");
            for (int j = 0; j < i; j++) {
                if (pids[j] > 0) {
                    kill(pids[j], SIGTERM);
                }
            }
            return ERR_RDSH_CMD_EXEC;
        } else if (pids[i] == 0) {
            if (i == 0) {
                if (dup2(cli_sock, STDIN_FILENO) < 0) {
                    perror("dup2 stdin from socket");
                    exit(EXIT_FAILURE);
                }
            } else {
                if (dup2(pipe_fds[1 - current][0], STDIN_FILENO) < 0) {
                    perror("dup2 stdin from pipe");
                    exit(EXIT_FAILURE);
                }
            }

            if (i == num_cmds - 1) {
                if (dup2(cli_sock, STDOUT_FILENO) < 0) {
                    perror("dup2 stdout to socket");
                    exit(EXIT_FAILURE);
                }
                
                if (dup2(cli_sock, STDERR_FILENO) < 0) {
                    perror("dup2 stderr to socket");
                    exit(EXIT_FAILURE);
                }
            } else {
                if (dup2(pipe_fds[current][1], STDOUT_FILENO) < 0) {
                    perror("dup2 stdout to pipe");
                    exit(EXIT_FAILURE);
                }
            }

            for (int j = 0; j < 2; j++) {
                for (int k = 0; k < 2; k++) {
                    if (pipe_fds[j][k] >= 0) {
                        close(pipe_fds[j][k]);
                    }
                }
            }

            execvp(clist->commands[i].argv[0], clist->commands[i].argv);
            fprintf(stderr, "exec %s: %s\n", clist->commands[i].argv[0], strerror(errno));
            exit(EXIT_FAILURE);
        } 
        
        else {
            if (i > 0) {
                if (pipe_fds[1 - current][0] >= 0) {
                    close(pipe_fds[1 - current][0]);
                    pipe_fds[1 - current][0] = -1;
                }
                if (pipe_fds[1 - current][1] >= 0) {
                    close(pipe_fds[1 - current][1]);
                    pipe_fds[1 - current][1] = -1;
                }
            }
            
            current = 1 - current;
        }
    }

    for (int j = 0; j < 2; j++) {
        for (int k = 0; k < 2; k++) {
            if (pipe_fds[j][k] >= 0) {
                close(pipe_fds[j][k]);
                pipe_fds[j][k] = -1;
            }
        }
    }

    if (waitpid(pids[num_cmds - 1], &status, 0) == -1) {
        perror("waitpid");
        return ERR_RDSH_CMD_EXEC;
    }

    for (i = 0; i < num_cmds - 1; i++) {
        if (pids[i] > 0) {
            waitpid(pids[i], NULL, 0);
        }
    }

    return WEXITSTATUS(status);
}

/**************   OPTIONAL STUFF  ***************/
/****
 **** NOTE THAT THE FUNCTIONS BELOW ALIGN TO HOW WE CRAFTED THE SOLUTION
 **** TO SEE IF A COMMAND WAS BUILT IN OR NOT.  YOU CAN USE A DIFFERENT
 **** STRATEGY IF YOU WANT.  IF YOU CHOOSE TO DO SO PLEASE REMOVE THESE
 **** FUNCTIONS AND THE PROTOTYPES FROM rshlib.h
 **** 
 */

/*
 * rsh_match_command(const char *input)
 *      cli_socket:  The string command for a built-in command, e.g., dragon,
 *                   cd, exit-server
 *   
 *  This optional function accepts a command string as input and returns
 *  one of the enumerated values from the BuiltInCmds enum as output. For
 *  example:
 * 
 *      Input             Output
 *      exit              BI_CMD_EXIT
 *      dragon            BI_CMD_DRAGON
 * 
 *  This function is entirely optional to implement if you want to handle
 *  processing built-in commands differently in your implementation. 
 * 
 *  Returns:
 * 
 *      BI_CMD_*:   If the command is built-in returns one of the enumeration
 *                  options, for example "cd" returns BI_CMD_CD
 * 
 *      BI_NOT_BI:  If the command is not "built-in" the BI_NOT_BI value is
 *                  returned. 
 */
Built_In_Cmds rsh_match_command(const char *input)
{
    if (strcmp(input, "exit") == 0)
        return BI_CMD_EXIT;
    else if (strcmp(input, "stop-server") == 0)
        return BI_CMD_STOP_SVR;
    else if (strncmp(input, "cd", 3) == 0)
        return BI_CMD_CD;
    else
        return BI_NOT_BI;
}

/*
 * rsh_built_in_cmd(cmd_buff_t *cmd)
 *      cmd:  The cmd_buff_t of the command, remember, this is the 
 *            parsed version fo the command
 *   
 *  This optional function accepts a parsed cmd and then checks to see if
 *  the cmd is built in or not.  It calls rsh_match_command to see if the 
 *  cmd is built in or not.  Note that rsh_match_command returns BI_NOT_BI
 *  if the command is not built in. If the command is built in this function
 *  uses a switch statement to handle execution if appropriate.   
 * 
 *  Again, using this function is entirely optional if you are using a different
 *  strategy to handle built-in commands.  
 * 
 *  Returns:
 * 
 *      BI_NOT_BI:   Indicates that the cmd provided as input is not built
 *                   in so it should be sent to your fork/exec logic
 *      BI_EXECUTED: Indicates that this function handled the direct execution
 *                   of the command and there is nothing else to do, consider
 *                   it executed.  For example the cmd of "cd" gets the value of
 *                   BI_CMD_CD from rsh_match_command().  It then makes the libc
 *                   call to chdir(cmd->argv[1]); and finally returns BI_EXECUTED
 *      BI_CMD_*     Indicates that a built-in command was matched and the caller
 *                   is responsible for executing it.  For example if this function
 *                   returns BI_CMD_STOP_SVR the caller of this function is
 *                   responsible for stopping the server.  If BI_CMD_EXIT is returned
 *                   the caller is responsible for closing the client connection.
 * 
 *   AGAIN - THIS IS TOTALLY OPTIONAL IF YOU HAVE OR WANT TO HANDLE BUILT-IN
 *   COMMANDS DIFFERENTLY. 
 */
Built_In_Cmds rsh_built_in_cmd(cmd_buff_t *cmd)
{
    Built_In_Cmds bi_cmd = rsh_match_command(cmd->argv[0]);
    switch (bi_cmd) {
        case BI_CMD_EXIT:
            return BI_CMD_EXIT;
        case BI_CMD_STOP_SVR:
            return BI_CMD_STOP_SVR;
        case BI_CMD_CD:
            if (cmd->argv[1] != NULL) {
                if (chdir(cmd->argv[1]) != 0) {
                    perror("cd");
                }
            } else {
                perror("cd");
            }

            return BI_EXECUTED;
        default:
            return BI_NOT_BI;
    }
}