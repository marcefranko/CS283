1. Can you think of why we use `fork/execvp` instead of just calling `execvp` directly? What value do you think the `fork` provides?

    > **Answer**:  As we have seen in class, execvp errases the running program from memory. If we call execvp directly, our running shell (dsh2) will be replaced with the program we called with execvp, meaning we couldn't return to it after the program finishes. Using fork() solves this by creating a child process that calls execvp(). This way, only the child process is replaced, while the parent shell remains running. When the child process finishes execution, the parent can continue accepting commands, maintaining the functionality of the shell.

2. What happens if the fork() system call fails? How does your implementation handle this scenario?

    > **Answer**:  if the fork() syscall falis, it will return -1. My program handles this scenario by asking if the result of fork() was negative. if it is, then we immediatly return to the exec_local_cmd_loop() with ERR_EXEC_CMD

3. How does execvp() find the command to execute? What system environment variable plays a role in this process?

    > **Answer**:  execvp() finds the command by looking through the directories listed in PATH. If the command includes a /, it tries to run it as a file path. Otherwise, it checks each folder in PATH, looking for a matching program. If it finds one, it runs it; if not, it gives an error saying the command wasn’t found

4. What is the purpose of calling wait() in the parent process after forking? What would happen if we didn’t call it?

    > **Answer**:  As seen in class, wait() makes the parent process wait for the child process to finish executing. If we didn't call wait, the parent would keep running, which can lead to the parent finishing before the child, which would create a zombie process that might perform unexpected behaviour in our shell.

5. In the referenced demo code we used WEXITSTATUS(). What information does this provide, and why is it important?

    > **Answer**:  WEXITSTATUS() provides the exit status of the child process to the parent process. It is important because when a child process exits, it will give us the exit code combined with other junk. WEXITSTATUS() will filter all that junk for us, and will provide the actuall child's exit code.

6. Describe how your implementation of build_cmd_buff() handles quoted arguments. Why is this necessary?

    > **Answer**:  In the build_cmd_buff() function, quoted arguments are handled by using a flag (in_quote) to check if the parser is inside quotes. When a double quote (") is found, the function sets the flag, so characters inside the quotes are treated as one argument, even if they have spaces. When the closing quote is found, the argument is finished and added to the cmd_buff->argv list. This is important because without this, spaces inside quotes would make the function think there are separate arguments, which would lead to errors. This method ensures that text within quotes is treated as a single argument.

7. What changes did you make to your parsing logic compared to the previous assignment? Were there any unexpected challenges in refactoring your old code?

    > **Answer**:  To begin with, since we are not implementing pipelines yet, I got rid of every logic related to handling pipelines. I also changed the approach of getting rid of any leading, trailing, or multiple whitespace from using strtok to make every whitespace encountered (except the ones in between "") become a null character, and for every argument that the inputted command has, make argv[argument_number - 1] point to the beggining character of that argument. This last one was an unexpected challenge, because I thought that implementing strtok in this assigment would work too, but that approach didn't work.

8. For this quesiton, you need to do some research on Linux signals. You can use [this google search](https://www.google.com/search?q=Linux+signals+overview+site%3Aman7.org+OR+site%3Alinux.die.net+OR+site%3Atldp.org&oq=Linux+signals+overview+site%3Aman7.org+OR+site%3Alinux.die.net+OR+site%3Atldp.org&gs_lcrp=EgZjaHJvbWUyBggAEEUYOdIBBzc2MGowajeoAgCwAgA&sourceid=chrome&ie=UTF-8) to get started.

- What is the purpose of signals in a Linux system, and how do they differ from other forms of interprocess communication (IPC)?

    > **Answer**:  Signals in Linux are used to send simple notifications to processes, telling them to take certain actions, like stopping or terminating. Unlike other methods of communication, like pipes, signals only notify a process about an event, without sending any actual data. 

- Find and describe three commonly used signals (e.g., SIGKILL, SIGTERM, SIGINT). What are their typical use cases?

    > **Answer**:  SIGKILL: This signal forces a process to immediately terminate and cannot be caught or ignored. typically used when a process does not respond or cannot be terminated in some other way. It ensures that the process is killed immediately. SIGTERM: This is a request to terminate a process 'gracefully', allowing it to clean up resources before exiting. It's commonly used by applications or system administrators to stop processes in a 'proper' way. SIGINT: This signal is sent when the user presses Ctrl+C in the terminal. It's used to interrupt a running process, often to stop it manually.

- What happens when a process receives SIGSTOP? Can it be caught or ignored like SIGINT? Why or why not?

    > **Answer**:  When a process receives SIGSTOP, it is stopped/paused and it cannot continue executing until it is resumed with a signal like SIGCONT. SIGSTOP cannot be caught or ignored like SIGINT. This is because SIGSTOP is designed to control process execution at the kernel level, ensuring the process is halted regardless of what the process itself wants to do. It is used to pause a process for debugging or other management tasks.
