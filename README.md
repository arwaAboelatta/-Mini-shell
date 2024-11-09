# -Mini-shell
This project's main goal is to build a mini shell that executes simple shell commands.


This is a simple shell implementation in C. The shell supports basic command execution, input/output redirection, and some built-in commands.

- Execute multiple commands in sequence.
- Input and output redirection.
- Basic support for built-in commands (e.g., `cd`,'ls','pwd').
- Signal handling for child process termination.
- Wildcard support for pattern matching in file names.

### Prerequisites
- C compiler (e.g., GCC).
- Bison and Flex installed for shell.l , shell.y

### How to Run

1. **Compile the Project**  
   Run this command the first time you set up the project:
   ```bash
   make
   ```

2. **Start the Shell**  
   To start the shell, use:
   ```bash
   ./shell /path_to_your_folder
   ```

3. **Exit the Shell**  
   To exit the shell, type:
   ```bash
   myshell> exit
   ```


