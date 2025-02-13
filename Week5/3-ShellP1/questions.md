1. In this assignment I suggested you use `fgets()` to get user input in the main while loop. Why is `fgets()` a good choice for this application?

    > **Answer**:  fgets() for this assignemtn was wonderful. First, because in comparison to other functions like write(), fgets() does not stop getting input from the user until it hits enter, which is far more efficient than doing things like stopping every character to make a systemcall. Also, one of my favourite things about fgets() is that this function is designed to stop recieving input, and save it as soon as we hit 'enter'. With other methods, achieving this feature might have taken extra lines of code.

2. You needed to use `malloc()` to allocte memory for `cmd_buff` in `dsh_cli.c`. Can you explain why you needed to do that, instead of allocating a fixed-size array?

    > **Answer**: I have to be honest, I am not very sure why, but malloc() provides flexibility in memory management. By using a fixed_size array, we would be using the stack memory, however, malloc() allocates actual memory which is better for large buffers. 


3. In `dshlib.c`, the function `build_cmd_list(`)` must trim leading and trailing spaces from each command before storing it. Why is this necessary? If we didn't trim spaces, what kind of issues might arise when executing commands in our shell?

    > **Answer**:  triming space in commands is very important, because the command " cmd1" is not the same as the command "cmd1". Not trimming these extra spaces can cause misinterpretation from the shell, leading to unexpected errors.

4. For this question you need to do some research on STDIN, STDOUT, and STDERR in Linux. We've learned this week that shells are "robust brokers of input and output". Google _"linux shell stdin stdout stderr explained"_ to get started.

- One topic you should have found information on is "redirection". Please provide at least 3 redirection examples that we should implement in our custom shell, and explain what challenges we might have implementing them.

    > **Answer**:  First example: echo "I like vanilla" > output.txt". This command redirects stdout of echo to the file output.txt instead of displaying it on the terminal. Challenges we might face include handling file creation and overwriting. 

    > Second example: sort < text.txt. This command redirects the contents of text.txt into the stdin of the sort command. Challenges can include to handle the redirection of the input streams: we would need to open the file, read its contents, and pass them to the command as if they were typed by the user. Moreover, we could face empty or non-existent files situations. 

    > Third example: echo "I like vanilla" >> output.txt, which appends the output of echo into output.txt. To achieve this, we need to know how to open output.txt in any sort of "append mode" if thre exists one, and make sure that we are appending to the file and not overwriting it. We will also need to check if the file we are inputting in the command actually exists, and if it doesn't, output an error or create one.

- You should have also learned about "pipes". Redirection and piping both involve controlling input and output in the shell, but they serve different purposes. Explain the key differences between redirection and piping.

    > **Answer**:  Maintaining the answer brief, redirection is used to change the destination of stdin, stdout, or stderr streams. Pipes are used to grab the output of one command and use it as the input for the other command at the other side of the pipe.

- STDERR is often used for error messages, while STDOUT is for regular output. Why is it important to keep these separate in a shell?

    > **Answer**:  It makes it easier to distinguish between the intended output of a command and the errors that occured during execution. This allows programmers and users to see if the command, program, process run as expected or didn't. I have also seen that when we want to pipeline from one command to the other, the pipe transfers stdout to the stdin of the other command, and not stderr. By separating the two streams we can make sure that only stdout is passed, so the other command is not playing around with the error messages of the other command. 

- How should our custom shell handle errors from commands that fail? Consider cases where a command outputs both STDOUT and STDERR. Should we provide a way to merge them, and if so, how?

    > **Answer**:  To handle these errors, our shell should be able to distinguish between stdout and stderr. As mentioned previously, this will make sure that 'normal' output and error messages are kept separate for clarity. It also should be able to give us clear feedback based on the command's return code. Cases where we might want to merge stdout and stderr might include logging or troubleshooting, because having all output in a single stream makes it easier to track the flow of execution and identify the errors. A way we can manage to merge these two can be by redirecting both streams to a single stream.