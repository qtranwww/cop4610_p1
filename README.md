# Shell Project

A Unix-style shell written in C for COP4610. The shell supports command execution, environment variable and tilde expansion, PATH search, I/O redirection, piping, background processing, and built-in commands.

## Group Members
- **Quan Tran**: qt24b@fsu.edu
- **Ty Officer**: tro23@fsu.edu
- **Alexander Turner**: ajt22g@fsu.edu

## Division of Labor

### Part 1: Prompt
- **Responsibilities**: Handles displaying shell prompt with current user, machine name, and working directory
- **Assigned to**: Quan, Alex

### Part 2: Environment Variables
- **Responsibilities**: Handles expanding environment variables into corresponding values within command tokens
- **Assigned to**: Quan, Alex

### Part 3: Tilde Expansion
- **Responsibilities**: Handles exapnding ~ and ~/ paths to user's home directory
- **Assigned to**: Quan, Alex

### Part 4: PATH Search
- **Responsibilities**: Handles locating extenal commands by searching through directories in $PATH environment variable
- **Assigned to**: Quan, Ty

### Part 5: External Command Execution
- **Responsibilities**: Handles creation of child processes and external command execution
- **Assigned to**: Quan, Ty

### Part 6: I/O Redirection
- **Responsibilities**: Handles redirecting standard input and output between commands and files.
- **Assigned to**: Quan, Ty

### Part 7: Piping
- **Responsibilities**: Handles connecting multiple commands through pipes so that output of one command is input of the next.
- **Assigned to**: Ty, Alex

### Part 8: Background Processing
- **Responsibilities**: Handles executing commands and pipelines in the background while tracking job numbers, process IDs, and completion status
- **Assigned to**: Ty, Alex

### Part 9: Internal Command Execution
- **Responsibilities**: Handles built-in shell commands such as cd, exit, and jobs, with command history and background process management
- **Assigned to**: Ty, Alex

### C Project Structure

```text
root/
├── bin/           -- executables
├── include/       -- header files
├── obj/           -- object files
├── src/           -- source files
├── Makefile
└── README.md
```

## File Listing

```text
root/
├── src/
│   ├── execution.c
│   ├── jobs.c
│   ├── lexer.c
│   ├── path.c
│   ├── pipeline.c
│   └── redirection.c
├── include/
│   ├── execution.h
│   ├── jobs.h
│   ├── lexer.h
│   ├── path.h
│   ├── pipeline.h
│   └── redirection.h
├── README.md
├── Makefile
└── .gitignore
```

## How to Compile & Execute

### Requirements
- **Compiler**: `gcc`
- **Language**: C
- **Environment**: FSU `linprog`
- **Dependencies**: None

### Compilation

```bash
make
```

This builds the shell executable in:

```text
bin/shell
```

### Execution

```bash
make run
```

or:

```bash
./bin/shell
```

### Clean

```bash
make clean
```

## Bugs

- No known major bugs at the time of submission.
- Quote handling, glob expansion, escaped characters, and autocomplete are not implemented because they are outside the project requirements.
- Background process output may appear after the prompt because background processes and the shell share the same terminal output.

## Considerations

The shell only supports the syntax required by the project specification. Features such as glob expansion, quote parsing, regular expressions, escaped characters, and autocomplete are not implemented.

## Use of AI

AI assistance was used primarily to help create test cases and determine expected shell behavior during testing. The test cases were manually run on the `linprog` environment and compared with the project requirements.

AI was also used in a limited capacity to help explain unexpected test results. The main use of AI was test planning and validation.

### Part 1: Prompt

| Test Case | Expected Result |
|---|---|
| Start the shell with `make run` | Prompt displays `USER@MACHINE:ABSOLUTE_PATH>` |
| Run a command and return to the shell | Prompt appears again after the command finishes |

### Part 2: Environment Variables

| Test Case | Expected Result |
|---|---|
| `echo $USER` | Prints the current username |
| `echo $HOME` | Prints the user's home directory |
| `echo $VARIABLE_THAT_DOES_NOT_EXIST` | Prints a blank line |
| `echo $user` | Prints a blank line if lowercase `user` is not defined |

### Part 3: Tilde Expansion

| Test Case | Expected Result |
|---|---|
| `ls ~` | Lists the user's home directory |
| `ls ~/COP4610` | Accesses `$HOME/COP4610` |
| `ls ~abc` | `~abc` is not expanded |
| `ls abc/~` | `abc/~` is not expanded |

### Part 4: PATH Search

| Test Case | Expected Result |
|---|---|
| `ls` | Finds the first matching executable in `$PATH` |
| `ls -al` | Resolves `ls` and keeps `-al` as an argument |
| `/bin/ls` | Uses the supplied path directly |
| `./program` | Uses the relative path directly if the executable exists |
| `command_that_does_not_exist` | Prints a command-not-found error |

### Part 5: External Command Execution

| Test Case | Expected Result |
|---|---|
| `ls` | Executes successfully |
| `ls -al` | Passes command arguments correctly |
| `sleep 2` | Shell waits for the foreground process |
| `/bin/ls` | Executes directly through `execv()` |

### Part 6: I/O Redirection

| Test Case | Expected Result |
|---|---|
| `echo hello > output.txt` | Creates `output.txt` containing `hello` |
| `cat < input.txt` | Reads input from `input.txt` |
| `sort < input.txt` | Prints sorted input |
| `sort < input.txt > output.txt` | Writes sorted data to `output.txt` |
| `sort > output.txt < input.txt` | Same result as the previous test |
| `cat < doesnotexist.txt` | Reports an input file error |
| Create a new output file | File permission is `-rw-------` |

### Part 7: Piping

| Test Case | Expected Result |
|---|---|
| `echo hello \| cat` | Prints `hello` |
| `echo hello \| wc -c` | Prints `6` |
| `seq 100 \| wc -l` | Prints `100` |
| `seq 10 \| grep 5 \| wc -l` | Prints `1` |
| `yes \| head -n 5` | Prints five lines and exits without hanging |
| `echo hello \| sleep 2` | Waits about two seconds and does not print `hello` |

### Part 8: Background Processing

| Test Case | Expected Result |
|---|---|
| `sleep 3 &` | Prints job number and PID, then returns to prompt |
| `sleep 10 &` followed by `jobs` | Displays the active background job |
| Multiple background jobs | Job numbers increase and are not reused |
| `sleep 3 \| cat &` | Pipeline runs in the background |
| `sort < input.txt > output.txt &` | Redirection works in the background |
| Finished job followed by another input | Shell reports the job as done |

### Part 9: Internal Command Execution

| Test Case | Expected Result |
|---|---|
| `cd /tmp` | Changes current directory to `/tmp` |
| `cd` | Changes current directory to `$HOME` |
| `cd /does/not/exist` | Reports an error |
| `cd one two` | Reports too many arguments |
| `jobs` with no active jobs | Reports no active background jobs |
| `sleep 5 &` then `exit` | Waits for the background job before exiting |
| More than three valid commands then `exit` | Prints only the last three valid commands |
