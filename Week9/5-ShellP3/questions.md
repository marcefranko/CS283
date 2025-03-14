1. Your shell forks multiple child processes when executing piped commands. How does your implementation ensure that all child processes complete before the shell continues accepting user input? What would happen if you forgot to call waitpid() on all child processes?

_answer here_ My shell ensures that all childs complete beofre the parent continues by calling the waitpid() in every child created: 
{

    for (int i = 0; i < clist->num; i++) {

        waitpid(pids[i], NULL, 0);

    }

If I forgot to call waitpid() for every child that has been created, they would become zombie processes, which may lead to confusing behaviour (for example, printing the shell prompt before recieving output from one of the childs). Moreover, it could lead to resource exhaustion due to an accumulation of zombie processes.

2. The dup2() function is used to redirect input and output file descriptors. Explain why it is necessary to close unused pipe ends after calling dup2(). What could go wrong if you leave pipes open?

_answer here_ Closing unused pipe ends is necessary to prevent file descriptor leaks and ensure proper pipe behavior. For example, if a process is waiting for input from a pipe but the write end remains open in another process, the read operation may never return EOF, causing the process to hang indefinitely. Or, it could also lead to file descriptor exaustion.

3. Your shell recognizes built-in commands (cd, exit, dragon). Unlike external commands, built-in commands do not require execvp(). Why is cd implemented as a built-in rather than an external command? What challenges would arise if cd were implemented as an external process?

_answer here_ After doing some investigation, I realized that the reason why cd is a built in command is because cd should apply its "effects" on the shell itself. If we implement cd as an external command, then the one being influenced by cd is going to be the child, and not the shell; meaning that the cwd will remain unchanged. So if we were to implement cd externally, we would have to figure out a way to create a child that influences on the parent too, which I imagine it would be inefficient, and we would be breaking our head for such a simple operation.

4. Currently, your shell supports a fixed number of piped commands (CMD_MAX). How would you modify your implementation to allow an arbitrary number of piped commands while still handling memory allocation efficiently? What trade-offs would you need to consider?

_answer here_ To allow an unknown number of piped commands, I would probably have to dynamically allocate memory for the pipes and pids arrays using malloc() or calloc() based on the number of commands in clist->num. This ensures memory is allocated only as needed rather than reserving a fixed, potentially excessive amount. After creating and using the pipes, you would free the allocated memory to prevent leaks.
