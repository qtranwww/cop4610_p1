# Shell Project

A Unix-style shell written in C for COP4610. The shell supports command execution, environment variable and tilde expansion, PATH search, I/O redirection, piping, background processing, and built-in commands.

## Group Members
**Group Number**: 34

- Quan Tran
- Ty Officer
- Alexander Turner

## Division of Labor

### Part 1: Prompt
- **Responsibilities**: Print the `USER@MACHINE:PWD>` prompt.
- **Assigned to**: Quan Tran, Alexander Turner

### Part 2: Environment Variables
- **Responsibilities**: Expand whole-token `$VAR` arguments.
- **Assigned to**: Quan Tran, Alexander Turner

### Part 3: Tilde Expansion
- **Responsibilities**: Expand standalone `~` and leading `~/`.
- **Assigned to**: Quan Tran, Alexander Turner

### Part 4: PATH Search
- **Responsibilities**: Search `$PATH` for commands without a slash.
- **Assigned to**: Quan Tran, Ty Officer

### Part 5: External Command Execution
- **Responsibilities**: Run external commands with `fork()` and `execv()`.
- **Assigned to**: Quan Tran, Ty Officer

### Part 6: I/O Redirection
- **Responsibilities**: Handle `<` and `>` with the required file permissions.
- **Assigned to**: Quan Tran, Ty Officer

### Part 7: Piping
- **Responsibilities**: Connect commands with pipes.
- **Assigned to**: Ty Officer, Alexander Turner

### Part 8: Background Processing
- **Responsibilities**: Run jobs with `&`, track job numbers and report completion.
- **Assigned to**: Ty Officer, Alexander Turner

### Part 9: Internal Command Execution
- **Responsibilities**: Implement the `exit`, `cd` and `jobs` built-ins.
- **Assigned to**: Alexander Turner, Ty Officer

### Extra Credit
- **Responsibilities**: Unlimited pipes, piping with I/O redirection and shell-ception.
- **Assigned to**: Alexander Turner, Ty Officer

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

If you are programming in C, you must include a makefile to compile your project.

## File Listing

```text
root/
├── src/
│   ├── execution.c
│   ├── jobs.c
│   ├── lexer.c
│   ├── main.c
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

## Development Log

Each member records their contributions here.

### Quan Tran

| Date | Work Completed / Notes |
|---|---|
| 2026-09-22 | Added the provided starter code; implemented the prompt (part 1) |
| 2026-09-23 | Environment variable expansion (part 2), tilde expansion (part 3), PATH search (part 4), external command execution (part 5) |
| 2026-09-24 | I/O redirection (part 6) |
| 2026-09-28 | Fixed redirection handling in pipelines (`<` `>` `\|`); first draft of the README |

### Ty Officer

| Date | Work Completed / Notes |
|---|---|
| 2026-09-25 | Piping and background processing (parts 7-8) on the `tyofficer` branch |

### Alexander Turner

| Date | Work Completed / Notes |
|---|---|
| 2026-09-26 | Internal commands: `exit`, `cd`, `jobs` (part 9); merged the `tyofficer` branch; removed object and program files from the repo |

## Meetings

| Date | Attendees | Topics Discussed | Outcomes / Decisions |
|---|---|---|---|
| YYYY-MM-DD | [Names] | [Agenda items] | [Actions/Next steps] |
| YYYY-MM-DD | [Names] | [Agenda items] | [Actions/Next steps] |

## Bugs

- No known major bugs at the time of submission.
- Quote handling, glob expansion, escaped characters, and autocomplete are not implemented because they are outside the project requirements.
- Background process output may appear after the prompt because background processes and the shell share the same terminal output.

## Extra Credit

- **Unlimited piping**: Supports more than two pipe operators in one command.
- **Piping with I/O redirection**: Supports commands that combine pipes with `<` and `>`.
- **Shell-ception**: Supports starting another shell instance from within the shell.

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

### Extra Credit

| Test Case | Expected Result |
|---|---|
| `echo hello \| cat \| cat \| cat \| wc -c` | Prints `6` |
| `seq 10000 \| sort \| uniq \| wc -l` | Prints `10000` |
| `cat < input.txt \| sort > output.txt` | Stores sorted input in `output.txt` |
| `./bin/shell` | Starts a nested shell |
| `exit` inside nested shell | Returns to the parent shell |

### Additional Validation

| Test Case | Expected Result |
|---|---|
| Run with `valgrind --leak-check=full` | No memory leaks or invalid memory access |
| Check background jobs using `ps` | No persistent zombie processes |
| `make clean` followed by `make` | Project compiles successfully |
